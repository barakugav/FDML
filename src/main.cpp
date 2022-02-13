#include <CGAL/Arr_segment_traits_2.h>
#include <CGAL/Arr_vertical_decomposition_2.h>
#include <CGAL/Arrangement_2.h>
#include <CGAL/Object.h>
#include <CGAL/Polygon_vertical_decomposition_2.h>
#include <CGAL/Rotational_sweep_visibility_2.h>
#include <CGAL/Visibility_2/visibility_utils.h>
#include <CGAL/basic.h>
#include <iostream>
#include <set>
#include <type_traits>

typedef CGAL::Exact_predicates_exact_constructions_kernel Kernel;
typedef CGAL::Segment_2<Kernel> Segment;
typedef Kernel::Point_2 Point;
typedef CGAL::Arr_segment_traits_2<Kernel> Traits;
typedef CGAL::Arrangement_2<Traits> Arrangement;

typedef Arrangement::Vertex_const_handle Vertex_const_handle;
typedef Arrangement::Halfedge_const_handle Halfedge_const_handle;
typedef Arrangement::Face_const_handle Face_const_handle;

typedef CGAL::Rotational_sweep_visibility_2<Arrangement> Rotational_sweep;

// TODO remove
#define DEBUG_PRINT_EN 1
#define DEBUG_PRINT() std::cout << "L" << __LINE__ << std::endl
// #define DEBUG_PRINT() std::cout << __func__ << ":" << __LINE__ << std::endl

static Halfedge_const_handle direct_edge(const Halfedge_const_handle &edge) {
	if (edge->curve().target().hx() != edge->curve().source().hx())
		return edge->curve().target().hx() >= edge->curve().source().hx() ? edge : edge->twin();
	return edge->curve().target().hy() >= edge->curve().source().hy() ? edge : edge->twin();
}

class DecompVertexData {
  public:
	bool is_edge_above;
	bool is_edge_below;
	Halfedge_const_handle edge_above;
	Halfedge_const_handle edge_below;
	DecompVertexData() { is_edge_above = is_edge_below = false; }
};

static void vertical_decomposition(const Arrangement &arr, std::vector<Vertex_const_handle> &vertices,
								   std::map<Vertex_const_handle, DecompVertexData> &decomp) {
	std::vector<std::pair<Vertex_const_handle, std::pair<CGAL::Object, CGAL::Object>>> vd_list;
	CGAL::decompose(arr, std::back_inserter(vd_list));

	std::map<Vertex_const_handle, CGAL::Object> above_orig;
	std::map<Vertex_const_handle, CGAL::Object> below_orig;
	for (auto &decomp_entry : vd_list) {
		above_orig[decomp_entry.first] = decomp_entry.second.second;
		below_orig[decomp_entry.first] = decomp_entry.second.first;
	}

	for (auto &decomp_entry : vd_list)
		vertices.push_back(decomp_entry.first);

	sort(vertices.begin(), vertices.end(), [](const auto &v1, const auto &v2) {
		if (v1->point().hx() != v2->point().hx())
			return v1->point().hx() < v2->point().hx();
		return v1->point().hy() < v2->point().hy();
	});
	for (auto &v : vertices) {
		DecompVertexData v_data;
		Halfedge_const_handle edge;
		bool is_edge;
		for (Vertex_const_handle p = v, up_vertex;; p = up_vertex) {
			auto &above_obj = above_orig[p];
			if (CGAL::assign(edge, above_obj)) {
				v_data.edge_above = direct_edge(edge);
				v_data.is_edge_above = true;
				break;
			}
			if (!CGAL::assign(up_vertex, above_obj))
				break;
		}
		for (Vertex_const_handle p = v, below_vertex;; p = below_vertex) {
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

class Closer_edge : public CGAL::cpp98::binary_function<Halfedge_const_handle, Halfedge_const_handle, bool> {
	typedef Halfedge_const_handle EH;
	typedef Arrangement::Geometry_traits_2 Geometry_traits_2;
	typedef typename Geometry_traits_2::Point_2 Point_2;

	const Geometry_traits_2 *geom_traits;
	Point_2 q;

  public:
	Closer_edge() {}
	Closer_edge(const Geometry_traits_2 *traits, const Point_2 &q) : geom_traits(traits), q(q) {}

	int vtype(const Point_2 &c, const Point_2 &p) const {
		switch (CGAL::Visibility_2::orientation_2(geom_traits, q, c, p)) {
		case CGAL::COLLINEAR:
			if (CGAL::Visibility_2::less_distance_to_point_2(geom_traits, q, c, p))
				return 0;
			else
				return 3;
		case CGAL::RIGHT_TURN:
			return 1;
		case CGAL::LEFT_TURN:
			return 2;
		default:
			CGAL_assume(false);
		}
		return -1;
	}

	bool operator()(const EH &e1, const EH &e2) const {
		if (e1 == e2)
			return false;
		const Point_2 &s1 = e1->source()->point(), t1 = e1->target()->point(), s2 = e2->source()->point(),
					  t2 = e2->target()->point();
		if (e1->source() == e2->source()) {

			int vt1 = vtype(s1, t1), vt2 = vtype(s1, t2);
			if (vt1 != vt2)
				return vt1 > vt2;
			else
				return (CGAL::Visibility_2::orientation_2(geom_traits, s1, t2, t1) ==
						CGAL::Visibility_2::orientation_2(geom_traits, s1, t2, q));
		}

		if (e1->target() == e2->source()) {
			int vt1 = vtype(t1, s1), vt2 = vtype(t1, t2);
			if (vt1 != vt2)
				return vt1 > vt2;
			else
				return (CGAL::Visibility_2::orientation_2(geom_traits, s2, t2, s1) ==
						CGAL::Visibility_2::orientation_2(geom_traits, s2, t2, q));
		}

		if (e1->source() == e2->target()) {
			int vt1 = vtype(s1, t1), vt2 = vtype(s1, s2);
			if (vt1 != vt2)
				return vt1 > vt2;
			else
				return (CGAL::Visibility_2::orientation_2(geom_traits, s1, s2, t1) ==
						CGAL::Visibility_2::orientation_2(geom_traits, s1, s2, q));
		}

		if (e1->target() == e2->target()) {
			int vt1 = vtype(t1, s1), vt2 = vtype(t1, s2);
			if (vt1 != vt2)
				return vt1 > vt2;
			else
				return (CGAL::Visibility_2::orientation_2(geom_traits, t1, s2, s1) ==
						CGAL::Visibility_2::orientation_2(geom_traits, t1, s2, q));
		}

		CGAL::Orientation e1q = CGAL::Visibility_2::orientation_2(geom_traits, s1, t1, q);
		switch (e1q) {
		case CGAL::COLLINEAR:
			if (CGAL::Visibility_2::collinear(geom_traits, q, s2, t2)) {
				// q is collinear with e1 and e2.
				return (CGAL::Visibility_2::less_distance_to_point_2(geom_traits, q, s1, s2) ||
						CGAL::Visibility_2::less_distance_to_point_2(geom_traits, q, t1, t2));
			} else {
				// q is collinear with e1 not with e2.
				if (CGAL::Visibility_2::collinear(geom_traits, s2, t2, s1))
					return (CGAL::Visibility_2::orientation_2(geom_traits, s2, t2, q) ==
							CGAL::Visibility_2::orientation_2(geom_traits, s2, t2, t1));
				else
					return (CGAL::Visibility_2::orientation_2(geom_traits, s2, t2, q) ==
							CGAL::Visibility_2::orientation_2(geom_traits, s2, t2, s1));
			}
			break;
		case CGAL::RIGHT_TURN:
			switch (CGAL::Visibility_2::orientation_2(geom_traits, s1, t1, s2)) {
			case CGAL::COLLINEAR:
				return CGAL::Visibility_2::orientation_2(geom_traits, s1, t1, t2) != e1q;
			case CGAL::RIGHT_TURN:
				if (CGAL::Visibility_2::orientation_2(geom_traits, s1, t1, t2) == CGAL::LEFT_TURN)
					return CGAL::Visibility_2::orientation_2(geom_traits, s2, t2, q) ==
						   CGAL::Visibility_2::orientation_2(geom_traits, s2, t2, s1);
				else
					return false;
			case CGAL::LEFT_TURN:
				if (CGAL::Visibility_2::orientation_2(geom_traits, s1, t1, t2) == CGAL::RIGHT_TURN)
					return CGAL::Visibility_2::orientation_2(geom_traits, s2, t2, q) ==
						   CGAL::Visibility_2::orientation_2(geom_traits, s2, t2, s1);
				else
					return true;
			default:
				CGAL_assume(false);
			}
			break;
		case CGAL::LEFT_TURN:
			switch (CGAL::Visibility_2::orientation_2(geom_traits, s1, t1, s2)) {
			case CGAL::COLLINEAR:
				return CGAL::Visibility_2::orientation_2(geom_traits, s1, t1, t2) != e1q;
			case CGAL::LEFT_TURN:
				if (CGAL::Visibility_2::orientation_2(geom_traits, s1, t1, t2) == CGAL::RIGHT_TURN)
					return CGAL::Visibility_2::orientation_2(geom_traits, s2, t2, q) ==
						   CGAL::Visibility_2::orientation_2(geom_traits, s2, t2, s1);
				else
					return false;
			case CGAL::RIGHT_TURN:
				if (CGAL::Visibility_2::orientation_2(geom_traits, s1, t1, t2) == CGAL::LEFT_TURN)
					return CGAL::Visibility_2::orientation_2(geom_traits, s2, t2, q) ==
						   CGAL::Visibility_2::orientation_2(geom_traits, s2, t2, s1);
				else
					return true;
			default:
				CGAL_assume(false);
			}
		}

		CGAL_assume(false);
		return false;
	}
};

// static const Point ANGLE_NONE(0, 0);
static const std::pair<Point, Point> ANGLE_NONE(Point(0, 0), Point(0, 0));

typedef unsigned int TrapezoidID;

class Trapezoid {
  private:
	TrapezoidID id;

  public:
	// Instead of calculating an actual angle, we represent the angle as a
	// vector
	// Point angle_begin;
	// Point angle_end;
	std::pair<Point, Point> angle_begin;
	std::pair<Point, Point> angle_end;
	Halfedge_const_handle top_edge;
	Halfedge_const_handle bottom_edge;
	Vertex_const_handle left_vertex;
	Vertex_const_handle right_vertex;
	Trapezoid() = default;
	Trapezoid(TrapezoidID id, Halfedge_const_handle top_edge, Halfedge_const_handle bottom_edge,
			  Vertex_const_handle left_vertex, Vertex_const_handle right_vertex)
		: id(id), top_edge(top_edge), bottom_edge(bottom_edge), left_vertex(left_vertex), right_vertex(right_vertex),
		  angle_begin(ANGLE_NONE), angle_end(ANGLE_NONE) {}
	Trapezoid(const Trapezoid &) = default;
	TrapezoidID get_id() const { return id; }
};

#define INVALID_TRAPEZOID_ID -1

static bool undirected_eq(const Halfedge_const_handle &e1, const Halfedge_const_handle &e2) {
	return e1 == e2 || e1 == e2->twin();
}

struct VertexData {
	TrapezoidID top_left_trapezoid;
	TrapezoidID top_right_trapezoid;
	TrapezoidID bottom_left_trapezoid;
	TrapezoidID bottom_right_trapezoid;
	std::set<Halfedge_const_handle, Closer_edge> ray_edges;
	VertexData() {}
	VertexData(Point &v, const Arrangement::Geometry_traits_2 *geom_traits) {
		top_left_trapezoid = top_right_trapezoid = INVALID_TRAPEZOID_ID;
		bottom_left_trapezoid = bottom_right_trapezoid = INVALID_TRAPEZOID_ID;
		ray_edges = std::set<Halfedge_const_handle, Closer_edge>(Closer_edge(geom_traits, v));
	}
};

class Less_edge : public CGAL::cpp98::binary_function<Halfedge_const_handle, Halfedge_const_handle, bool> {
	const Arrangement::Geometry_traits_2 *geom_traits;

  public:
	Less_edge() {}
	Less_edge(const Arrangement::Geometry_traits_2 *traits) : geom_traits(traits) {}
	bool operator()(Halfedge_const_handle e1, Halfedge_const_handle e2) const {
		if (undirected_eq(e1, e2))
			return false;
		else
			return &(*e1) < &(*e2);
	}
};

template <typename OP> static void foreach_vertex_edge(Vertex_const_handle v, OP op) {
	auto edge = v->incident_halfedges();
	for (auto edges_end = edge;;) {
		auto directed_edge = edge->source() == v ? edge : edge->twin();
		assert(directed_edge->source() == v);
		op(directed_edge);
		if (++edge == edges_end)
			break;
	}
}

static void create_arrangement(Arrangement &arr, const std::vector<Segment> &segments) {
	if (segments.size() == 0)
		throw std::invalid_argument("no segments provided");
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

	std::set<Face_const_handle> faces;
	for (auto v = arr.vertices_begin(); v != arr.vertices_end(); ++v)
		foreach_vertex_edge(v, [&faces](const auto &e) { faces.insert(e->face()); });
	if (faces.size() != 2)
		throw std::invalid_argument("Invalid faces number, expected 2.");
	// Expecting one bounded and one unbounded faces. The bounded face is the
	// interiour of the room.

	// for (auto v = arr.vertices_begin(); v != arr.vertices_end(); ++v) {
	// 	foreach_vertex_edge(v, [](const auto &e) {
	// 		std::cout << e->source()->point() << " " << e->target()->point() << " "
	// 				  << (!e->face()->is_unbounded() ? "in" : "out") << std::endl;
	// 		auto e2 = e->twin();
	// 		std::cout << e2->source()->point() << " " << e2->target()->point() << " "
	// 				  << (!e2->face()->is_unbounded() ? "in" : "out") << std::endl;
	// 	});
	// }
	// edge->face() is the face that is LEFT relative to the edge direction.
}

static bool is_free(Face_const_handle face) { return !face->is_unbounded(); }

enum HalfPlaneSide {
	None, // exactly on plane
	Left,
	Right,
};

enum MinMax {
	Min,
	Max,
};

static int sign(Kernel::FT val) { return (Kernel::FT(0) < val) - (val < Kernel::FT(0)); }

static enum HalfPlaneSide calc_half_plane_side(const Point &angle, const Point &p) {
	// determinant of vectors
	int s = sign(angle.hx() * p.hy() - angle.hy() * p.hx());
	return s == -1 ? HalfPlaneSide::Right : s == 1 ? HalfPlaneSide::Left : HalfPlaneSide::None;
}

static bool find_edge_relative_to_angle(Vertex_const_handle v, const Point &angle, enum HalfPlaneSide side,
										enum MinMax min_max, Halfedge_const_handle &res) {
	bool found = false;
	Halfedge_const_handle best;
	// DEBUG_PRINT();
	// std::cout << "Begin V(" << v->point() << ") A(" << angle << ")"
	//           << (side == HalfPlaneSide::Left ? " Left" : " Right")
	//           << (min_max == MinMax::Max ? " Max" : " Min") << std::endl;
	foreach_vertex_edge(v, [&v, &angle, side, min_max, &found, &best](const auto &edge) {
		// std::cout << "Edge: (" << edge->curve() << ")";
		// DEBUG_PRINT();
		auto calc_edge_angle = [&v](const auto &e) {
			auto target = (e->source() == v ? e->target() : e->source())->point();
			auto vp = v->point();
			return Point(target.hx() - vp.hx(), target.hy() - vp.hy());
		};
		// DEBUG_PRINT();
		auto e_angle = calc_edge_angle(edge);
		// auto s = calc_half_plane_side(angle, e_angle);
		// std::cout << " angle= (" << e_angle << ") side= "
		//           << (s == HalfPlaneSide::Left
		//                   ? "Left"
		//                   : s == HalfPlaneSide::Right ? "Right" : "None");

		// DEBUG_PRINT();
		if (side == calc_half_plane_side(angle, e_angle)) {
			// DEBUG_PRINT();
			auto min_max_side = min_max == MinMax::Max ? HalfPlaneSide::Left : HalfPlaneSide::Right;
			// if (found) {
			//   s = calc_half_plane_side(calc_edge_angle(best), e_angle);
			//   std::cout << " angle_side_relative_to_best= "
			//             << (s == HalfPlaneSide::Left
			//                     ? "Left"
			//                     : s == HalfPlaneSide::Right ? "Right" :
			//                     "None");
			// } else
			//   std::cout << " first";
			if (!found || calc_half_plane_side(calc_edge_angle(best), e_angle) == min_max_side) {
				// DEBUG_PRINT();
				best = edge;
				found = true;
			}
		}
		// std::cout << std::endl;h
		// DEBUG_PRINT();
	});
	// DEBUG_PRINT();
	if (found) {
		// std::cout << "Best: (" << best->curve() << ")" << std::endl;
		// DEBUG_PRINT();
		res = best;
	}
	return found;
}

static bool find_vertex_left_edge_with_min_angle(Vertex_const_handle v, Halfedge_const_handle &res) {
	return find_edge_relative_to_angle(v, Point(0, 1), HalfPlaneSide::Left, MinMax::Min, res);
}

static bool find_vertex_left_edge_with_max_angle(Vertex_const_handle v, Halfedge_const_handle &res) {
	return find_edge_relative_to_angle(v, Point(0, 1), HalfPlaneSide::Left, MinMax::Max, res);
}

class Event {
  public:
	Vertex_const_handle v1;
	Vertex_const_handle v2;

	Event(Vertex_const_handle v1, Vertex_const_handle v2) : v1(v1), v2(v2) {}

	Point get_angle_vector() const {
		const auto &p1 = v1->point(), &p2 = v2->point();
		return Point(p2.hx() - p1.hx(), p2.hy() - p1.hy());
	}
};

class Trapezoider {

  private:
	Arrangement arr;
	std::map<TrapezoidID, Trapezoid> trapezoids;
	std::unordered_map<Vertex_const_handle, VertexData> vtrapezoids;
	TrapezoidID trapezoids_id_counter;

  public:
	Trapezoider(const std::vector<Segment> &segments) : trapezoids_id_counter(0) { create_arrangement(arr, segments); }

  private:
	TrapezoidID create_trapezoid(const Halfedge_const_handle &top_edge, const Halfedge_const_handle &bottom_edge,
								 const Vertex_const_handle &left_vertex, const Vertex_const_handle &right_vertex) {

		TrapezoidID t_id = ++trapezoids_id_counter;
		Trapezoid trapezoid(t_id, top_edge, bottom_edge, left_vertex, right_vertex);
		trapezoids[t_id] = trapezoid;

		auto &left_v_data = vtrapezoids[trapezoid.left_vertex];
		auto &right_v_data = vtrapezoids[trapezoid.right_vertex];

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

	void finalize_trapezoid(const Trapezoid &trapezoid) {
		auto &left_v_data = vtrapezoids[trapezoid.left_vertex];
		auto &right_v_data = vtrapezoids[trapezoid.right_vertex];

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

	void init_trapezoids_with_regular_vertical_decomposition() {
		std::vector<Vertex_const_handle> vertices;
		std::map<Vertex_const_handle, DecompVertexData> decomp;
		vertical_decomposition(arr, vertices, decomp);

		std::map<Halfedge_const_handle, Vertex_const_handle, Less_edge> most_right_vertex(
			Less_edge(arr.geometry_traits()));
		for (const auto &v : vertices) {
			const DecompVertexData &v_decomp_data = decomp[v];
			Halfedge_const_handle top_edge, bottom_edge;

			if ((v_decomp_data.is_edge_above && is_free(v_decomp_data.edge_above->face())) &&
				(v_decomp_data.is_edge_below && is_free(v_decomp_data.edge_below->face())) &&
				!find_vertex_left_edge_with_min_angle(v, top_edge)) {

				// Reflex (more than 180 degrees) vertex
				std::cout << "trapezoid from reflex vertex " << v->point() << std::endl;
				Vertex_const_handle left_v = most_right_vertex[v_decomp_data.edge_above];
				auto &trapezoid =
					trapezoids[create_trapezoid(v_decomp_data.edge_above, v_decomp_data.edge_below, left_v, v)];

				most_right_vertex[trapezoid.top_edge] = v;

			} else if ((!v_decomp_data.is_edge_above || !is_free(v_decomp_data.edge_above->face())) &&
					   (!v_decomp_data.is_edge_below || !is_free(v_decomp_data.edge_below->face())) &&
					   find_vertex_left_edge_with_min_angle(v, top_edge) &&
					   find_vertex_left_edge_with_max_angle(v, bottom_edge)) {

				// Triangle cell terminates at v
				std::cout << "triangle trapezoid " << v->point() << std::endl;
				Vertex_const_handle left_v = most_right_vertex[top_edge];
				create_trapezoid(top_edge, bottom_edge, left_v, v);

			} else {
				if (v_decomp_data.is_edge_above && is_free(v_decomp_data.edge_above->face())) {

					// Edge above the vertex
					std::cout << "trapezoid up " << v->point() << std::endl;
					if (!find_vertex_left_edge_with_min_angle(v, bottom_edge))
						throw std::logic_error("find_vertex_left_edge_with_min_angle fail");

					Vertex_const_handle left_v = most_right_vertex[v_decomp_data.edge_above];
					auto &trapezoid = trapezoids[create_trapezoid(v_decomp_data.edge_above, bottom_edge, left_v, v)];
					most_right_vertex[trapezoid.top_edge] = v;
				}

				if (v_decomp_data.is_edge_below && is_free(v_decomp_data.edge_below->face())) {

					// Edge below the vertex
					std::cout << "trapezoid down " << v->point() << std::endl;
					if (!find_vertex_left_edge_with_max_angle(v, top_edge))
						throw std::logic_error("find_vertex_left_edge_with_max_angle fail");

					Vertex_const_handle left_v = most_right_vertex[top_edge];
					create_trapezoid(top_edge, v_decomp_data.edge_below, left_v, v);
				}
			}

			foreach_vertex_edge(v, [&v, &most_right_vertex](const auto &edge) { most_right_vertex[edge] = v; });
		}

		// TODO remove
		std::cout << "After regular vertical decomposition, trapezoids:" << std::endl;
		for (const auto &t : trapezoids) {
			const auto &trapezoid = t.second;
			std::cout << "\tT" << trapezoid.get_id() << " Top(" << trapezoid.top_edge->curve() << ") Bottom("
					  << trapezoid.bottom_edge->curve() << ") Left(" << trapezoid.left_vertex->point() << ") Right("
					  << trapezoid.right_vertex->point() << ")" << std::endl;
		}
		std::cout << "After regular vertical decomposition, vtrapezoids:" << std::endl;
		for (const auto &v_pair : vtrapezoids) {
			const auto &v = v_pair.first;
			const auto &v_data = v_pair.second;
			std::cout << "\t(" << v->point() << "):"
					  << " TopLeftT " << (int)v_data.top_left_trapezoid << " TopRightT "
					  << (int)v_data.top_right_trapezoid << " BottomLeftT " << (int)v_data.bottom_left_trapezoid
					  << " BottomRightT " << (int)v_data.bottom_right_trapezoid << std::endl;
		}
	}

	static bool get_edge(Vertex_const_handle source, Vertex_const_handle target, Halfedge_const_handle &res) {
		bool found = false;
		foreach_vertex_edge(source, [&target, &res, &found](const auto &edge) {
			if (edge->target() == target) {
				res = edge;
				found = true;
			}
		});
		return found;
	}

	static int get_event_angle(const Event &event) {
		double v1x = event.v1->point().hx().exact().convert_to<double>();
		double v1y = event.v1->point().hy().exact().convert_to<double>();
		double v2x = event.v2->point().hx().exact().convert_to<double>();
		double v2y = event.v2->point().hy().exact().convert_to<double>();
		return (int)(atan2(v2y - v1y, v2x - v1x) * 180 / M_PI);
	}

	static const char *plane_side_tostr(HalfPlaneSide s) {
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

	static const char *print_trapezoid(const Trapezoid &trapezoid) {
		std::cout << "Top(" << trapezoid.top_edge->curve() << ") Bottom(" << trapezoid.bottom_edge->curve() << ") Left("
				  << trapezoid.left_vertex->point() << ") Right(" << trapezoid.right_vertex->point() << ")";
		return "";
	}

	void calc_trapezoids_with_rotational_sweep() {
		// Calc all events
		std::vector<Event> events;
		events.reserve(arr.number_of_vertices() * arr.number_of_vertices());
		for (auto v1 = arr.vertices_begin(); v1 != arr.vertices_end(); ++v1)
			for (auto v2 = arr.vertices_begin(); v2 != arr.vertices_end(); ++v2)
				if (v1 != v2)
					events.push_back(Event(v1, v2));

		// Sort events by their angle
		// std::cout << "Events sort:" << std::endl;
		sort(events.begin(), events.end(), [](const Event &e1, const Event &e2) {
			auto a1 = e1.get_angle_vector(), a2 = e2.get_angle_vector();
			if (a1 == a2)
				return false;
			Point y_axis(0, 1);
			HalfPlaneSide a1_side = calc_half_plane_side(y_axis, a1);
			HalfPlaneSide a2_side = calc_half_plane_side(y_axis, a2);

			// std::cout << std::endl;
			// std::cout << "\t(" << e1.v1->point() << ") (" << e1.v2->point() << ") angle= " << get_event_angle(e1)
			// 		  << " side= " << plane_side_tostr(a1_side) << " angle vector= " << a1 << std::endl;
			// std::cout << "\t(" << e2.v1->point() << ") (" << e2.v2->point() << ") angle= " << get_event_angle(e2)
			// 		  << " side= " << plane_side_tostr(a2_side) << " angle vector= " << a2 << std::endl;

			if (a1_side == HalfPlaneSide::None)
				return a2_side == HalfPlaneSide::Right ||
					   (a1.hy() >= 0 && (a2_side == HalfPlaneSide::Left || a2.hy() < 0));
			if (a2_side == HalfPlaneSide::None)
				return a1_side == HalfPlaneSide::Left && a2.hy() < 0;
			if (a1_side == HalfPlaneSide::Left)
				return a2_side == HalfPlaneSide::Right || calc_half_plane_side(a2, a1) == HalfPlaneSide::Right;
			// a1_side == HalfPlaneSide::Right
			return a2_side == HalfPlaneSide::Right && calc_half_plane_side(a2, a1) == HalfPlaneSide::Right;
		});

		std::cout << "Events:" << std::endl;
		for (const auto &event : events)
			std::cout << "\t(" << event.v1->point() << ") (" << event.v2->point()
					  << ") angle= " << get_event_angle(event) << std::endl;

		// TODO init  ray_edges with right edges

		for (auto &event : events) {
			assert(vtrapezoids.find(event.v1) != vtrapezoids.end());
			const auto ray = event.get_angle_vector();
			auto &ray_edges = vtrapezoids.find(event.v1)->second.ray_edges;
			auto closest_edge_ptr_orig = ray_edges.begin();

			std::cout << std::endl;
			std::cout << "Handle event (" << event.v1->point() << ") (" << event.v2->point() << ")" << std::endl;
			// std::cout << "Ray before event:" << std::endl;
			// for (const auto &edge : ray_edges)
			// 	std::cout << "\t(" << edge->curve() << ")" << std::endl;

			// Maintaine ray_edges
			std::vector<Halfedge_const_handle> edges_to_insert;
			std::vector<Halfedge_const_handle> edges_to_remove;
			// std::cout << "ray_edges:" << std::endl;
			foreach_vertex_edge(
				event.v2, [ray, /* TODO ray_edges */ &ray_edges, &edges_to_insert, &edges_to_remove](const auto &edge) {
					auto s = edge->source()->point(), t = edge->target()->point();
					Point edge_direction(t.hx() - s.hx(), t.hy() - s.hy());

					if (calc_half_plane_side(ray, edge_direction) == HalfPlaneSide::Left) {
						// std::cout << "\tAdd (" << edge->curve() << ")" << std::endl;
						edges_to_insert.push_back(edge);
					} else {
						if (ray_edges.find(edge) != ray_edges.end())
							; // std::cout << "\tRemove (" << edge->curve() << ")" << std::endl;
						edges_to_remove.push_back(edge);
					}
				});
			for (const auto &edge : edges_to_remove)
				ray_edges.erase(edge);
			for (const auto &edge : edges_to_insert)
				ray_edges.insert(edge);
			// std::cout << "Ray after event:" << std::endl;
			// for (const auto &edge : ray_edges)
			// 	std::cout << "\t(" << edge->curve() << ")" << std::endl;

			// auto current_angle = event.get_angle_vector();
			std::pair<Point, Point> current_angle(event.v1->point(), event.v2->point());
			auto angle_vec = event.get_angle_vector();
			VertexData &v1_data = vtrapezoids.find(event.v1)->second;
			VertexData &v2_data = vtrapezoids.find(event.v2)->second;

			Halfedge_const_handle v1v2_edge;
			if (get_edge(event.v1, event.v2, v1v2_edge)) {
				// type 1
				bool left_is_free = is_free(v1v2_edge->face());
				std::cout << "Type 1: v1v2_edge " << v1v2_edge->curve() << " left is "
						  << (left_is_free ? "free" : "not free") << std::endl;
				if (left_is_free) {
					// assert(v1_data.top_left_trapezoid == INVALID_TRAPEZOID_ID);
					// assert(v1_data.top_right_trapezoid == INVALID_TRAPEZOID_ID);
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

					std::cout << "old left: " << print_trapezoid(left) << std::endl;
					auto &left_new =
						trapezoids[create_trapezoid(left.top_edge, left.bottom_edge, left.left_vertex, event.v1)];
					left_new.angle_begin = current_angle;
					std::cout << "new left: " << print_trapezoid(left_new) << std::endl;

					std::cout << "old mid: " << print_trapezoid(mid) << std::endl;
					auto &mid_new = trapezoids[create_trapezoid(left.top_edge, v1v2_edge, event.v1, event.v2)];
					mid_new.angle_begin = current_angle;
					std::cout << "new mid: " << print_trapezoid(mid_new) << std::endl;

				} else {
					assert(v1_data.top_left_trapezoid != INVALID_TRAPEZOID_ID);
					assert(v1_data.top_right_trapezoid != INVALID_TRAPEZOID_ID);
					// assert(v2_data.bottom_left_trapezoid == INVALID_TRAPEZOID_ID);
					// assert(v2_data.bottom_right_trapezoid == INVALID_TRAPEZOID_ID);
					Trapezoid &mid = trapezoids[v1_data.top_left_trapezoid];
					Trapezoid &right = trapezoids[v1_data.top_right_trapezoid];

					assert(mid.left_vertex == event.v2);
					assert(mid.right_vertex == event.v1);
					assert(right.left_vertex == event.v1);

					mid.angle_end = current_angle;
					finalize_trapezoid(mid);
					right.angle_end = current_angle;
					finalize_trapezoid(right);

					std::cout << "old mid: " << print_trapezoid(mid) << std::endl;
					auto &mid_new = trapezoids[create_trapezoid(v1v2_edge, right.bottom_edge, event.v1, event.v2)];
					mid_new.angle_begin = current_angle;
					std::cout << "new mid: " << print_trapezoid(mid_new) << std::endl;

					std::cout << "old right: " << print_trapezoid(right) << std::endl;
					auto &right_new =
						trapezoids[create_trapezoid(right.top_edge, right.bottom_edge, event.v2, right.right_vertex)];
					right_new.angle_begin = current_angle;
					std::cout << "new right: " << print_trapezoid(right_new) << std::endl;
				}
			} else {
				if (ray_edges.size() == 0)
					continue;
				auto closest_edge_ptr = ray_edges.begin();
				if (closest_edge_ptr_orig == closest_edge_ptr)
					continue; // Closest edge didn't changed
				auto closest_edge = *closest_edge_ptr;
				std::cout << "Type 2: closest edge: " << closest_edge->curve() << " "
						  << (is_free(closest_edge->face()) ? "free" : "non free") << std::endl;
				if (!is_free(closest_edge->face())) // TODO validate this works
					continue;						// The ray is in non free area of the room

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

				std::cout << "old left: " << print_trapezoid(left) << std::endl;
				auto &left_new =
					trapezoids[create_trapezoid(left.top_edge, left.bottom_edge, left.left_vertex, event.v1)];
				left_new.angle_begin = current_angle;
				std::cout << "new left: " << print_trapezoid(left_new) << std::endl;

				std::cout << "old mid: " << print_trapezoid(mid) << std::endl;
				Halfedge_const_handle bottom_edge;
				// if (!find_edge_relative_to_angle(event.v1, current_angle, HalfPlaneSide::Right, MinMax::Max,
				// 								 bottom_edge))
				if (!find_edge_relative_to_angle(event.v1, angle_vec, HalfPlaneSide::Right, MinMax::Max, bottom_edge))
					bottom_edge = right.bottom_edge;
				auto &mid_new = trapezoids[create_trapezoid(left.top_edge, bottom_edge, event.v1, event.v2)];
				mid_new.angle_begin = current_angle;
				std::cout << "new mid: " << print_trapezoid(mid_new) << std::endl;

				std::cout << "old right: " << print_trapezoid(right) << std::endl;
				auto &right_new =
					trapezoids[create_trapezoid(right.top_edge, right.bottom_edge, event.v2, right.right_vertex)];
				right_new.angle_begin = current_angle;
				std::cout << "new right: " << print_trapezoid(right_new) << std::endl;
			}
		}

		std::cout << "Merge unfinished trapezoids:" << std::endl;
		std::map<std::pair<Vertex_const_handle, Vertex_const_handle>, TrapezoidID> no_begin_ts;
		std::map<std::pair<Vertex_const_handle, Vertex_const_handle>, TrapezoidID> no_end_ts;
		for (auto &p : trapezoids) {
			auto &trapezoid = p.second;
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
			const std::pair<Vertex_const_handle, Vertex_const_handle> &v = p.first;
			auto &trapezoid = trapezoids[p.second];
			assert(no_end_ts.find(v) != no_end_ts.end());
			auto &other = trapezoids[no_end_ts[v]];
			std::cout << "\tT" << trapezoid.get_id() << " with T" << other.get_id() << ": "
					  << print_trapezoid(trapezoid) << std::endl;
			assert(undirected_eq(trapezoid.top_edge, other.top_edge));
			assert(undirected_eq(trapezoid.bottom_edge, other.bottom_edge));
			assert(trapezoid.left_vertex == other.left_vertex);
			assert(trapezoid.right_vertex == other.right_vertex);
			trapezoid.angle_begin = other.angle_end;
			trapezoids.erase(other.get_id());
		}
	}

  public:
	int calc_trapezoids() {
		trapezoids.clear();
		vtrapezoids.clear();

		for (auto v = arr.vertices_begin(); v != arr.vertices_end(); ++v)
			vtrapezoids[v] = VertexData(v->point(), arr.geometry_traits());

		init_trapezoids_with_regular_vertical_decomposition();
		// throw std::logic_error("enough for now");

		std::cout << std::endl << std::endl;
		calc_trapezoids_with_rotational_sweep();

		std::cout << "After rotational sweep, trapezoids:" << std::endl;
		for (const auto &p : trapezoids) {
			const auto &trapezoid = p.second;
			printf("\tT%02u ", trapezoid.get_id());
			// double begin_x = trapezoid.angle_begin.hx().exact().convert_to<double>();
			// double begin_y = trapezoid.angle_begin.hy().exact().convert_to<double>();
			// double end_x = trapezoid.angle_end.hx().exact().convert_to<double>();
			// double end_y = trapezoid.angle_end.hy().exact().convert_to<double>();
			// unsigned int angle_begin = (int)(atan2(begin_y, begin_x) * 180 / M_PI);
			// unsigned int angle_end = (int)(atan2(end_y, end_x) * 180 / M_PI);
			// printf("(% 4d, % 4d)", angle_begin, angle_end);
			std::cout << "((" << trapezoid.angle_begin.first << ", " << trapezoid.angle_begin.second << "), ("
					  << trapezoid.angle_end.first << ", " << trapezoid.angle_end.second << "))";
			std::cout << " " << print_trapezoid(trapezoid) << std::endl;
		}

		return 0;
	}
};

static void create_segments_from_points(const std::vector<Point> &points, std::vector<Segment> &segments) {
	for (unsigned int i = 0; i < points.size(); i++) {
		unsigned int j = i != points.size() - 1 ? i + 1 : 0;
		Segment s(points[i], points[j]);
		segments.push_back(s);
	}
}

int main() {
	// std::vector<Point> points = {
	// 	Point(0, 0), Point(2, 3), Point(5, 4), Point(4, 2), Point(5, 0),
	// };
	std::vector<Point> points = {
		Point(1, 1), Point(3, 4), Point(6, 5), Point(5, 3), Point(6, 1),
	};
	std::vector<Segment> segments;
	create_segments_from_points(points, segments);

	Trapezoider trapezoider(segments);
	trapezoider.calc_trapezoids();

	return 0;
}
