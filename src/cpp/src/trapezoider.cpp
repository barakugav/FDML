#include "trapezoider.h"
#include "utils.hpp"
#include <CGAL/Arr_vertical_decomposition_2.h>

/* An event that should be handled during a parallel rotational sweep. The event is composed as two vertices that align
 * on the same line for some angle. */
class Event {
  public:
	/* the ray base vertex */
	Vertex v1;
	/* the ray end vertex */
	Vertex v2;

	Event(const Vertex &v1, const Vertex &v2) : v1(v1), v2(v2) {}

	Direction get_ray() const {
		const Point &p1 = v1->point(), &p2 = v2->point();
		return Direction(p2.hx() - p1.hx(), p2.hy() - p1.hy());
	}
};

/* return an edge or it's twin, always such source.x <= target.x && (source.x != tagert.x && source.y <= target.y) */
static Halfedge direct_edge(const Halfedge &edge) {
	if (edge->curve().target().hx() != edge->curve().source().hx())
		return edge->curve().target().hx() >= edge->curve().source().hx() ? edge : edge->twin();
	return edge->curve().target().hy() >= edge->curve().source().hy() ? edge : edge->twin();
}

/* perform an operation on all edges coming out of a vertex */
template <typename OP> static void foreach_vertex_edge(const Vertex &v, const OP &op) {
	auto edge = v->incident_halfedges();
	for (auto edges_end = edge;;) {
		auto directed_edge = edge->source() == v ? edge : edge->twin();
		assert(directed_edge->source() == v);
		op(directed_edge);
		if (++edge == edges_end)
			break;
	}
}

enum MinMax {
	Min,
	Max,
};

/**
 * @brief Finds an edge coming out of a vertex, which is on the right/left relative to a given angle, and has a max/min
 * angle relative to the angle.
 *
 * @param v the source vertex
 * @param angle the relative angle
 * @param side right/left side to search for an edge
 * @param min_max min/max to find the most fit edge
 * @param res output result
 * @return true if found, else false
 */
static bool find_edge_relative_to_angle(const Vertex &v, const Direction &angle, enum HalfPlaneSide side,
										enum MinMax min_max, Halfedge &res) {
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

/* same as the generic function, but always relative to y-axis and the left side */
static bool find_vertex_left_edge_with(const Vertex &v, enum MinMax min_max, Halfedge &res) {
	return find_edge_relative_to_angle(v, Direction(0, 1), HalfPlaneSide::Left, min_max, res);
}

class DecompVertexData {
  public:
	bool is_edge_above;
	bool is_edge_below;
	Halfedge edge_above;
	Halfedge edge_below;
	DecompVertexData() { is_edge_above = is_edge_below = false; }
};

/* Perform a regular vertical decomposition, and fill the decomp data structure. In contract to the CGAL decomposition,
 * we are only interested in edges that are above and below each vertex. If a vertex v is exactly above (or below)
 * another vertex u, we say the edge going out of v to the x negative side is the object above u. If there is no such
 * edge, the vertex v is a reflex vertex, and we say the object above u is the object above v recursively. */
static void vertical_decomposition(const Arrangement &arr, std::vector<Vertex> &vertices,
								   std::map<Vertex, DecompVertexData> &decomp) {
	/* use CGAL vertical decomposition, which will result for each vertex the object above and below the vertex. The
	 * object might be an edge, a vertex, or an unbounded face. */
	std::vector<std::pair<Vertex, std::pair<CGAL::Object, CGAL::Object>>> vd_list;
	CGAL::decompose(arr, std::back_inserter(vd_list));

	/* convert the pairs list into map data structure for fast access and fill the vertices list */
	std::map<Vertex, CGAL::Object> above_orig;
	std::map<Vertex, CGAL::Object> below_orig;
	for (auto &decomp_entry : vd_list) {
		const auto &v = decomp_entry.first;
		vertices.push_back(v);
		above_orig[v] = decomp_entry.second.second;
		below_orig[v] = decomp_entry.second.first;
	}

	/* sort vertices firstly by x and than by y */
	sort(vertices.begin(), vertices.end(),
		 [](const Vertex &v1, const Vertex &v2) { return cmp(v1->point(), v2->point()) < 0; });

	/* for each vertex, findout the edge above it. If there is a vertex above it, take the edge that goes out of it
	 * towards the negative x direction or if there is no such edge, the edge above it recursively */
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

/* return true if two edges are equal, ignoring direction */
static bool undirected_eq(const Halfedge &e1, const Halfedge &e2) { return e1 == e2 || e1 == e2->twin(); }

#define INVALID_TRAPEZOID_ID -1

Trapezoider::VertexData::VertexData(Point &v, const Arrangement::Geometry_traits_2 *geom_traits) {
	top_left_trapezoid = top_right_trapezoid = INVALID_TRAPEZOID_ID;
	bottom_left_trapezoid = bottom_right_trapezoid = INVALID_TRAPEZOID_ID;
	ray_edges = std::set<Halfedge, Closer_edge>(Closer_edge(geom_traits, v));
}

/* "less" object used for maps of edges */
class Less_edge : public CGAL::cpp98::binary_function<Halfedge, Halfedge, bool> {
  private:
	const Arrangement::Geometry_traits_2 *geom_traits;

  public:
	Less_edge() {}
	Less_edge(const Arrangement::Geometry_traits_2 *traits) : geom_traits(traits) {}
	bool operator()(Halfedge e1, Halfedge e2) const { return !undirected_eq(e1, e2) && &(*e1) < &(*e2); }
};

/* Create an arrangement out of a list of points. The accepted arrangements are only the ones which represent a simple
 * polygon with no holes - each vertex has a degree of 2, there are only 2 faces (one bounded, one unbounded), each edge
 * has each of the two faces on each side. */
static void create_arrangement(Arrangement &arr, const std::vector<Point> &points) {
	if (points.size() < 3)
		throw std::invalid_argument("too few points for polygon");

	/* create segments from points */
	std::vector<Segment> segments;
	for (unsigned int i = 0; i < points.size(); i++) {
		unsigned int j = i != points.size() - 1 ? i + 1 : 0;
		segments.push_back({points[i], points[j]});
	}

	/* validate no intersection */
	for (unsigned int i = 0; i < segments.size(); i++) {
		auto &segment = segments[i];
		if (segment.start() == segment.end())
			throw std::invalid_argument("zero length segment");
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
	}

	insert(arr, segments.begin(), segments.end());

	/* validate all vertices have a degree of 2 */
	for (auto v = arr.vertices_begin(); v != arr.vertices_end(); ++v)
		if (v->degree() != 2)
			throw std::invalid_argument("Invalid vertex degree, expected 2.");

	/* validate only two faces exists, one bounded and another unbounded */
	std::set<Face> faces;
	for (auto v = arr.vertices_begin(); v != arr.vertices_end(); ++v)
		foreach_vertex_edge(v, [&faces](const auto &e) { faces.insert(e->face()); });
	if (faces.size() != 2)
		throw std::invalid_argument("Invalid faces number, expected 2.");

	/* validate no zero width edges exists */
	for (auto v = arr.vertices_begin(); v != arr.vertices_end(); ++v)
		foreach_vertex_edge(v, [](const auto &e) {
			if (is_free(e->face()) ^ is_free(e->face()))
				throw std::invalid_argument("zero width edges are not supported");
		});
}

/* return an edge or it's twin, the one with the free face, that is face representing the interior of the room */
static Halfedge direct_edge_free_face(const Halfedge &edge) { return is_free(edge->face()) ? edge : edge->twin(); }

/* Create a new Trapezoid and update the relevant data structures */
Trapezoid::ID Trapezoider::create_trapezoid(const Halfedge &top_edge, const Halfedge &bottom_edge,
											const Vertex &left_vertex, const Vertex &right_vertex) {
	/* create the Trapezoid */
	Trapezoid::ID t_id = ++trapezoids_id_counter;
	auto top_edge_d = direct_edge_free_face(top_edge), bottom_edge_d = direct_edge_free_face(bottom_edge);
	Trapezoid trapezoid(t_id, top_edge_d, bottom_edge, left_vertex, right_vertex);
	trapezoids[t_id] = trapezoid;

	auto &left_v_data = vertices_data[trapezoid.left_vertex];
	auto &right_v_data = vertices_data[trapezoid.right_vertex];

	/* update trapezoid left limiting vertex data */
	bool left_on_top = trapezoid.top_edge->curve().line().has_on(trapezoid.left_vertex->point());
	bool left_on_bottom = trapezoid.bottom_edge->curve().line().has_on(trapezoid.left_vertex->point());
	if (!left_on_top || left_on_bottom)
		left_v_data.top_right_trapezoid = t_id;
	if (!left_on_bottom || left_on_top)
		left_v_data.bottom_right_trapezoid = t_id;

	/* update trapezoid right limiting vertex data */
	bool right_on_top = trapezoid.top_edge->curve().line().has_on(trapezoid.right_vertex->point());
	bool right_on_bottom = trapezoid.bottom_edge->curve().line().has_on(trapezoid.right_vertex->point());
	if (!right_on_top || right_on_bottom)
		right_v_data.top_left_trapezoid = t_id;
	if (!right_on_bottom || right_on_top)
		right_v_data.bottom_left_trapezoid = t_id;

	return t_id;
}

/* finalize trapezoid, that it updating the relevant data structures */
void Trapezoider::finalize_trapezoid(const Trapezoid &trapezoid) {
	auto &left_v_data = vertices_data[trapezoid.left_vertex];
	auto &right_v_data = vertices_data[trapezoid.right_vertex];

	/* update trapezoid left limiting vertex data */
	bool left_on_top = trapezoid.top_edge->curve().line().has_on(trapezoid.left_vertex->point());
	bool left_on_bottom = trapezoid.bottom_edge->curve().line().has_on(trapezoid.left_vertex->point());
	if (!left_on_top || left_on_bottom)
		left_v_data.top_right_trapezoid = INVALID_TRAPEZOID_ID;
	if (!left_on_bottom || left_on_top)
		left_v_data.bottom_right_trapezoid = INVALID_TRAPEZOID_ID;

	/* update trapezoid right limiting vertex data */
	bool right_on_top = trapezoid.top_edge->curve().line().has_on(trapezoid.right_vertex->point());
	bool right_on_bottom = trapezoid.bottom_edge->curve().line().has_on(trapezoid.right_vertex->point());
	if (!right_on_top || right_on_bottom)
		right_v_data.top_left_trapezoid = INVALID_TRAPEZOID_ID;
	if (!right_on_bottom || right_on_top)
		right_v_data.bottom_left_trapezoid = INVALID_TRAPEZOID_ID;
}

/* perform a regular vertical decomposition and calculate all trapezoids that exists in that angle */
void Trapezoider::init_trapezoids_with_regular_vertical_decomposition() {
	infoln("[Trapezoider] Performing regular vertcal decomposition");

	std::vector<Vertex> vertices;
	std::map<Vertex, DecompVertexData> decomp;
	vertical_decomposition(arr, vertices, decomp);

	/* for each edge, stores the vertex that it's ray is hitting the edge */
	std::map<Halfedge, Vertex, Less_edge> most_right_vertex(Less_edge(arr.geometry_traits()));
	for (const auto &v : vertices) {
		const DecompVertexData &v_decomp_data = decomp[v];
		Halfedge top_edge, bottom_edge;

		if ((v_decomp_data.is_edge_above && is_free(v_decomp_data.edge_above->face())) &&
			(v_decomp_data.is_edge_below && is_free(v_decomp_data.edge_below->face())) &&
			!find_vertex_left_edge_with(v, MinMax::Min, top_edge)) {

			/* Reflex (more than 180 degrees) vertex */
			debugln("[Trapezoider] New trapezoid: reflex (" << v->point() << ')');
			auto left_v = most_right_vertex[v_decomp_data.edge_above];
			auto id = create_trapezoid(v_decomp_data.edge_above, v_decomp_data.edge_below, left_v, v);
			most_right_vertex[trapezoids.at(id).top_edge] = v;

		} else if ((!v_decomp_data.is_edge_above || !is_free(v_decomp_data.edge_above->face())) &&
				   (!v_decomp_data.is_edge_below || !is_free(v_decomp_data.edge_below->face())) &&
				   find_vertex_left_edge_with(v, MinMax::Min, top_edge) &&
				   find_vertex_left_edge_with(v, MinMax::Max, bottom_edge)) {

			/* v is a vertex of a triangle trapezoid */
			debugln("[Trapezoider] New trapezoid: triangle (" << v->point() << ')');
			auto left_v = most_right_vertex[top_edge];
			create_trapezoid(top_edge, bottom_edge, left_v, v);

		} else {
			if (v_decomp_data.is_edge_above && is_free(v_decomp_data.edge_above->face())) {

				// Edge above the vertex
				debugln("[Trapezoider] New trapezoid: up (" << v->point() << ')');
				if (!find_vertex_left_edge_with(v, MinMax::Min, bottom_edge))
					throw std::logic_error("failed to find bottom edge for up trapezoid");

				auto left_v = most_right_vertex[v_decomp_data.edge_above];
				auto id = create_trapezoid(v_decomp_data.edge_above, bottom_edge, left_v, v);
				most_right_vertex[trapezoids.at(id).top_edge] = v;
			}
			if (v_decomp_data.is_edge_below && is_free(v_decomp_data.edge_below->face())) {

				// Edge below the vertex
				debugln("[Trapezoider] New trapezoid: down (" << v->point() << ')');
				if (!find_vertex_left_edge_with(v, MinMax::Max, top_edge))
					throw std::logic_error("failed to find top edge for down trapezoid");

				auto left_v = most_right_vertex[top_edge];
				create_trapezoid(top_edge, v_decomp_data.edge_below, left_v, v);
			}
		}
		foreach_vertex_edge(v, [&v, &most_right_vertex](const auto &edge) { most_right_vertex[edge] = v; });
	}

	debugln("[Trapezoider] After regular vertical decomposition, trapezoids:");
	for (const auto &t : trapezoids)
		debugln("\t" << t.second);
}

static bool get_edge(const Vertex &source, const Vertex &target, Halfedge &res) {
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

static bool is_same_direction(Direction d1, Direction d2) {
	return calc_half_plane_side(d1, d2) == HalfPlaneSide::None && d1.vector() * d2.vector() > 0;
}

/* perfrom a parallel rotational sweep to calculate all trapezoids of all angles. Assume the data structres have been
 * filled with trapezoids that exists in the regular vertical decomposition direction */
void Trapezoider::calc_trapezoids_with_rotational_sweep() {
	infoln("[Trapezoider] Performing parallel rotational sweep (PRS)");
	/* Calculate all events */
	std::vector<Event> events;
	events.reserve(arr.number_of_vertices() * (arr.number_of_vertices() - 1));
	for (auto v1 = arr.vertices_begin(); v1 != arr.vertices_end(); ++v1)
		for (auto v2 = arr.vertices_begin(); v2 != arr.vertices_end(); ++v2)
			if (v1 != v2)
				events.push_back(Event(v1, v2));

	/* Sort events by their angle */
	sort(events.begin(), events.end(), [](const Event &e1, const Event &e2) {
		auto a1 = e1.get_ray(), a2 = e2.get_ray();
		if (a1 == a2)
			return false;

		if (is_same_direction(a1, a2))
			/* Both are exactly on the same angle, consider further vertices first */
			return a1.dx() * a1.dx() + a1.dy() * a1.dy() > a2.dx() * a2.dx() + a2.dy() * a2.dy();

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

	debugln("[Trapezoider] PRS events:");
	for (const auto &event : events)
		debugln("\t(" << event.v1->point() << ") (" << event.v2->point()
					  << ") angle= " << direction_to_angles(event.get_ray()));

	/* Perform rotational sweep by handling all events in the sorted order */
	for (Event &event : events) {
		assert(vertices_data.find(event.v1) != vertices_data.end());
		const auto ray = event.get_ray();
		auto &ray_edges = vertices_data.find(event.v1)->second.ray_edges;
		bool closest_edge_orig_valid;
		Halfedge closest_edge_orig;
		if (closest_edge_orig_valid = ray_edges.size() > 0)
			closest_edge_orig = *ray_edges.begin();

		debugln("[Trapezoider] PRS handle event (" << event.v1->point() << ") (" << event.v2->point() << ')');

		/* Maintaine ray_edges - the binary search tree used to determine the closest edge intersection the ray */
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

		/* Create and terminate trapezoids due to the event */
		auto current_angle = ray;
		VertexData &v1_data = vertices_data.find(event.v1)->second;
		VertexData &v2_data = vertices_data.find(event.v2)->second;
		Halfedge v1v2_edge;
		if (get_edge(event.v1, event.v2, v1v2_edge)) {

			/* Type 1 event - an edge between v1 and v2 exists. */
			bool left_is_free = is_free(v1v2_edge->face());
			debugln("[Trapezoider] PRS event type 1: v1v2_edge (" << v1v2_edge->curve() << ") left is "
																  << (left_is_free ? "free" : "not free"));
			if (left_is_free) {
				assert(v2_data.bottom_left_trapezoid != INVALID_TRAPEZOID_ID);
				assert(v2_data.bottom_right_trapezoid != INVALID_TRAPEZOID_ID);
				Trapezoid &left = trapezoids.at(v2_data.bottom_left_trapezoid);
				Trapezoid &mid = trapezoids.at(v2_data.bottom_right_trapezoid);

				assert(left.right_vertex == event.v2);
				assert(mid.left_vertex == event.v2);
				assert(mid.right_vertex == event.v1);

				/* finalize the triangle trapezoid and its neighbor */
				left.angle_end = current_angle;
				finalize_trapezoid(left);
				mid.angle_end = current_angle;
				finalize_trapezoid(mid);

				/* replace neighbor trapezoid with one with updated limiting vertices */
				debugln("\told left: " << left);
				auto left_new = create_trapezoid(left.top_edge, left.bottom_edge, left.left_vertex, event.v1);
				trapezoids.at(left_new).angle_begin = current_angle;
				debugln("\tnew left: " << trapezoids.at(left_new));

				/* Crate new triangle trapezoid */
				debugln("\told mid: " << mid);
				auto mid_new = create_trapezoid(left.top_edge, v1v2_edge, event.v1, event.v2);
				trapezoids.at(mid_new).angle_begin = current_angle;
				debugln("\tnew mid: " << trapezoids.at(mid_new));

			} else {
				assert(v1_data.top_left_trapezoid != INVALID_TRAPEZOID_ID);
				assert(v1_data.top_right_trapezoid != INVALID_TRAPEZOID_ID);
				Trapezoid &mid = trapezoids.at(v1_data.top_left_trapezoid);
				Trapezoid &right = trapezoids.at(v1_data.top_right_trapezoid);

				assert(mid.left_vertex == event.v2);
				assert(mid.right_vertex == event.v1);
				assert(right.left_vertex == event.v1);

				/* finalize the triangle trapezoid and its neighbor */
				mid.angle_end = current_angle;
				finalize_trapezoid(mid);
				right.angle_end = current_angle;
				finalize_trapezoid(right);

				/* Crate new triangle trapezoid */
				debugln("\told mid: " << mid);
				auto mid_new = create_trapezoid(v1v2_edge, right.bottom_edge, event.v1, event.v2);
				trapezoids.at(mid_new).angle_begin = current_angle;
				debugln("\tnew mid: " << trapezoids.at(mid_new));

				/* replace neighbor trapezoid with one with updated limiting vertices */
				debugln("\told right: " << right);
				auto right_new = create_trapezoid(right.top_edge, right.bottom_edge, event.v2, right.right_vertex);
				trapezoids.at(right_new).angle_begin = current_angle;
				debugln("\tnew right: " << trapezoids.at(right_new));
			}
		} else {
			if (ray_edges.size() == 0)
				continue; /* the ray intersect no edge */
			auto closest_edge = *ray_edges.begin();
			if (closest_edge_orig_valid && closest_edge_orig == closest_edge)
				continue; /* Closest edge didn't changed */
			if (!is_free(closest_edge->face()))
				continue; // The ray is in non free area of the room

			/* Type 2 event */
			debugln("[Trapezoider] PRS event type 2: closest edge (" << closest_edge->curve() << ')');
			assert(v2_data.bottom_left_trapezoid != INVALID_TRAPEZOID_ID);
			assert(v1_data.top_left_trapezoid != INVALID_TRAPEZOID_ID);
			assert(v1_data.top_left_trapezoid == v2_data.bottom_right_trapezoid);
			assert(v1_data.top_right_trapezoid != INVALID_TRAPEZOID_ID);
			Trapezoid &left = trapezoids.at(v2_data.bottom_left_trapezoid);
			Trapezoid &mid = trapezoids.at(v1_data.top_left_trapezoid);
			Trapezoid &right = trapezoids.at(v1_data.top_right_trapezoid);

			assert(left.right_vertex == event.v2);
			assert(mid.left_vertex == event.v2);
			assert(mid.right_vertex == event.v1);
			assert(right.left_vertex == event.v1);

			/* finalize all the trapezoids around the ray */
			left.angle_end = current_angle;
			finalize_trapezoid(left);
			mid.angle_end = current_angle;
			finalize_trapezoid(mid);
			right.angle_end = current_angle;
			finalize_trapezoid(right);

			/* create new left trapezoid */
			debugln("\told left: " << left);
			auto left_new = create_trapezoid(left.top_edge, left.bottom_edge, left.left_vertex, event.v1);
			trapezoids.at(left_new).angle_begin = current_angle;
			debugln("\tnew left: " << trapezoids.at(left_new));

			/* create new mid trapezoid */
			debugln("\told mid: " << mid);
			Halfedge bottom_edge;
			if (!find_edge_relative_to_angle(event.v1, current_angle, HalfPlaneSide::Right, MinMax::Max, bottom_edge))
				bottom_edge = right.bottom_edge;
			auto mid_new = create_trapezoid(left.top_edge, bottom_edge, event.v1, event.v2);
			trapezoids.at(mid_new).angle_begin = current_angle;
			debugln("\tnew mid: " << trapezoids.at(mid_new));

			/* create new right trapezoid */
			debugln("\told right: " << right);
			auto right_new = create_trapezoid(right.top_edge, right.bottom_edge, event.v2, right.right_vertex);
			trapezoids.at(right_new).angle_begin = current_angle;
			debugln("\tnew right: " << trapezoids.at(right_new));
		}
	}

	/* We started with regular vertical decomposition, and the trapezoids calculated from the beginning lack the start
	 * angle. In addition, near the end of the rotational sweep, we created some trapezoids we considered new, but they
	 * are actually a duplication of the original starting trapezoids. We union them and remove the later ones. */
	debugln("[Trapezoider] PRS merge unfinished trapezoids:");
	std::map<std::pair<Vertex, Vertex>, Trapezoid::ID> no_begin_ts;
	std::map<std::pair<Vertex, Vertex>, Trapezoid::ID> no_end_ts;
	for (auto &p : trapezoids) {
		Trapezoid &trapezoid = p.second;
		assert(!(trapezoid.angle_begin == Trapezoid::ANGLE_NONE && trapezoid.angle_end == Trapezoid::ANGLE_NONE));
		if (trapezoid.angle_begin == Trapezoid::ANGLE_NONE) {
			assert(no_begin_ts.find({trapezoid.left_vertex, trapezoid.right_vertex}) == no_begin_ts.end());
			no_begin_ts[{trapezoid.left_vertex, trapezoid.right_vertex}] = trapezoid.get_id();
		} else if (trapezoid.angle_end == Trapezoid::ANGLE_NONE) {
			assert(no_end_ts.find({trapezoid.left_vertex, trapezoid.right_vertex}) == no_end_ts.end());
			no_end_ts[{trapezoid.left_vertex, trapezoid.right_vertex}] = trapezoid.get_id();
		}
	}
	assert(no_begin_ts.size() == no_end_ts.size());
	for (auto &p : no_begin_ts) {
		const std::pair<Vertex, Vertex> &v = p.first;
		Trapezoid &trapezoid = trapezoids.at(p.second);
		assert(no_end_ts.find(v) != no_end_ts.end());
		Trapezoid &other = trapezoids.at(no_end_ts[v]);
		debugln("\tT" << trapezoid.get_id() << " with T" << other.get_id() << ": " << trapezoid);
		assert(undirected_eq(trapezoid.top_edge, other.top_edge));
		assert(undirected_eq(trapezoid.bottom_edge, other.bottom_edge));
		assert(trapezoid.left_vertex == other.left_vertex);
		assert(trapezoid.right_vertex == other.right_vertex);
		trapezoid.angle_begin = other.angle_begin;
		trapezoids.erase(other.get_id());
	}

	/* Remove degenerated trapezoids, these are trapezoids that start and finish at the same angle */
	std::vector<Trapezoid::ID> empty_trapezoids;
	for (const auto &p : trapezoids) {
		const auto &trapezoid = p.second;
		if (is_same_direction(trapezoid.angle_begin, trapezoid.angle_end))
			empty_trapezoids.push_back(p.first);
	}
	for (Trapezoid::ID empty_trapezoid : empty_trapezoids)
		trapezoids.erase(empty_trapezoid);
}

void Trapezoider::calc_trapezoids(const std::vector<Point> &points, std::vector<Trapezoid> &res) {
	infoln("[Trapezoider] Calculating trapezoids...");
	trapezoids.clear();
	vertices_data.clear();
	trapezoids_id_counter = 0;

	/* arr.clear() has a bug. This function should only be used once for a Trapezoider object */
	arr.clear();
	create_arrangement(arr, points);

	/* init vertices associated data structure */
	for (auto v = arr.vertices_begin(); v != arr.vertices_end(); ++v)
		vertices_data[v] = VertexData(v->point(), arr.geometry_traits());

	/* perform all trapezoids by useing regular vertical decomposition followed by a parallel rotational sweep */
	init_trapezoids_with_regular_vertical_decomposition();
	calc_trapezoids_with_rotational_sweep();

	debugln("[Trapezoider] After rotational sweep, trapezoids:");
	for (const auto &p : trapezoids)
		debugln("\t" << p.second);
	infoln("[Trapezoider] " << trapezoids.size() << " trapezoid found successfully");

	for (const auto &p : trapezoids)
		res.push_back(p.second);
}
