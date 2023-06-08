#include <fstream>
#include <iostream>

#include "fdml/internal/json_utils.hpp"
#include "fdml/internal/utils.hpp"

#include <boost/json.hpp>
#ifdef FDML_LINUX
#include <boost/json/src.hpp>
#endif

#include <CGAL/Boolean_set_operations_2.h>
#include <CGAL/Boolean_set_operations_2/Gps_polygon_validation.h>
#include <CGAL/Polygon_with_holes_2.h>

namespace FDML {

static boost::json::value parse_file(std::string filename) {
    std::ifstream fin(filename, std::ifstream::binary);
    char buf[4096];

    boost::json::stream_parser p;
    boost::json::error_code ec;
    while (!fin.eof()) {
        fin.read(buf, 4096);
        std::streamsize s = fin.gcount();
        p.write(buf, (size_t)s, ec);
    }
    fin.close();

    if (ec)
        throw std::runtime_error("failed to parse json file");
    p.finish(ec);
    if (ec)
        throw std::runtime_error("failed to parse json file");
    return p.release();
}

#define ERR(...)                                                                                                       \
    do {                                                                                                               \
        std::ostringstream oss;                                                                                        \
        oss << __VA_ARGS__ << std::endl;                                                                               \
        throw std::runtime_error(oss.str());                                                                           \
    } while (0)

static const boost::json::object& get_object(const boost::json::value& v, const char* obj_name) {
    if (v.kind() != boost::json::kind::object)
        ERR("Expected field \"" << obj_name << "\" to be Json object (" << boost::json::kind::object << "), actual is "
                                << v.kind());
    return v.get_object();
}

static const boost::json::array& get_array(const boost::json::value& v, const char* obj_name) {
    if (v.kind() != boost::json::kind::array)
        ERR("Expected field \"" << obj_name << "\" to be Json array (" << boost::json::kind::array << "), actual is "
                                << v.kind());
    return v.get_array();
}

static double get_double(const boost::json::value& v, const char* obj_name) {
    if (v.kind() != boost::json::kind::double_)
        ERR("Expected field \"" << obj_name << "\" to be Json double (" << boost::json::kind::double_ << "), actual is "
                                << v.kind());
    return v.get_double();
}

template <typename JsonObj> static void validate_size(const JsonObj& obj, unsigned int size, const char* obj_name) {
    if (obj.size() != size)
        ERR("Unexpected number of elements in \"" << obj_name << "\" Json object: " << obj.size() << " != " << size);
}

static Polygon parse_polygon(const boost::json::array& poly_obj) {
    std::vector<Point> points;

    for (auto points_it = poly_obj.begin(); points_it != poly_obj.end(); ++points_it) {
        const auto& point = get_array(*points_it, "point");
        validate_size(point, 2, "point");

        double x = get_double(point[0], "x");
        double y = get_double(point[1], "y");
        points.push_back({x, y});
    }

    Polygon poly(points.begin(), points.end());
    CGAL::Gps_default_traits<Polygon>::Traits poly_traits;
    if (!CGAL::has_valid_orientation_polygon(poly, poly_traits)) {
        std::reverse(points.begin(), points.end());
        poly = Polygon(points.begin(), points.end());
    }
    assert(CGAL::has_valid_orientation_polygon(poly, poly_traits));
    return poly;
}

Polygon_with_holes JsonUtils::read_scene(const std::string& filename) {
    fdml_debugln("[JsonUtils] parsing scene from file: " << filename);
    const auto j = parse_file(filename);
    Polygon scene_boundary;
    std::vector<Polygon> holes;
    CGAL::Gps_default_traits<Polygon>::Traits poly_traits;

    const auto& obj = get_object(j, "top_level");
    for (auto obj_it = obj.begin(); obj_it != obj.end(); ++obj_it) {
        auto obj_key = obj_it->key();

        if (std::string(obj_key) == "scene_boundary") {
            scene_boundary = parse_polygon(get_array(obj_it->value(), "scene_boundary"));

        } else if (std::string(obj_key) == "holes") {
            const auto& holes_obj = get_array(obj_it->value(), "holes");

            /* parse holes */
            for (auto holes_it = holes_obj.begin(); holes_it != holes_obj.end(); ++holes_it)
                holes.push_back(parse_polygon(get_array(*holes_it, "hole")));

        } else {
            ERR("Unknown Json tag: " << obj_key);
        }
    }

    fdml_debugln("[JsonUtils] parsed scene:");
    fdml_debugln("\tscene_boundary:");
    fdml_debug("\t\t");
    for (auto vit = scene_boundary.vertices_begin(); vit != scene_boundary.vertices_end(); ++vit)
        fdml_debug(" (" << *vit << ")");
    fdml_debug("\n");
    fdml_debugln("\tholes:");
    for (const auto& hole : holes) {
        fdml_debug("\t\t");
        for (auto vit = hole.vertices_begin(); vit != hole.vertices_end(); ++vit)
            fdml_debug(" (" << *vit << ")");
        fdml_debug("\n");
    }

    /* construct polygon with holes result */
    Polygon_with_holes scene(scene_boundary);
    assert(CGAL::has_valid_orientation_polygon_with_holes(scene, poly_traits));
    for (auto& hole : holes) {
        assert(CGAL::has_valid_orientation_polygon(hole, poly_traits));
        /* reverse hole vertices order. When CGAL checks for a polygon with hole validity, it expect the output boundary
         * to be in the usual orientation (vertices order), but the holes order should be reversed as they OUTER edges
         * list is checked rather than their inner edges list which is the usual polygon orientation */
        std::vector<Point> holes_points(hole.vertices_begin(), hole.vertices_end());
        std::reverse(holes_points.begin(), holes_points.end());
        hole = Polygon(holes_points.begin(), holes_points.end());
        assert(!CGAL::has_valid_orientation_polygon(hole, poly_traits));
        scene.add_hole(hole);
    }

    assert(CGAL::has_valid_orientation_polygon_with_holes(scene, poly_traits));
    return scene;
}

template <typename Out> void json_format_pretty(Out& os, boost::json::value const& jv, std::string* indent = nullptr) {
    std::string indent_;
    if (!indent)
        indent = &indent_;
    switch (jv.kind()) {
    case boost::json::kind::object: {
        os << "{\n";
        indent->append(4, ' ');
        auto const& obj = jv.get_object();
        if (!obj.empty()) {
            auto it = obj.begin();
            for (;;) {
                os << *indent << boost::json::serialize(it->key()) << " : ";
                json_format_pretty(os, it->value(), indent);
                if (++it == obj.end())
                    break;
                os << ",\n";
            }
        }
        os << "\n";
        indent->resize(indent->size() - 4);
        os << *indent << "}";
        break;
    }

    case boost::json::kind::array: {
        os << "[\n";
        indent->append(4, ' ');
        auto const& arr = jv.get_array();
        if (!arr.empty()) {
            auto it = arr.begin();
            for (;;) {
                os << *indent;
                json_format_pretty(os, *it, indent);
                if (++it == arr.end())
                    break;
                os << ",\n";
            }
        }
        os << "\n";
        indent->resize(indent->size() - 4);
        os << *indent << "]";
        break;
    }

    case boost::json::kind::string: {
        os << boost::json::serialize(jv.get_string());
        break;
    }

    case boost::json::kind::uint64:
        os << jv.get_uint64();
        break;

    case boost::json::kind::int64:
        os << jv.get_int64();
        break;

    case boost::json::kind::double_:
        os << jv.get_double();
        break;

    case boost::json::kind::bool_:
        if (jv.get_bool())
            os << "true";
        else
            os << "false";
        break;

    case boost::json::kind::null:
        os << "null";
        break;
    }

    if (indent->empty())
        os << "\n";
}

static boost::json::array point2json(const Point& p) {
    std::vector<double> point{CGAL::to_double(p.x()), CGAL::to_double(p.y())};
    return boost::json::array(point.begin(), point.end());
}

void JsonUtils::write_polygons(const std::vector<Polygon>& polygons, const std::string& filename) {
    fdml_debugln("[JsonUtils] writing polygons into: " << filename);
    std::vector<boost::json::array> polygon_objs;
    for (const Polygon& polygon : polygons) {
        std::vector<boost::json::array> point_objs;
        for (auto it = polygon.vertices_begin(); it != polygon.vertices_end(); ++it)
            point_objs.push_back(point2json(*it));
        boost::json::array polygon_obj(point_objs.begin(), point_objs.end());
        polygon_objs.emplace_back(polygon_obj);
    }
    std::vector<std::pair<std::string, boost::json::value>> top_lvl_fields;
    top_lvl_fields.emplace_back("polygons", boost::json::array(polygon_objs.begin(), polygon_objs.end()));
    boost::json::object top_lvl_obj(top_lvl_fields.begin(), top_lvl_fields.end());
    boost::json::value top_lvl_obj2(top_lvl_obj);

    std::ofstream outfile(filename);
    json_format_pretty(outfile, top_lvl_obj2);
    outfile.close();
}

void JsonUtils::write_segments(const std::vector<Segment>& segments, const std::string& filename) {
    fdml_debugln("[JsonUtils] writing segments into: " << filename);
    std::vector<boost::json::array> segments_objs;
    for (const Segment& segment : segments) {
        std::vector<boost::json::array> point_objs;
        point_objs.push_back(point2json(segment.source()));
        point_objs.push_back(point2json(segment.target()));
        segments_objs.emplace_back(point_objs.begin(), point_objs.end());
    }
    std::vector<std::pair<std::string, boost::json::value>> top_lvl_fields;
    top_lvl_fields.emplace_back("segments", boost::json::array(segments_objs.begin(), segments_objs.end()));
    boost::json::object top_lvl_obj(top_lvl_fields.begin(), top_lvl_fields.end());
    boost::json::value top_lvl_obj2(top_lvl_obj);

    std::ofstream outfile(filename);
    json_format_pretty(outfile, top_lvl_obj2);
    outfile.close();
}

void JsonUtils::write_points(const std::vector<Point>& points, const std::string& filename) {
    fdml_debugln("[JsonUtils] writing points into: " << filename);
    std::vector<boost::json::array> point_objs;
    for (auto it = points.begin(); it != points.end(); ++it)
        point_objs.push_back(point2json(*it));
    boost::json::array points_obj(point_objs.begin(), point_objs.end());

    std::vector<std::pair<std::string, boost::json::value>> top_lvl_fields;
    top_lvl_fields.emplace_back("points", points_obj);
    boost::json::object top_lvl_obj(top_lvl_fields.begin(), top_lvl_fields.end());
    boost::json::value top_lvl_obj2(top_lvl_obj);

    std::ofstream outfile(filename);
    json_format_pretty(outfile, top_lvl_obj2);
    outfile.close();
}

} // namespace FDML
