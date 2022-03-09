#include "trapezoider.h"
#include "utils.hpp"
#include <CGAL/Arr_vertical_decomposition_2.h>

namespace FDML {

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
		return Direction(p2.x() - p1.x(), p2.y() - p1.y());
	}
};

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
static bool find_edge_relative_to_angle(const Vertex &v, const Direction &angle, CGAL::Oriented_side side,
										enum MinMax min_max, Halfedge &res) {
	bool found = false;
	Halfedge best;
	foreach_vertex_edge(v, [&v, &angle, side, min_max, &found, &best](const Halfedge &edge) {
		auto calc_edge_angle = [&v](const auto &e) {
			Point target = (e->source() == v ? e->target() : e->source())->point();
			Point vp = v->point();
			return Point(target.x() - vp.x(), target.y() - vp.y());
		};
		auto e_angle = calc_edge_angle(edge);

		if (side == Line({0, 0}, angle).oriented_side(e_angle)) {
			auto min_max_side = min_max == MinMax::Max ? CGAL::ON_POSITIVE_SIDE : CGAL::ON_NEGATIVE_SIDE;
			if (!found || Line({0, 0}, calc_edge_angle(best)).oriented_side(e_angle) == min_max_side) {
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
static bool find_edge_left_from_vertex(const Vertex &v, enum MinMax min_max, Halfedge &res) {
	return find_edge_relative_to_angle(v, Direction(0, 1), CGAL::ON_POSITIVE_SIDE, min_max, res);
}

static bool find_edge_vertical(const Vertex &v, enum CGAL::Sign dir, Halfedge &res) {
	bool found = false;
	foreach_vertex_edge(v, [&v, dir, &found, &res](const Halfedge &edge) {
		assert(edge->source() == v);
		if (v->point().x() != edge->target()->point().x())
			return;
		if ((dir == CGAL::NEGATIVE && edge->target()->point().y() < v->point().y()) ||
			(dir == CGAL::POSITIVE && edge->target()->point().y() > v->point().y())) {
			res = edge;
			found = true;
		}
	});
	return found;
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
		 [](const Vertex &v1, const Vertex &v2) { return v1->point() < v2->point(); });

	/* This assume the DCEL implementation stores the LEFT face of an edge as edge->face() */
	auto direct_above_edge = [](const Halfedge &edge) {
		return edge->target()->point().x() <= edge->source()->point().x() ? edge : edge->twin();
	};
	auto direct_below_edge = [](const Halfedge &edge) {
		return edge->target()->point().x() >= edge->source()->point().x() ? edge : edge->twin();
	};

	/* CGAL decomposition doesn't seems to compute the above and below vertices correctly, we do it manually */
	std::map<Vertex, std::pair<bool, Vertex>> vertex_above;
	std::map<Vertex, std::pair<bool, Vertex>> vertex_below;
	for (unsigned int i = 0; i < vertices.size(); i++) {
		const auto &v = vertices[i];
		bool has_above = i < (vertices.size() - 1) && v->point().x() == vertices[i + 1]->point().x() &&
						 v->point().y() < vertices[i + 1]->point().y();
		bool has_below =
			i > 0 && v->point().x() == vertices[i - 1]->point().x() && v->point().y() > vertices[i - 1]->point().y();
		vertex_above[v] = std::make_pair(has_above, has_above ? vertices[i + 1] : /* dummy */ v);
		vertex_below[v] = std::make_pair(has_below, has_below ? vertices[i - 1] : /* dummy */ v);
	}
	auto get_above_vertex = [&vertex_above](const Vertex &v, Vertex &above) {
		const auto &p = vertex_above[v];
		if (!p.first)
			return false;
		above = p.second;
		return true;
	};
	auto get_below_vertex = [&vertex_below](const Vertex &v, Vertex &below) {
		const auto &p = vertex_below[v];
		if (!p.first)
			return false;
		below = p.second;
		return true;
	};

	/* for each vertex, findout the edge above it. If there is a vertex above it, take the edge that goes out of it
	 * towards the negative x direction or if there is no such edge, the edge above it recursively */
	for (auto &v : vertices) {
		DecompVertexData v_data;
		Halfedge edge;
		for (Vertex p = v, up_vertex;; p = up_vertex) {
			auto &above_obj = above_orig[p];
			/* if the above object is an edge, we are done */
			if (CGAL::assign(edge, above_obj)) {
				v_data.edge_above = direct_above_edge(edge);
				v_data.is_edge_above = true;
				break;
			}
			/* if there is no vertex above, we reached the end of the room, done */
			if (!CGAL::assign(up_vertex, above_obj) && !get_above_vertex(p, up_vertex))
				break;
			/* there is a vertex above, search for an edge from it to the negative x direction */
			if (find_edge_relative_to_angle(up_vertex, Direction(0, 1), CGAL::ON_NEGATIVE_SIDE, MinMax::Min, edge)) {
				v_data.edge_above = direct_above_edge(edge);
				v_data.is_edge_above = true;
				break;
			}
			/* continue searching up */
		}
		for (Vertex p = v, below_vertex;; p = below_vertex) {
			auto &below_obj = below_orig[p];
			/* if the below object is an edge, we are done */
			if (CGAL::assign(edge, below_obj)) {
				v_data.edge_below = direct_below_edge(edge);
				v_data.is_edge_below = true;
				break;
			}
			/* if there is no vertex below, we reached the end of the room, done */
			if (!CGAL::assign(below_vertex, below_obj) && !get_below_vertex(p, below_vertex))
				break;
			/* there is a vertex below, search for an edge from it to the negative x direction */
			if (find_edge_left_from_vertex(below_vertex, MinMax::Min, edge)) {
				v_data.edge_below = direct_below_edge(edge);
				v_data.is_edge_below = true;
				break;
			}
			/* continue searching down */
		}
		decomp[v] = v_data;
	}
}

/* return true if two edges are equal, ignoring direction */
static bool undirected_eq(const Halfedge &e1, const Halfedge &e2) { return e1 == e2 || e1 == e2->twin(); }

static const unsigned int INVALID_TRAPEZOID_ID = 0xffffffff;

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
	bool operator()(const Halfedge &e1, const Halfedge &e2) const {
		const auto e1_ = e1->source()->point() <= e1->target()->point() ? e1 : e1->twin();
		const auto e2_ = e2->source()->point() <= e2->target()->point() ? e2 : e2->twin();
		CGAL::Comparison_result c;
		if ((c = CGAL::compare_xy(e1_->source()->point(), e2_->source()->point())) != CGAL::EQUAL)
			return c == CGAL::SMALLER;
		if ((c = CGAL::compare_xy(e1_->target()->point(), e2_->target()->point())) != CGAL::EQUAL)
			return c == CGAL::SMALLER;
		return false;
	}
};

/* Create an arrangement out of a list of points. The accepted arrangements are only the ones which represent a
 * simple polygon with no holes - each vertex has a degree of 2, there are only 2 faces (one bounded, one
 * unbounded), each edge has each of the two faces on each side. */
static void create_arrangement(Arrangement &arr, const Polygon &scene) {
	if (scene.size() < 3 || !scene.is_simple())
		throw std::invalid_argument("input scene is not a simple polygon");

	insert(arr, scene.edges_begin(), scene.edges_end());

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
	Trapezoid::ID t_id = trapezoids.size();
	auto top_edge_d = direct_edge_free_face(top_edge), bottom_edge_d = direct_edge_free_face(bottom_edge);
	Trapezoid trapezoid(t_id, top_edge_d, bottom_edge_d, left_vertex, right_vertex);

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

	trapezoids.push_back(std::move(trapezoid));

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
	fdml_infoln("[Trapezoider] Performing regular vertcal decomposition");

	std::vector<Vertex> vertices;
	std::map<Vertex, DecompVertexData> decomp;
	vertical_decomposition(arr, vertices, decomp);

	/* sort vertices. unusual sort, prefer smaller x bigger y */
	sort(vertices.begin(), vertices.end(), [](const Vertex &v1, const Vertex &v2) {
		if (v1->point().x() != v2->point().x())
			return v1->point().x() < v2->point().x();
		return v1->point().y() > v2->point().y();
	});

	/* for each edge, stores the vertex that it's ray is hitting the edge */
	std::map<Halfedge, Vertex, Less_edge> most_right_vertex(Less_edge(arr.geometry_traits()));
	for (const auto &v : vertices) {
		const DecompVertexData &v_decomp_data = decomp[v];
		Halfedge top_edge, bottom_edge;

		if ((v_decomp_data.is_edge_above && is_free(v_decomp_data.edge_above->face())) &&
			(v_decomp_data.is_edge_below && is_free(v_decomp_data.edge_below->face())) &&
			!find_edge_left_from_vertex(v, MinMax::Min, top_edge)) {

			/* Reflex (more than 180 degrees) vertex */
			fdml_debugln("[Trapezoider] New trapezoid: reflex (" << v->point() << ')');
			auto left_v = most_right_vertex.at(v_decomp_data.edge_above);
			auto id = create_trapezoid(v_decomp_data.edge_above, v_decomp_data.edge_below, left_v, v);
			most_right_vertex[trapezoids.at(id).top_edge] = v;

		} else if ((!v_decomp_data.is_edge_above || !is_free(v_decomp_data.edge_above->face())) &&
				   (!v_decomp_data.is_edge_below || !is_free(v_decomp_data.edge_below->face())) &&
				   (find_edge_vertical(v, CGAL::POSITIVE, top_edge) ||
					find_edge_left_from_vertex(v, MinMax::Min, top_edge)) &&
				   find_edge_left_from_vertex(v, MinMax::Max, bottom_edge)) {

			/* v is a vertex of a triangle trapezoid */
			fdml_debugln("[Trapezoider] New trapezoid: triangle (" << v->point() << ')');
			auto left_v = most_right_vertex.at(top_edge);
			create_trapezoid(top_edge, bottom_edge, left_v, v);

		} else {
			if (v_decomp_data.is_edge_above && is_free(v_decomp_data.edge_above->face())) {

				// Edge above the vertex
				fdml_debugln("[Trapezoider] New trapezoid: up (" << v->point() << ')');
				if (!find_edge_vertical(v, CGAL::POSITIVE, bottom_edge) &&
					!find_edge_left_from_vertex(v, MinMax::Min, bottom_edge))
					throw std::logic_error("failed to find bottom edge for up trapezoid");

				auto left_v = most_right_vertex.at(v_decomp_data.edge_above);
				auto id = create_trapezoid(v_decomp_data.edge_above, bottom_edge, left_v, v);
				most_right_vertex[trapezoids.at(id).top_edge] = v;
			}
			if (v_decomp_data.is_edge_below && is_free(v_decomp_data.edge_below->face())) {

				// Edge below the vertex
				fdml_debugln("[Trapezoider] New trapezoid: down (" << v->point() << ')');
				if (!find_edge_left_from_vertex(v, MinMax::Max, top_edge))
					throw std::logic_error("failed to find top edge for down trapezoid");

				auto left_v = most_right_vertex.at(top_edge);
				create_trapezoid(top_edge, v_decomp_data.edge_below, left_v, v);
			}
		}
		foreach_vertex_edge(v, [&v, &most_right_vertex](const auto &edge) { most_right_vertex[edge] = v; });
	}

	fdml_debugln("[Trapezoider] After regular vertical decomposition, trapezoids:");
	for (const auto &trapezoid : trapezoids)
		fdml_debugln("\t" << trapezoid);
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
	return Line({0, 0}, d1).oriented_side({d2.dx(), d2.dy()}) == CGAL::ON_ORIENTED_BOUNDARY &&
		   d1.vector() * d2.vector() > 0;
}

/* Calculate which of an edge endpoint is "left" and "right" relative to some direction */
static void calc_edge_left_right_vertices(const Halfedge &edge, const Direction &dir, Point &left, Point &right) {
	Point p1 = edge->source()->point(), p2 = edge->target()->point();
	if (Line({0, 0}, dir).oriented_side({(p1.x() - p2.x()) / 2, (p1.y() - p2.y()) / 2}) == CGAL::ON_POSITIVE_SIDE) {
		left = p1;
		right = p2;
	} else {
		left = p2;
		right = p1;
	}
}

/* perfrom a parallel rotational sweep to calculate all trapezoids of all angles. Assume the data structres have
 * been filled with trapezoids that exists in the regular vertical decomposition direction */
void Trapezoider::calc_trapezoids_with_rotational_sweep() {
	fdml_infoln("[Trapezoider] Performing parallel rotational sweep (PRS)");
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

		Line y_axis({0, 0}, Point(0, 1));
		CGAL::Oriented_side a1_side = y_axis.oriented_side({a1.dx(), a1.dy()});
		CGAL::Oriented_side a2_side = y_axis.oriented_side({a2.dx(), a2.dy()});

		if (a1_side == CGAL::ON_ORIENTED_BOUNDARY)
			return a2_side == CGAL::ON_NEGATIVE_SIDE ||
				   (a1.dy() >= 0 && (a2_side == CGAL::ON_POSITIVE_SIDE || a2.dy() < 0));
		if (a2_side == CGAL::ON_ORIENTED_BOUNDARY)
			return a1_side == CGAL::ON_POSITIVE_SIDE && a2.dy() < 0;
		if (a1_side == CGAL::ON_POSITIVE_SIDE)
			return a2_side == CGAL::ON_NEGATIVE_SIDE ||
				   Line({0, 0}, a2).oriented_side({a1.dx(), a1.dy()}) == CGAL::ON_NEGATIVE_SIDE;
		// a1_side == CGAL::ON_NEGATIVE_SIDE
		return a2_side == CGAL::ON_NEGATIVE_SIDE &&
			   Line({0, 0}, a2).oriented_side({a1.dx(), a1.dy()}) == CGAL::ON_NEGATIVE_SIDE;
	});

	fdml_debugln("[Trapezoider] PRS events:");
	for (const auto &event : events)
		fdml_debugln("\t(" << event.v1->point() << ") (" << event.v2->point()
						   << ") angle= " << direction_to_angles(event.get_ray()));

	/* init rays */
	Direction init_ray_direction(0, 1);
	for (auto &p : vertices_data) {
		VertexData &v_data = p.second;
		const auto vp = p.first->point();
		const auto init_ray = Kernel::Ray_2(vp, init_ray_direction);
		for (auto uit = arr.vertices_begin(); uit != arr.vertices_end(); ++uit) {
			foreach_vertex_edge(uit, [&vp, &init_ray, &v_data](const auto &edge) {
				if (edge->source()->point() < edge->target()->point())
					return; /* consider only one of the edge and its twin */
				if (edge->source()->point().x() == vp.x() || edge->target()->point().x() == vp.x())
					return; /* avoid edges the ray itersect at an endpoint */
				if (do_intersect(init_ray, Segment(edge->source()->point(), edge->target()->point())))
					v_data.ray_edges.insert(edge);
			});
		}
	}

	/* Perform rotational sweep by handling all events in the sorted order */
	for (Event &event : events) {
		VertexData &v1_data = vertices_data.at(event.v1);
		VertexData &v2_data = vertices_data.at(event.v2);
		const auto ray = event.get_ray();
		auto &ray_edges = v1_data.ray_edges;
		bool closest_edge_orig_valid = ray_edges.size() > 0;
		Halfedge closest_edge_orig;
		if (closest_edge_orig_valid)
			closest_edge_orig = *ray_edges.begin();

		fdml_debugln("[Trapezoider] PRS handle event (" << event.v1->point() << ") (" << event.v2->point() << ')');

		/* Maintaine ray_edges - the binary search tree used to determine the closest edge intersection the ray */
		std::vector<Halfedge> edges_to_insert;
		std::vector<Halfedge> edges_to_remove;
		foreach_vertex_edge(event.v2, [ray, &edges_to_insert, &edges_to_remove](const auto &edge) {
			auto s = edge->source()->point(), t = edge->target()->point();
			Point edge_direction(t.x() - s.x(), t.y() - s.y());

			if (Line({0, 0}, ray).oriented_side(edge_direction) == CGAL::ON_POSITIVE_SIDE)
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
		Halfedge v1v2_edge;
		if (get_edge(event.v1, event.v2, v1v2_edge)) {

			/* Type 1 event - an edge between v1 and v2 exists. */
			bool left_is_free = is_free(v1v2_edge->face());
			fdml_debugln("[Trapezoider] PRS event type 1: v1v2_edge (" << v1v2_edge->curve() << ") left is "
																	   << (left_is_free ? "free" : "not free"));
			if (left_is_free) {
				assert(v2_data.bottom_left_trapezoid != INVALID_TRAPEZOID_ID);
				assert(v2_data.bottom_right_trapezoid != INVALID_TRAPEZOID_ID);
				Trapezoid &left = trapezoids.at(v2_data.bottom_left_trapezoid);
				Trapezoid &mid = trapezoids.at(v2_data.bottom_right_trapezoid);
				/* CAREFUL - don't use these references after create_trapezoid is called, container may change */

				assert(left.right_vertex == event.v2);
				assert(mid.left_vertex == event.v2);
				assert(mid.right_vertex == event.v1);

				/* finalize the triangle trapezoid and its neighbor */
				left.angle_end = current_angle;
				finalize_trapezoid(left);
				mid.angle_end = current_angle;
				finalize_trapezoid(mid);
				fdml_debugln("\told mid: " << mid);
				fdml_debugln("\told left: " << left);

				auto left_top = left.top_edge, left_bottom = left.bottom_edge;
				auto left_lvertex = left.left_vertex;

				/* replace neighbor trapezoid with one with updated limiting vertices */
				auto left_new = create_trapezoid(left_top, left_bottom, left_lvertex, event.v1);
				trapezoids.at(left_new).angle_begin = current_angle;

				/* Crate new triangle trapezoid */
				auto mid_new = create_trapezoid(left_top, v1v2_edge, event.v1, event.v2);
				trapezoids.at(mid_new).angle_begin = current_angle;

				fdml_debugln("\tnew mid: " << trapezoids.at(mid_new));
				fdml_debugln("\tnew left: " << trapezoids.at(left_new));

			} else {
				assert(v1_data.top_left_trapezoid != INVALID_TRAPEZOID_ID);
				assert(v1_data.top_right_trapezoid != INVALID_TRAPEZOID_ID);
				Trapezoid &mid = trapezoids.at(v1_data.top_left_trapezoid);
				Trapezoid &right = trapezoids.at(v1_data.top_right_trapezoid);
				/* CAREFUL - don't use these references after create_trapezoid is called, container may change */

				assert(mid.left_vertex == event.v2);
				assert(mid.right_vertex == event.v1);
				assert(right.left_vertex == event.v1);

				/* finalize the triangle trapezoid and its neighbor */
				mid.angle_end = current_angle;
				finalize_trapezoid(mid);
				right.angle_end = current_angle;
				finalize_trapezoid(right);
				fdml_debugln("\told right: " << right);
				fdml_debugln("\told mid: " << mid);

				auto right_top = right.top_edge, right_bottom = right.bottom_edge;
				auto right_rvertex = right.right_vertex;

				/* Crate new triangle trapezoid */
				auto mid_new = create_trapezoid(v1v2_edge, right_bottom, event.v1, event.v2);
				trapezoids.at(mid_new).angle_begin = current_angle;

				/* replace neighbor trapezoid with one with updated limiting vertices */
				auto right_new = create_trapezoid(right_top, right_bottom, event.v2, right_rvertex);
				trapezoids.at(right_new).angle_begin = current_angle;

				fdml_debugln("\tnew right: " << trapezoids.at(right_new));
				fdml_debugln("\tnew mid: " << trapezoids.at(mid_new));
			}
		} else {
			if (ray_edges.size() == 0)
				continue; /* the ray intersect no edge */
			auto closest_edge = *ray_edges.begin();
			if (closest_edge_orig_valid && closest_edge_orig == closest_edge)
				continue; /* Closest edge didn't changed */

			Point closest_left, closest_right;
			calc_edge_left_right_vertices(closest_edge, ray, closest_left, closest_right);
			if (closest_edge->source()->point() != closest_right)
				closest_edge = closest_edge->twin();
			if (!is_free(closest_edge->face()))
				continue; // The ray is in non free area of the room

			/* Type 2 event */
			fdml_debugln("[Trapezoider] PRS event type 2: closest edge (" << closest_edge->curve() << ')');
			assert(v2_data.bottom_left_trapezoid != INVALID_TRAPEZOID_ID);
			assert(v1_data.top_left_trapezoid != INVALID_TRAPEZOID_ID);
			assert(v1_data.top_left_trapezoid == v2_data.bottom_right_trapezoid);
			assert(v1_data.top_right_trapezoid != INVALID_TRAPEZOID_ID);
			Trapezoid &left = trapezoids.at(v2_data.bottom_left_trapezoid);
			Trapezoid &mid = trapezoids.at(v1_data.top_left_trapezoid);
			Trapezoid &right = trapezoids.at(v1_data.top_right_trapezoid);
			/* CAREFUL - don't use these references after create_trapezoid is called, container may change */

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

			fdml_debugln("\told right: " << right);
			fdml_debugln("\told mid: " << mid);
			fdml_debugln("\told left: " << left);

			auto right_top = right.top_edge, right_bottom = right.bottom_edge;
			auto right_rvertex = right.right_vertex;
			auto left_top = left.top_edge, left_bottom = left.bottom_edge;
			auto left_lvertex = left.left_vertex;

			/* create new right trapezoid */
			auto right_new = create_trapezoid(right_top, right_bottom, event.v2, right_rvertex);
			trapezoids.at(right_new).angle_begin = current_angle;

			/* create new mid trapezoid */
			Halfedge bottom_edge;
			if (!find_edge_relative_to_angle(event.v1, current_angle, CGAL::ON_NEGATIVE_SIDE, MinMax::Max, bottom_edge))
				bottom_edge = right_bottom;
			auto mid_new = create_trapezoid(left_top, bottom_edge, event.v1, event.v2);
			trapezoids.at(mid_new).angle_begin = current_angle;

			/* create new left trapezoid */
			auto left_new = create_trapezoid(left_top, left_bottom, left_lvertex, event.v1);
			trapezoids.at(left_new).angle_begin = current_angle;

			fdml_debugln("\tnew right: " << trapezoids.at(right_new));
			fdml_debugln("\tnew mid: " << trapezoids.at(mid_new));
			fdml_debugln("\tnew left: " << trapezoids.at(left_new));
		}
	}

	/* We started with regular vertical decomposition, and the trapezoids calculated from the beginning lack the
	 * start angle. In addition, near the end of the rotational sweep, we created some trapezoids we considered new,
	 * but they are actually a duplication of the original starting trapezoids. We union them and remove the later
	 * ones. */
	fdml_debugln("[Trapezoider] PRS merge unfinished trapezoids:");
	std::map<std::pair<Vertex, Vertex>, Trapezoid::ID> no_begin_ts;
	std::map<std::pair<Vertex, Vertex>, Trapezoid::ID> no_end_ts;
	for (const auto &trapezoid : trapezoids) {
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
	std::set<Trapezoid::ID> to_remove;
	for (auto &p : no_begin_ts) {
		const std::pair<Vertex, Vertex> &v = p.first;
		Trapezoid &trapezoid = trapezoids.at(p.second);
		assert(no_end_ts.find(v) != no_end_ts.end());
		Trapezoid &other = trapezoids.at(no_end_ts[v]);
		fdml_debugln("\tT" << trapezoid.get_id() << " with T" << other.get_id() << ": " << trapezoid);
		assert(undirected_eq(trapezoid.top_edge, other.top_edge));
		assert(undirected_eq(trapezoid.bottom_edge, other.bottom_edge));
		assert(trapezoid.left_vertex == other.left_vertex);
		assert(trapezoid.right_vertex == other.right_vertex);
		trapezoid.angle_begin = other.angle_begin;
		to_remove.insert(other.get_id());
	}

	trapezoids.erase(std::remove_if(trapezoids.begin(), trapezoids.end(),
									[&to_remove](const Trapezoid &trapezoid) {
										return to_remove.find(trapezoid.get_id()) != to_remove.end();
									}),
					 trapezoids.end());

	/* Remove degenerated trapezoids, these are trapezoids that start and finish at the same angle */
	trapezoids.erase(std::remove_if(trapezoids.begin(), trapezoids.end(),
									[&to_remove](const Trapezoid &trapezoid) {
										return is_same_direction(trapezoid.angle_begin, trapezoid.angle_end);
									}),
					 trapezoids.end());

	/* reassign IDs after erasing and reordering */
	for (unsigned int i = 0; i < trapezoids.size(); i++)
		trapezoids[i].id = i;
}

void Trapezoider::calc_trapezoids(const Polygon &scene) {
	fdml_infoln("[Trapezoider] Calculating trapezoids...");
	trapezoids.clear();
	vertices_data.clear();

	/* arr.clear() has a bug. This function should only be used once for a Trapezoider object */
	arr.clear();
	create_arrangement(arr, scene);

	/* init vertices associated data structure */
	for (auto v = arr.vertices_begin(); v != arr.vertices_end(); ++v)
		vertices_data[v] = VertexData(v->point(), arr.geometry_traits());

	/* perform all trapezoids by useing regular vertical decomposition followed by a parallel rotational sweep */
	init_trapezoids_with_regular_vertical_decomposition();
	calc_trapezoids_with_rotational_sweep();

	/* Fix exact numbers and avoid lazy evaluation */
	for (auto &trapezoid : trapezoids) {
		trapezoid.angle_begin = Direction(trapezoid.angle_begin.dx().exact(), trapezoid.angle_begin.dy().exact());
		trapezoid.angle_end = Direction(trapezoid.angle_end.dx().exact(), trapezoid.angle_end.dy().exact());
	}

	fdml_debugln("[Trapezoider] After rotational sweep, trapezoids:");
	for (const auto &trapezoid : trapezoids)
		fdml_debugln("\t" << trapezoid);
	fdml_infoln("[Trapezoider] " << trapezoids.size() << " trapezoids found successfully");
}

Trapezoider::TrapezoidIterator Trapezoider::trapezoids_begin() const { return trapezoids.begin(); }

Trapezoider::TrapezoidIterator Trapezoider::trapezoids_end() const { return trapezoids.end(); }

size_t Trapezoider::number_of_trapezoids() const { return trapezoids.size(); }

Trapezoider::TrapezoidIterator Trapezoider::get_trapezoid(Trapezoid::ID id) const { return trapezoids.begin() + id; }

} // namespace FDML
