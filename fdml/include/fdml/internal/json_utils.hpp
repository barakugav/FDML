#ifndef FDML_JSON_UTILS_HPP
#define FDML_JSON_UTILS_HPP

#include <vector>

#include "fdml/config.hpp"
#include "fdml/defs.hpp"

namespace FDML {

class FDML_FDML_DECL JsonUtils {
  public:
    /**
     * @brief Parse a scene from a JSON file
     *
     * @param filename path to scene file
     * @return parsed scene as a polygon object
     */
    static Polygon_with_holes read_scene(const std::string& filename);

    /**
     * @brief Write polygons into a JSON file
     *
     * @param polygons list of polygons
     * @param filename output filename
     */
    static void write_polygons(const std::vector<Polygon>& polygons, const std::string& filename);

    /**
     * @brief Write segments into a JSON file
     *
     * @param segments list of segments
     * @param filename output file
     */
    static void write_segments(const std::vector<Segment>& segments, const std::string& filename);

    static void write_points(const std::vector<Point>& points, const std::string& filename);

};

} // namespace FDML

#endif
