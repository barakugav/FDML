#include <math.h>
#ifndef M_PI
// windows
#define _USE_MATH_DEFINES
#include <cmath>
#endif

#include "trapezoider.h"
#include "utils.hpp"
#include <CGAL/Arr_vertical_decomposition_2.h>

static Halfedge direct_edge(const Halfedge &edge) {
	if (edge->curve().target().hx() != edge->curve().source().hx())
		return edge->curve().target().hx() >= edge->curve().source().hx() ? edge : edge->twin();
	return edge->curve().target().hy() >= edge->curve().source().hy() ? edge : edge->twin();
}

class DecompVertexData {
  public:
	bool is_edge_above;
	bool is_edge_below;
	Halfedge edge_above;
	Halfedge edge_below;
	DecompVertexData() { is_edge_above = is_edge_below = false; }
};

static void vertical_decomposition(const Arrangement &arr, std::vector<Vertex> &vertices,
								   std::map<Vertex, DecompVertexData> &decomp) {
	std::vector<std::pair<Vertex, std::pair<CGAL::Object, CGAL::Object>>> vd_list;
	CGAL::decompose(arr, std::back_inserter(vd_list));

	std::map<Vertex, CGAL::Object> above_orig;
	std::map<Vertex, CGAL::Object> below_orig;
	for (auto &decomp_entry : vd_list) {
		above_orig[decomp_entry.first] = decomp_entry.second.second;
		below_orig[decomp_entry.first] = decomp_entry.second.first;
	}

	for (auto &decomp_entry : vd_list)
		vertices.push_back(decomp_entry.first);

	sort(vertices.begin(), vertices.end(), [](const Vertex &v1, const Vertex &v2) {
		if (v1->point().hx() != v2->point().hx())
			return v1->point().hx() < v2->point().hx();
		return v1->point().hy() < v2->point().hy();
	});
	for (auto &v : vertices) {
		DecompVertexData v_data;
		Halfedge edge;
		bool is_edge;
		for (Vertex p = v, up_vertex;; p = up_vertex) {
			auto &above_obj = above_orig[p];
			if (CGAL::assign(edge, above_obj)) {
				v_data.edge_above = direct_edge(edge);
				v_data.is_edge_above = true;
				break;
			}
			if (!CGAL::assign(up_vertex, above_obj))
				break;
		}
		for (Vertex p = v, below_vertex;; p = below_vertex) {
			auto &below_obj = below_orig[p];
			if (CGAL::assign(edge, below_obj)) {
				v_data.edge_below = direct_edge(edge);
				v_data.is_edge_below = true;
				break;
			}
			if (!CGAL::assign(below_vertex, below_obj))
				break;
		}
		decomp[v] = v_data;
	}
}

static const Direction ANGLE_NONE(0, 0);

Trapezoid::Trapezoid(TrapezoidID id, Halfedge top_edge, Halfedge bottom_edge, Vertex left_vertex, Vertex right_vertex)
	: id(id), top_edge(top_edge), bottom_edge(bottom_edge), left_vertex(left_vertex), right_vertex(right_vertex),
	  angle_begin(ANGLE_NONE), angle_end(ANGLE_NONE) {}

TrapezoidID Trapezoid::get_id() const { return id; }

static bool undirected_eq(const Halfedge &e1, const Halfedge &e2) { return e1 == e2 || e1 == e2->twin(); }

#define INVALID_TRAPEZOID_ID -1

VertexData::VertexData(Point &v, const Arrangement::Geometry_traits_2 *geom_traits) {
	top_left_trapezoid = top_right_trapezoid = INVALID_TRAPEZOID_ID;
	bottom_left_trapezoid = bottom_right_trapezoid = INVALID_TRAPEZOID_ID;
	ray_edges = std::set<Halfedge, Closer_edge>(Closer_edge(geom_traits, v));
}

class Less_edge : public CGAL::cpp98::binary_function<Halfedge, Halfedge, bool> {
	const Arrangement::Geometry_traits_2 *geom_traits;

  public:
	Less_edge() {}
	Less_edge(const Arrangement::Geometry_traits_2 *traits) : geom_traits(traits) {}
	bool operator()(Halfedge e1, Halfedge e2) const {
		if (undirected_eq(e1, e2))
			return false;
		else
			return &(*e1) < &(*e2);
	}
};

template <typename OP> static void foreach_vertex_edge(Vertex v, OP op) {
	auto edge = v->incident_halfedges();
	for (auto edges_end = edge;;) {
		auto directed_edge = edge->source() == v ? edge : edge->twin();
		assert(directed_edge->source() == v);
		op(directed_edge);
		if (++edge == edges_end)
			break;
	}
}

static void create_arrangement(Arrangement &arr, const std::vector<Point> &points) {
	if (points.size() == 0)
		throw std::invalid_argument("no points provided");

	std::vector<Segment> segments;
	for (unsigned int i = 0; i < points.size(); i++) {
		unsigned int j = i != points.size() - 1 ? i + 1 : 0;
		Segment s(points[i], points[j]);
		segments.push_back(s);
	}

	Point prev = segments[0].start();
	for (unsigned int i = 0; i < segments.size(); i++) {
		auto &segment = segments[i];
		if (segment.start() == segment.end())
			throw std::invalid_argument("zero length segment");
		if (segment.start() != prev)
			throw std::invalid_argument("non continuous segments");
		for (unsigned int j = 0; j < segments.size(); j++) {
			if (CGAL::do_intersect(segment, segments[j])) {
				unsigned int i1 = i != 0 ? i - 1 : segments.size() - 1;
				unsigned int i2 = i != segments.size() - 1 ? i + 1 : 0;
				if (j != i1 && j != i && j != i2) {
					std::ostringstream oss;
					oss << "segments intersect: " << segment << " " << segments[j];
					throw std::invalid_argument(oss.str());
				}
			}
		}
		prev = segment.end();
	}
	if (segments.begin()->start() != segments.back().end())
		throw std::invalid_argument("segments doesn't close a cycle");

	insert(arr, segments.begin(), segments.end());

	// Validate all vertices have a degree of 2
	for (auto v = arr.vertices_begin(); v != arr.vertices_end(); ++v)
		if (v->degree() != 2)
			throw std::invalid_argument("Invalid vertex degree, expected 2.");

	// Expecting one bounded and one unbounded faces. The bounded face is the
	// interiour of the room.
	std::set<Face> faces;
	for (auto v = arr.vertices_begin(); v != arr.vertices_end(); ++v)
		foreach_vertex_edge(v, [&faces](const auto &e) { faces.insert(e->face()); });
	if (faces.size() != 2)
		throw std::invalid_argument("Invalid faces number, expected 2.");

	// Validate no zero width edges exists
	for (auto v = arr.vertices_begin(); v != arr.vertices_end(); ++v)
		foreach_vertex_edge(v, [](const auto &e) {
			if (is_free(e->face()) ^ is_free(e->face()))
				throw std::invalid_argument("zero width edges are not supported");
		});
}

enum MinMax {
	Min,
	Max,
};

static bool find_edge_relative_to_angle(Vertex v, const Direction &angle, enum HalfPlaneSide side, enum MinMax min_max,
										Halfedge &res) {
	bool found = false;
	Halfedge best;
	foreach_vertex_edge(v, [&v, &angle, side, min_max, &found, &best](const Halfedge &edge) {
		auto calc_edge_angle = [&v](const auto &e) {
			Point target = (e->source() == v ? e->target() : e->source())->point();
			Point vp = v->point();
			return Direction(target.hx() - vp.hx(), target.hy() - vp.hy());
		};
		auto e_angle = calc_edge_angle(edge);

		if (side == calc_half_plane_side(angle, e_angle)) {
			auto min_max_side = min_max == MinMax::Max ? HalfPlaneSide::Left : HalfPlaneSide::Right;
			if (!found || calc_half_plane_side(calc_edge_angle(best), e_angle) == min_max_side) {
				best = edge;
				found = true;
			}
		}
	});
	if (found)
		res = best;
	return found;
}

static bool find_vertex_left_edge_with_min_angle(Vertex v, Halfedge &res) {
	return find_edge_relative_to_angle(v, Direction(0, 1), HalfPlaneSide::Left, MinMax::Min, res);
}

static bool find_vertex_left_edge_with_max_angle(Vertex v, Halfedge &res) {
	return find_edge_relative_to_angle(v, Direction(0, 1), HalfPlaneSide::Left, MinMax::Max, res);
}

static bool is_same_direction(Direction d1, Direction d2) {
	return calc_half_plane_side(d1, d2) == HalfPlaneSide::None && d1.vector() * d2.vector() > 0;
}

class Event {
  public:
	Vertex v1;
	Vertex v2;

	Event(Vertex v1, Vertex v2) : v1(v1), v2(v2) {}

	Direction get_angle_vector() const {
		const Point &p1 = v1->point(), &p2 = v2->point();
		return Direction(p2.hx() - p1.hx(), p2.hy() - p1.hy());
	}
};

static Halfedge direct_edge_free_face(const Halfedge &edge) { return is_free(edge->face()) ? edge : edge->twin(); }

TrapezoidID Trapezoider::create_trapezoid(const Halfedge &top_edge, const Halfedge &bottom_edge,
										  const Vertex &left_vertex, const Vertex &right_vertex) {
	TrapezoidID t_id = ++trapezoids_id_counter;
	auto top_edge_d = direct_edge_free_face(top_edge), bottom_edge_d = direct_edge_free_face(bottom_edge);
	Trapezoid trapezoid(t_id, top_edge_d, bottom_edge, left_vertex, right_vertex);
	trapezoids[t_id] = trapezoid;

	auto &left_v_data = vertices_data[trapezoid.left_vertex];
	auto &right_v_data = vertices_data[trapezoid.right_vertex];

	// update trapezoid limiting vertices data
	bool left_on_top = trapezoid.top_edge->curve().line().has_on(trapezoid.left_vertex->point());
	bool left_on_bottom = trapezoid.bottom_edge->curve().line().has_on(trapezoid.left_vertex->point());
	if (!left_on_top || left_on_bottom)
		left_v_data.top_right_trapezoid = t_id;
	if (!left_on_bottom || left_on_top)
		left_v_data.bottom_right_trapezoid = t_id;

	bool right_on_top = trapezoid.top_edge->curve().line().has_on(trapezoid.right_vertex->point());
	bool right_on_bottom = trapezoid.bottom_edge->curve().line().has_on(trapezoid.right_vertex->point());
	if (!right_on_top || right_on_bottom)
		right_v_data.top_left_trapezoid = t_id;
	if (!right_on_bottom || right_on_top)
		right_v_data.bottom_left_trapezoid = t_id;

	return t_id;
}

void Trapezoider::finalize_trapezoid(const Trapezoid &trapezoid) {
	auto &left_v_data = vertices_data[trapezoid.left_vertex];
	auto &right_v_data = vertices_data[trapezoid.right_vertex];

	bool left_on_top = trapezoid.top_edge->curve().line().has_on(trapezoid.left_vertex->point());
	bool left_on_bottom = trapezoid.bottom_edge->curve().line().has_on(trapezoid.left_vertex->point());
	if (!left_on_top || left_on_bottom)
		left_v_data.top_right_trapezoid = INVALID_TRAPEZOID_ID;
	if (!left_on_bottom || left_on_top)
		left_v_data.bottom_right_trapezoid = INVALID_TRAPEZOID_ID;

	bool right_on_top = trapezoid.top_edge->curve().line().has_on(trapezoid.right_vertex->point());
	bool right_on_bottom = trapezoid.bottom_edge->curve().line().has_on(trapezoid.right_vertex->point());
	if (!right_on_top || right_on_bottom)
		right_v_data.top_left_trapezoid = INVALID_TRAPEZOID_ID;
	if (!right_on_bottom || right_on_top)
		right_v_data.bottom_left_trapezoid = INVALID_TRAPEZOID_ID;
}

void Trapezoider::init_trapezoids_with_regular_vertical_decomposition() {
	std::vector<Vertex> vertices;
	std::map<Vertex, DecompVertexData> decomp;
	vertical_decomposition(arr, vertices, decomp);

	std::map<Halfedge, Vertex, Less_edge> most_right_vertex(Less_edge(arr.geometry_traits()));
	for (const auto &v : vertices) {
		const DecompVertexData &v_decomp_data = decomp[v];
		Halfedge top_edge, bottom_edge;

		if ((v_decomp_data.is_edge_above && is_free(v_decomp_data.edge_above->face())) &&
			(v_decomp_data.is_edge_below && is_free(v_decomp_data.edge_below->face())) &&
			!find_vertex_left_edge_with_min_angle(v, top_edge)) {

			// Reflex (more than 180 degrees) vertex
			DEBUG_PRINT("trapezoid from reflex vertex " << v->point() << std::endl);
			auto left_v = most_right_vertex[v_decomp_data.edge_above];
			auto &trapezoid =
				trapezoids[create_trapezoid(v_decomp_data.edge_above, v_decomp_data.edge_below, left_v, v)];

			most_right_vertex[trapezoid.top_edge] = v;

		} else if ((!v_decomp_data.is_edge_above || !is_free(v_decomp_data.edge_above->face())) &&
				   (!v_decomp_data.is_edge_below || !is_free(v_decomp_data.edge_below->face())) &&
				   find_vertex_left_edge_with_min_angle(v, top_edge) &&
				   find_vertex_left_edge_with_max_angle(v, bottom_edge)) {

			// Triangle cell terminates at v
			DEBUG_PRINT("triangle trapezoid " << v->point() << std::endl);
			auto left_v = most_right_vertex[top_edge];
			create_trapezoid(top_edge, bottom_edge, left_v, v);

		} else {
			if (v_decomp_data.is_edge_above && is_free(v_decomp_data.edge_above->face())) {

				// Edge above the vertex
				DEBUG_PRINT("trapezoid up " << v->point() << std::endl);
				if (!find_vertex_left_edge_with_min_angle(v, bottom_edge))
					throw std::logic_error("find_vertex_left_edge_with_min_angle fail");

				auto left_v = most_right_vertex[v_decomp_data.edge_above];
				auto &trapezoid = trapezoids[create_trapezoid(v_decomp_data.edge_above, bottom_edge, left_v, v)];
				most_right_vertex[trapezoid.top_edge] = v;
			}

			if (v_decomp_data.is_edge_below && is_free(v_decomp_data.edge_below->face())) {

				// Edge below the vertex
				DEBUG_PRINT("trapezoid down " << v->point() << std::endl);
				if (!find_vertex_left_edge_with_max_angle(v, top_edge))
					throw std::logic_error("find_vertex_left_edge_with_max_angle fail");

				Vertex left_v = most_right_vertex[top_edge];
				create_trapezoid(top_edge, v_decomp_data.edge_below, left_v, v);
			}
		}

		foreach_vertex_edge(v, [&v, &most_right_vertex](const auto &edge) { most_right_vertex[edge] = v; });
	}

	// TODO remove
	DEBUG_PRINT("After regular vertical decomposition, trapezoids:" << std::endl);
	for (const auto &t : trapezoids) {
		const auto &trapezoid = t.second;
		DEBUG_PRINT("\tT" << trapezoid.get_id() << " Top(" << trapezoid.top_edge->curve() << ") Bottom("
						  << trapezoid.bottom_edge->curve() << ") Left(" << trapezoid.left_vertex->point() << ") Right("
						  << trapezoid.right_vertex->point() << ")" << std::endl);
	}
	DEBUG_PRINT("After regular vertical decomposition, vertices_data:" << std::endl);
	for (const auto &v_pair : vertices_data) {
		const auto &v = v_pair.first;
		const auto &v_data = v_pair.second;
		DEBUG_PRINT("\t(" << v->point() << "):"
						  << " TopLeftT " << (int)v_data.top_left_trapezoid << " TopRightT "
						  << (int)v_data.top_right_trapezoid << " BottomLeftT " << (int)v_data.bottom_left_trapezoid
						  << " BottomRightT " << (int)v_data.bottom_right_trapezoid << std::endl);
	}
}

static bool get_edge(Vertex source, Vertex target, Halfedge &res) {
	// TODO better to implement this as hashmap
	bool found = false;
	foreach_vertex_edge(source, [&target, &res, &found](const auto &edge) {
		if (edge->target() == target) {
			res = edge;
			found = true;
		}
	});
	return found;
}

static int _debug_get_event_angle(const Event &event) {
	double v1x = event.v1->point().hx().exact().convert_to<double>();
	double v1y = event.v1->point().hy().exact().convert_to<double>();
	double v2x = event.v2->point().hx().exact().convert_to<double>();
	double v2y = event.v2->point().hy().exact().convert_to<double>();
	return (int)(atan2(v2y - v1y, v2x - v1x) * 180 / M_PI);
}

static const char *_debug_plane_side_tostr(HalfPlaneSide s) {
	switch (s) {
	case HalfPlaneSide::Left:
		return "left";
	case HalfPlaneSide::Right:
		return "right";
	case HalfPlaneSide::None:
		return "none";
	default:
		assert(false);
	}
}

static const char *_debug_print_trapezoid(const Trapezoid &trapezoid) {
	DEBUG_PRINT("Top(" << trapezoid.top_edge->curve() << ") Bottom(" << trapezoid.bottom_edge->curve() << ") Left("
					   << trapezoid.left_vertex->point() << ") Right(" << trapezoid.right_vertex->point() << ")");
	return "";
}

void Trapezoider::calc_trapezoids_with_rotational_sweep() {
	// Calc all events
	std::vector<Event> events;
	events.reserve(arr.number_of_vertices() * arr.number_of_vertices());
	for (auto v1 = arr.vertices_begin(); v1 != arr.vertices_end(); ++v1)
		for (auto v2 = arr.vertices_begin(); v2 != arr.vertices_end(); ++v2)
			if (v1 != v2)
				events.push_back(Event(v1, v2));

	// Sort events by their angle
	sort(events.begin(), events.end(), [](const Event &e1, const Event &e2) {
		auto a1 = e1.get_angle_vector(), a2 = e2.get_angle_vector();
		if (a1 == a2)
			return false;

		if (is_same_direction(a1, a2)) {
			// Both are exactly on the same angle, consider further vertices first
			return a1.dx() * a1.dx() + a1.dy() * a1.dy() > a2.dx() * a2.dx() + a2.dy() * a2.dy();
		}

		Direction y_axis(0, 1);
		HalfPlaneSide a1_side = calc_half_plane_side(y_axis, a1);
		HalfPlaneSide a2_side = calc_half_plane_side(y_axis, a2);

		if (a1_side == HalfPlaneSide::None)
			return a2_side == HalfPlaneSide::Right || (a1.dy() >= 0 && (a2_side == HalfPlaneSide::Left || a2.dy() < 0));
		if (a2_side == HalfPlaneSide::None)
			return a1_side == HalfPlaneSide::Left && a2.dy() < 0;
		if (a1_side == HalfPlaneSide::Left)
			return a2_side == HalfPlaneSide::Right || calc_half_plane_side(a2, a1) == HalfPlaneSide::Right;
		// a1_side == HalfPlaneSide::Right
		return a2_side == HalfPlaneSide::Right && calc_half_plane_side(a2, a1) == HalfPlaneSide::Right;
	});

	DEBUG_PRINT("Events:" << std::endl);
	for (const auto &event : events)
		DEBUG_PRINT("\t(" << event.v1->point() << ") (" << event.v2->point()
						  << ") angle= " << _debug_get_event_angle(event) << std::endl);

	for (Event &event : events) {
		assert(vertices_data.find(event.v1) != vertices_data.end());
		const auto ray = event.get_angle_vector();
		auto &ray_edges = vertices_data.find(event.v1)->second.ray_edges;
		bool closest_edge_orig_valid;
		Halfedge closest_edge_orig;
		if (closest_edge_orig_valid = ray_edges.size() > 0)
			closest_edge_orig = *ray_edges.begin();

		DEBUG_PRINT(std::endl
					<< "Handle event (" << event.v1->point() << ") (" << event.v2->point() << ")" << std::endl);

		// Maintaine ray_edges
		std::vector<Halfedge> edges_to_insert;
		std::vector<Halfedge> edges_to_remove;
		foreach_vertex_edge(event.v2, [ray, &edges_to_insert, &edges_to_remove](const auto &edge) {
			auto s = edge->source()->point(), t = edge->target()->point();
			Direction edge_direction(t.hx() - s.hx(), t.hy() - s.hy());

			if (calc_half_plane_side(ray, edge_direction) == HalfPlaneSide::Left)
				edges_to_insert.push_back(edge);
			else
				edges_to_remove.push_back(edge);
		});
		for (const auto &edge : edges_to_remove)
			ray_edges.erase(edge);
		for (const auto &edge : edges_to_insert)
			ray_edges.insert(edge);

		auto current_angle = ray;
		VertexData &v1_data = vertices_data.find(event.v1)->second;
		VertexData &v2_data = vertices_data.find(event.v2)->second;

		Halfedge v1v2_edge;
		if (get_edge(event.v1, event.v2, v1v2_edge)) {
			// type 1
			bool left_is_free = is_free(v1v2_edge->face());
			DEBUG_PRINT("Type 1: v1v2_edge " << v1v2_edge->curve() << " left is "
											 << (left_is_free ? "free" : "not free") << std::endl);
			if (left_is_free) {
				assert(v2_data.bottom_left_trapezoid != INVALID_TRAPEZOID_ID);
				assert(v2_data.bottom_right_trapezoid != INVALID_TRAPEZOID_ID);
				Trapezoid &left = trapezoids[v2_data.bottom_left_trapezoid];
				Trapezoid &mid = trapezoids[v2_data.bottom_right_trapezoid];

				assert(left.right_vertex == event.v2);
				assert(mid.left_vertex == event.v2);
				assert(mid.right_vertex == event.v1);

				left.angle_end = current_angle;
				finalize_trapezoid(left);
				mid.angle_end = current_angle;
				finalize_trapezoid(mid);

				DEBUG_PRINT("old left: " << _debug_print_trapezoid(left) << std::endl);
				Trapezoid &left_new =
					trapezoids[create_trapezoid(left.top_edge, left.bottom_edge, left.left_vertex, event.v1)];
				left_new.angle_begin = current_angle;
				DEBUG_PRINT("new left: " << _debug_print_trapezoid(left_new) << std::endl);

				DEBUG_PRINT("old mid: " << _debug_print_trapezoid(mid) << std::endl);
				Trapezoid &mid_new = trapezoids[create_trapezoid(left.top_edge, v1v2_edge, event.v1, event.v2)];
				mid_new.angle_begin = current_angle;
				DEBUG_PRINT("new mid: " << _debug_print_trapezoid(mid_new) << std::endl);

			} else {
				assert(v1_data.top_left_trapezoid != INVALID_TRAPEZOID_ID);
				assert(v1_data.top_right_trapezoid != INVALID_TRAPEZOID_ID);
				Trapezoid &mid = trapezoids[v1_data.top_left_trapezoid];
				Trapezoid &right = trapezoids[v1_data.top_right_trapezoid];

				assert(mid.left_vertex == event.v2);
				assert(mid.right_vertex == event.v1);
				assert(right.left_vertex == event.v1);

				mid.angle_end = current_angle;
				finalize_trapezoid(mid);
				right.angle_end = current_angle;
				finalize_trapezoid(right);

				DEBUG_PRINT("old mid: " << _debug_print_trapezoid(mid) << std::endl);
				Trapezoid &mid_new = trapezoids[create_trapezoid(v1v2_edge, right.bottom_edge, event.v1, event.v2)];
				mid_new.angle_begin = current_angle;
				DEBUG_PRINT("new mid: " << _debug_print_trapezoid(mid_new) << std::endl);

				DEBUG_PRINT("old right: " << _debug_print_trapezoid(right) << std::endl);
				Trapezoid &right_new =
					trapezoids[create_trapezoid(right.top_edge, right.bottom_edge, event.v2, right.right_vertex)];
				right_new.angle_begin = current_angle;
				DEBUG_PRINT("new right: " << _debug_print_trapezoid(right_new) << std::endl);
			}
		} else {
			if (ray_edges.size() == 0)
				continue;
			auto closest_edge = *ray_edges.begin();
			if (closest_edge_orig_valid && closest_edge_orig == closest_edge)
				continue; // Closest edge didn't changed
			DEBUG_PRINT("Type 2: closest edge: " << closest_edge->curve() << " "
												 << (is_free(closest_edge->face()) ? "free" : "non free") << std::endl);
			if (!is_free(closest_edge->face()))
				continue; // The ray is in non free area of the room

			// type 2
			assert(v2_data.bottom_left_trapezoid != INVALID_TRAPEZOID_ID);
			assert(v1_data.top_left_trapezoid != INVALID_TRAPEZOID_ID);
			assert(v1_data.top_left_trapezoid == v2_data.bottom_right_trapezoid);
			assert(v1_data.top_right_trapezoid != INVALID_TRAPEZOID_ID);
			Trapezoid &left = trapezoids[v2_data.bottom_left_trapezoid];
			Trapezoid &mid = trapezoids[v1_data.top_left_trapezoid];
			Trapezoid &right = trapezoids[v1_data.top_right_trapezoid];

			assert(left.right_vertex == event.v2);
			assert(mid.left_vertex == event.v2);
			assert(mid.right_vertex == event.v1);
			assert(right.left_vertex == event.v1);

			left.angle_end = current_angle;
			finalize_trapezoid(left);
			mid.angle_end = current_angle;
			finalize_trapezoid(mid);
			right.angle_end = current_angle;
			finalize_trapezoid(right);

			DEBUG_PRINT("old left: " << _debug_print_trapezoid(left) << std::endl);
			Trapezoid &left_new =
				trapezoids[create_trapezoid(left.top_edge, left.bottom_edge, left.left_vertex, event.v1)];
			left_new.angle_begin = current_angle;
			DEBUG_PRINT("new left: " << _debug_print_trapezoid(left_new) << std::endl);

			DEBUG_PRINT("old mid: " << _debug_print_trapezoid(mid) << std::endl);
			Halfedge bottom_edge;
			if (!find_edge_relative_to_angle(event.v1, current_angle, HalfPlaneSide::Right, MinMax::Max, bottom_edge))
				bottom_edge = right.bottom_edge;
			Trapezoid &mid_new = trapezoids[create_trapezoid(left.top_edge, bottom_edge, event.v1, event.v2)];
			mid_new.angle_begin = current_angle;
			DEBUG_PRINT("new mid: " << _debug_print_trapezoid(mid_new) << std::endl);

			DEBUG_PRINT("old right: " << _debug_print_trapezoid(right) << std::endl);
			Trapezoid &right_new =
				trapezoids[create_trapezoid(right.top_edge, right.bottom_edge, event.v2, right.right_vertex)];
			right_new.angle_begin = current_angle;
			DEBUG_PRINT("new right: " << _debug_print_trapezoid(right_new) << std::endl);
		}
	}

	DEBUG_PRINT("Merge unfinished trapezoids:" << std::endl);
	std::map<std::pair<Vertex, Vertex>, TrapezoidID> no_begin_ts;
	std::map<std::pair<Vertex, Vertex>, TrapezoidID> no_end_ts;
	for (auto &p : trapezoids) {
		Trapezoid &trapezoid = p.second;
		assert(!(trapezoid.angle_begin == ANGLE_NONE && trapezoid.angle_end == ANGLE_NONE));
		if (trapezoid.angle_begin == ANGLE_NONE) {
			assert(no_begin_ts.find({trapezoid.left_vertex, trapezoid.right_vertex}) == no_begin_ts.end());
			no_begin_ts[{trapezoid.left_vertex, trapezoid.right_vertex}] = trapezoid.get_id();
		} else if (trapezoid.angle_end == ANGLE_NONE) {
			assert(no_end_ts.find({trapezoid.left_vertex, trapezoid.right_vertex}) == no_end_ts.end());
			no_end_ts[{trapezoid.left_vertex, trapezoid.right_vertex}] = trapezoid.get_id();
		}
	}
	assert(no_begin_ts.size() == no_end_ts.size());
	for (auto &p : no_begin_ts) {
		const std::pair<Vertex, Vertex> &v = p.first;
		Trapezoid &trapezoid = trapezoids[p.second];
		assert(no_end_ts.find(v) != no_end_ts.end());
		Trapezoid &other = trapezoids[no_end_ts[v]];
		DEBUG_PRINT("\tT" << trapezoid.get_id() << " with T" << other.get_id() << ": "
						  << _debug_print_trapezoid(trapezoid) << std::endl);
		assert(undirected_eq(trapezoid.top_edge, other.top_edge));
		assert(undirected_eq(trapezoid.bottom_edge, other.bottom_edge));
		assert(trapezoid.left_vertex == other.left_vertex);
		assert(trapezoid.right_vertex == other.right_vertex);
		trapezoid.angle_begin = other.angle_begin;
		trapezoids.erase(other.get_id());
	}

	// Remove trapezoids that start and finish at the same angle
	std::vector<TrapezoidID> empty_trapezoids;
	for (const auto &p : trapezoids) {
		const auto &trapezoid = p.second;
		if (is_same_direction(trapezoid.angle_begin, trapezoid.angle_end))
			empty_trapezoids.push_back(p.first);
	}
	for (TrapezoidID empty_trapezoid : empty_trapezoids)
		trapezoids.erase(empty_trapezoid);
}

void Trapezoider::calc_trapezoids(const std::vector<Point> &points, std::vector<Trapezoid> &res) {
	trapezoids.clear();
	vertices_data.clear();
	trapezoids_id_counter = 0;

	arr.clear();
	create_arrangement(arr, points);

	for (auto v = arr.vertices_begin(); v != arr.vertices_end(); ++v)
		vertices_data[v] = VertexData(v->point(), arr.geometry_traits());

	init_trapezoids_with_regular_vertical_decomposition();

	calc_trapezoids_with_rotational_sweep();

	DEBUG_PRINT("After rotational sweep, trapezoids:" << std::endl);
	for (const auto &p : trapezoids) {
		const auto &trapezoid = p.second;
		DEBUG_PRINT("\tT" << trapezoid.get_id() << " ");
		double begin_x = trapezoid.angle_begin.dx().exact().convert_to<double>();
		double begin_y = trapezoid.angle_begin.dy().exact().convert_to<double>();
		double end_x = trapezoid.angle_end.dx().exact().convert_to<double>();
		double end_y = trapezoid.angle_end.dy().exact().convert_to<double>();
		int angle_begin = (int)(atan2(begin_y, begin_x) * 180 / M_PI);
		int angle_end = (int)(atan2(end_y, end_x) * 180 / M_PI);
		DEBUG_PRINT("(" << angle_begin << ", " << angle_end << ")");
		DEBUG_PRINT(" " << _debug_print_trapezoid(trapezoid) << std::endl);
	}

	for (const auto &p : trapezoids)
		res.push_back(p.second);
}