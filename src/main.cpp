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
    return edge->curve().target().hx() >= edge->curve().source().hx()
               ? edge
               : edge->twin();
  return edge->curve().target().hy() >= edge->curve().source().hy()
             ? edge
             : edge->twin();
}

class DecompVertexData {
public:
  bool is_edge_above;
  bool is_edge_below;
  Halfedge_const_handle edge_above;
  Halfedge_const_handle edge_below;
  DecompVertexData() { is_edge_above = is_edge_below = false; }
};

static void vertical_decomposition(
    const Arrangement &arr, std::vector<Vertex_const_handle> &vertices,
    std::map<Vertex_const_handle, DecompVertexData> &decomp) {
  std::vector<
      std::pair<Vertex_const_handle, std::pair<CGAL::Object, CGAL::Object>>>
      vd_list;
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

class Closer_edge
    : public CGAL::cpp98::binary_function<Halfedge_const_handle,
                                          Halfedge_const_handle, bool> {
  typedef Halfedge_const_handle EH;
  typedef Arrangement::Geometry_traits_2 Geometry_traits_2;
  typedef typename Geometry_traits_2::Point_2 Point_2;

  const Geometry_traits_2 *geom_traits;
  Point_2 q;

public:
  Closer_edge() {}
  Closer_edge(const Geometry_traits_2 *traits, const Point_2 &q)
      : geom_traits(traits), q(q) {}

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
    const Point_2 &s1 = e1->source()->point(), t1 = e1->target()->point(),
                  s2 = e2->source()->point(), t2 = e2->target()->point();
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

    CGAL::Orientation e1q =
        CGAL::Visibility_2::orientation_2(geom_traits, s1, t1, q);
    switch (e1q) {
    case CGAL::COLLINEAR:
      if (CGAL::Visibility_2::collinear(geom_traits, q, s2, t2)) {
        // q is collinear with e1 and e2.
        return (CGAL::Visibility_2::less_distance_to_point_2(geom_traits, q, s1,
                                                             s2) ||
                CGAL::Visibility_2::less_distance_to_point_2(geom_traits, q, t1,
                                                             t2));
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
        return CGAL::Visibility_2::orientation_2(geom_traits, s1, t1, t2) !=
               e1q;
      case CGAL::RIGHT_TURN:
        if (CGAL::Visibility_2::orientation_2(geom_traits, s1, t1, t2) ==
            CGAL::LEFT_TURN)
          return CGAL::Visibility_2::orientation_2(geom_traits, s2, t2, q) ==
                 CGAL::Visibility_2::orientation_2(geom_traits, s2, t2, s1);
        else
          return false;
      case CGAL::LEFT_TURN:
        if (CGAL::Visibility_2::orientation_2(geom_traits, s1, t1, t2) ==
            CGAL::RIGHT_TURN)
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
        return CGAL::Visibility_2::orientation_2(geom_traits, s1, t1, t2) !=
               e1q;
      case CGAL::LEFT_TURN:
        if (CGAL::Visibility_2::orientation_2(geom_traits, s1, t1, t2) ==
            CGAL::RIGHT_TURN)
          return CGAL::Visibility_2::orientation_2(geom_traits, s2, t2, q) ==
                 CGAL::Visibility_2::orientation_2(geom_traits, s2, t2, s1);
        else
          return false;
      case CGAL::RIGHT_TURN:
        if (CGAL::Visibility_2::orientation_2(geom_traits, s1, t1, t2) ==
            CGAL::LEFT_TURN)
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

#define INVALID_TRAPEZOID_ID -1

class Trapezoid {
private:
  static unsigned int ID_COUNTER;
  unsigned int id;

public:
  Kernel::FT angle_begin; // TODO remove
  Kernel::FT angle_end;   // TODO remove
  Halfedge_const_handle top_edge;
  Halfedge_const_handle bottom_edge;
  Vertex_const_handle left_vertex;
  Vertex_const_handle right_vertex;
  Trapezoid() = default;
  Trapezoid(Halfedge_const_handle top_edge, Halfedge_const_handle bottom_edge,
            Vertex_const_handle left_vertex, Vertex_const_handle right_vertex)
      : id(++ID_COUNTER), top_edge(top_edge), bottom_edge(bottom_edge),
        left_vertex(left_vertex), right_vertex(right_vertex) {}
  Trapezoid(const Trapezoid &) = default;
  unsigned int get_id() const { return id; }
};
unsigned int Trapezoid::ID_COUNTER = 0;

struct VertexData {
  unsigned int top_left_trapezoid;
  unsigned int top_right_trapezoid;
  unsigned int bottom_left_trapezoid;
  unsigned int bottom_right_trapezoid;
  std::set<Halfedge_const_handle, Closer_edge> ray_edges;
  VertexData() {}
  VertexData(Point &v, const Arrangement::Geometry_traits_2 *geom_traits) {
    top_left_trapezoid = top_right_trapezoid = INVALID_TRAPEZOID_ID;
    bottom_left_trapezoid = bottom_right_trapezoid = INVALID_TRAPEZOID_ID;
    ray_edges = std::set<Halfedge_const_handle, Closer_edge>(
        Closer_edge(geom_traits, v));
  }
};

class Less_edge
    : public CGAL::cpp98::binary_function<Halfedge_const_handle,
                                          Halfedge_const_handle, bool> {
  const Arrangement::Geometry_traits_2 *geom_traits;

public:
  Less_edge() {}
  Less_edge(const Arrangement::Geometry_traits_2 *traits)
      : geom_traits(traits) {}
  bool operator()(Halfedge_const_handle e1, Halfedge_const_handle e2) const {
    if (e1 == e2 || e1 == e2->twin())
      return false;
    else
      return &(*e1) < &(*e2);
  }
};

template <typename OP>
static void foreach_vertex_edge(Vertex_const_handle v, OP op) {
  auto edge = v->incident_halfedges();
  for (auto edges_end = edge;;) {
    auto directed_edge = edge->source() == v ? edge : edge->twin();
    assert(directed_edge->source() == v);
    op(directed_edge);
    if (++edge == edges_end)
      break;
  }
}

static void create_arrangement(Arrangement &arr,
                               const std::vector<Segment> &segments) {
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
    foreach_vertex_edge(v, [&faces](auto &e) { faces.insert(e->face()); });
  if (faces.size() != 2)
    throw std::invalid_argument("Invalid faces number, expected 2.");
  // Expecting one bounded and one unbounded faces. The bounded is the interiour
  // of the room. std::cout << "\t" << face->is_unbounded() << std::endl;
}

static bool is_free(Face_const_handle face) { return !face->is_unbounded(); }

static Kernel::FT calc_angle(Point source, Point target) {
  if (source.hx() != target.hx())
    return (target.hy() - source.hy()) / (target.hx() - source.hx());
  else
    return source.hy() > target.hy() ? -10000 : 10000;
  // TODO #define  numeric_limits<T>::min()
}

static void direct_line(Point &p1, Point &p2) {
  if (p1.hx() != p2.hx()) {
    if (p1.hx() > p2.hx())
      std::swap(p1, p2);
  } else if (p1.hy() > p2.hy())
    std::swap(p1, p2);
}

static void get_source_target(Halfedge_const_handle e, Point &p1, Point &p2) {
  p1 = e->source()->point();
  p2 = e->target()->point();
  direct_line(p1, p2);
}

static Kernel::FT calc_edge_angle(Halfedge_const_handle e) {
  auto source = e->source()->point(), target = e->target()->point();
  direct_line(source, target);
  return calc_angle(source, target);
}

static bool find_vertex_left_edge_with_min_angle(Vertex_const_handle v,
                                                 Halfedge_const_handle &res) {
  bool found = false;
  Halfedge_const_handle best;
  foreach_vertex_edge(v, [&v, &found, &best](auto &edge) {
    auto source = edge->source()->point(), target = edge->target()->point();
    direct_line(source, target);
    if (source.hx() < v->point().hx()) { // TODO maybe <=
      if (!found || calc_angle(source, target) < calc_edge_angle(best)) {
        best = edge;
        found = true;
      }
    }
  });
  if (found)
    res = best;
  return found;
}

static bool find_vertex_left_edge_with_max_angle(Vertex_const_handle v,
                                                 Halfedge_const_handle &res) {
  bool found = false;
  Halfedge_const_handle best;
  foreach_vertex_edge(v, [&v, &found, &best](auto &edge) {
    auto source = edge->source()->point(), target = edge->target()->point();
    direct_line(source, target);
    if (source.hx() < v->point().hx()) { // TODO maybe <=
      Kernel::FT angle;
      if (!found ||
          (angle = calc_angle(source, target)) > calc_edge_angle(best)) {
        best = edge;
        found = true;
      }
    }
  });
  if (found)
    res = best;
  return found;
}

static void update_left_vertex_trapezoid_data(const Trapezoid &trapezoid,
                                              VertexData &left_v_data) {
  bool left_on_top =
      trapezoid.top_edge->curve().line().has_on(trapezoid.left_vertex->point());
  bool left_on_bottom = trapezoid.bottom_edge->curve().line().has_on(
      trapezoid.left_vertex->point());
  if (!left_on_top || left_on_bottom)
    left_v_data.top_right_trapezoid = trapezoid.get_id();
  if (!left_on_bottom || left_on_top)
    left_v_data.bottom_right_trapezoid = trapezoid.get_id();
}

static void init_trapezoids_with_regular_vertical_decomposition(
    const Arrangement &arr, std::map<unsigned int, Trapezoid> &trapezoids,
    std::unordered_map<Vertex_const_handle, VertexData> &vtrapezoids) {
  std::vector<Vertex_const_handle> vertices;
  std::map<Vertex_const_handle, DecompVertexData> decomp;
  vertical_decomposition(arr, vertices, decomp);

  std::map<Halfedge_const_handle, Vertex_const_handle, Less_edge>
      most_right_vertex(Less_edge(arr.geometry_traits()));
  for (const auto &v : vertices) {
    const DecompVertexData &v_decomp_data = decomp[v];
    Halfedge_const_handle top_edge, bottom_edge;

    if ((v_decomp_data.is_edge_above &&
         is_free(v_decomp_data.edge_above->face())) &&
        (v_decomp_data.is_edge_below &&
         is_free(v_decomp_data.edge_below->face())) &&
        !find_vertex_left_edge_with_min_angle(v, top_edge)) {

      // Reflex (more than 180 degrees) vertex
      std::cout << "trapezoid from reflex vertex " << v->point() << std::endl;
      Vertex_const_handle left_v = most_right_vertex[v_decomp_data.edge_above];
      Trapezoid trapezoid(v_decomp_data.edge_above, v_decomp_data.edge_below,
                          left_v, v);
      auto t_id = trapezoid.get_id();
      trapezoids[t_id] = trapezoid;

      vtrapezoids[v].top_left_trapezoid = t_id;
      vtrapezoids[v].bottom_left_trapezoid = t_id;
      update_left_vertex_trapezoid_data(trapezoid, vtrapezoids[left_v]);

      most_right_vertex[trapezoid.top_edge] = v;

    } else if ((!v_decomp_data.is_edge_above ||
                !is_free(v_decomp_data.edge_above->face())) &&
               (!v_decomp_data.is_edge_below ||
                !is_free(v_decomp_data.edge_below->face())) &&
               find_vertex_left_edge_with_min_angle(v, top_edge) &&
               find_vertex_left_edge_with_max_angle(v, bottom_edge)) {

      // Triangle cell terminates at v
      std::cout << "triangle trapezoid " << v->point() << std::endl;
      Vertex_const_handle left_v = most_right_vertex[top_edge];
      Trapezoid trapezoid(top_edge, bottom_edge, left_v, v);
      auto t_id = trapezoid.get_id();
      trapezoids[t_id] = trapezoid;

      vtrapezoids[v].top_left_trapezoid = t_id;
      vtrapezoids[v].bottom_left_trapezoid = t_id;
      update_left_vertex_trapezoid_data(trapezoid, vtrapezoids[left_v]);

    } else {
      if (v_decomp_data.is_edge_above &&
          is_free(v_decomp_data.edge_above->face())) {

        // Edge above the vertex
        std::cout << "trapezoid up " << v->point() << std::endl;
        if (!find_vertex_left_edge_with_min_angle(v, bottom_edge))
          throw std::logic_error("find_vertex_left_edge_with_min_angle fail");

        Vertex_const_handle left_v =
            most_right_vertex[v_decomp_data.edge_above];
        Trapezoid trapezoid(v_decomp_data.edge_above, bottom_edge, left_v, v);
        auto t_id = trapezoid.get_id();
        trapezoids[t_id] = trapezoid;

        vtrapezoids[v].top_left_trapezoid = t_id;
        update_left_vertex_trapezoid_data(trapezoid, vtrapezoids[left_v]);

        most_right_vertex[trapezoid.top_edge] = v;
      }

      if (v_decomp_data.is_edge_below &&
          is_free(v_decomp_data.edge_below->face())) {

        // Edge below the vertex
        std::cout << "trapezoid down " << v->point() << std::endl;
        if (!find_vertex_left_edge_with_max_angle(v, top_edge))
          throw std::logic_error("find_vertex_left_edge_with_max_angle fail");

        Vertex_const_handle left_v = most_right_vertex[top_edge];
        Trapezoid trapezoid(top_edge, v_decomp_data.edge_below, left_v, v);
        auto t_id = trapezoid.get_id();
        trapezoids[t_id] = trapezoid;

        vtrapezoids[v].top_left_trapezoid = t_id;
        update_left_vertex_trapezoid_data(trapezoid, vtrapezoids[left_v]);
      }
    }

    foreach_vertex_edge(v, [&v, &most_right_vertex](auto &edge) {
      most_right_vertex[edge] = v;
    });
  }

  // TODO remove
  std::cout << "After regular vertical decomposition, trapezoids:" << std::endl;
  for (const auto &t : trapezoids) {
    const auto &trapezoid = t.second;
    std::cout << "\tT" << trapezoid.get_id() << " Top("
              << trapezoid.top_edge->curve() << ") Bottom("
              << trapezoid.bottom_edge->curve() << ") Left("
              << trapezoid.left_vertex->point() << ") Right("
              << trapezoid.right_vertex->point() << ")" << std::endl;
  }
  std::cout << "After regular vertical decomposition, vtrapezoids:"
            << std::endl;
  for (const auto &v_pair : vtrapezoids) {
    const auto &v = v_pair.first;
    const auto &v_data = v_pair.second;
    std::cout << "\t(" << v->point() << "):"
              << " TopLeftT " << (int)v_data.top_left_trapezoid << " TopRightT "
              << (int)v_data.top_right_trapezoid << " BottomLeftT "
              << (int)v_data.bottom_left_trapezoid << " BottomRightT "
              << (int)v_data.bottom_right_trapezoid << std::endl;
  }
}

class Event {
public:
  Vertex_const_handle v1;
  Vertex_const_handle v2;

  Event(Vertex_const_handle v1, Vertex_const_handle v2) : v1(v1), v2(v2) {}
};

static int calc_trapezoids(const std::vector<Segment> &segments) {
  // Create arrangement
  Arrangement arr;
  create_arrangement(arr, segments);

  std::map<unsigned int, Trapezoid> trapezoids;
  std::unordered_map<Vertex_const_handle, VertexData> vtrapezoids(
      arr.number_of_vertices());
  for (auto v = arr.vertices_begin(); v != arr.vertices_end(); ++v)
    vtrapezoids[v] = VertexData(v->point(), arr.geometry_traits());

  // Init trapezoids with regular vertical decomposition
  init_trapezoids_with_regular_vertical_decomposition(arr, trapezoids,
                                                      vtrapezoids);

  throw std::logic_error("enough for now");

  // Calc all events
  std::vector<Event> events;
  events.reserve(arr.number_of_vertices() * arr.number_of_vertices());
  for (auto v1 = arr.vertices_begin(); v1 != arr.vertices_end(); ++v1)
    for (auto v2 = arr.vertices_begin(); v2 != arr.vertices_end(); ++v2)
      if (v1 != v2)
        events.push_back(Event(v1, v2));

  // Sort events by their angle
  sort(events.begin(), events.end(), [](const Event &e1, const Event &e2) {
    return calc_angle(e1.v2->point(), e1.v1->point()) <
           calc_angle(e2.v2->point(), e2.v1->point());
  });

  DEBUG_PRINT();
  for (auto &event : events) {
    auto ray_edges = &vtrapezoids.find(event.v1)->second.ray_edges;
    auto closest_edge_orig = *ray_edges->begin();

    // Maintaine ray_edges
    foreach_vertex_edge(event.v1, [&ray_edges](auto &edge) {
      if (ray_edges->find(edge) == ray_edges->end())
        ray_edges->insert(edge);
      else
        ray_edges->erase(edge);
    });

    DEBUG_PRINT();
    auto closest_edge = *ray_edges->begin();
    if (closest_edge_orig == closest_edge)
      continue; // Closest edge didn't changed

    // auto current_angle = event.angle;
    auto bottom_event_vertex_data = &vtrapezoids.find(event.v1)->second;
    auto top_event_vertex_data = &vtrapezoids.find(event.v2)->second;
    auto left_trapezoid_id = top_event_vertex_data->bottom_left_trapezoid;
    auto mid_trapezoid_id = bottom_event_vertex_data->top_left_trapezoid;
    auto right_trapezoid_id = bottom_event_vertex_data->top_right_trapezoid;

    DEBUG_PRINT();
    if (mid_trapezoid_id != INVALID_TRAPEZOID_ID) {
      Trapezoid &mid = trapezoids[mid_trapezoid_id];
      // mid->angle_end = current_angle;
      // Trapezoid trapezoid(closest_edge, right.bottom_edge, event.v1,
      // event.v2); trapezoid.angle_begin = current_angle;
      // TODO
    }
    if (left_trapezoid_id != INVALID_TRAPEZOID_ID) {
      Trapezoid &left = trapezoids[left_trapezoid_id];
      // left->angle_end = current_angle;
      Trapezoid trapezoid(left.top_edge, left.bottom_edge, left.left_vertex,
                          event.v1);
      // trapezoid.angle_begin = current_angle;
      // TODO
    }
    if (right_trapezoid_id != INVALID_TRAPEZOID_ID) {
      Trapezoid &right = trapezoids[right_trapezoid_id];
      // right->angle_end = current_angle;
      Trapezoid trapezoid(right.top_edge, right.bottom_edge, event.v2,
                          right.right_vertex);
      // trapezoid.angle_begin = current_angle;
      trapezoids[trapezoid.get_id()] = trapezoid;
      // TODO get a pointer to the inserted element
      // v2_data->bottom_right_trapezoid = &trapezoids[last_elm];
      // TODO check if left vertex is top or bottom
      // vtrapezoids.find(nt.right_vertex)->second.top_left_trapezoid =
      // &trapezoids[last_elm];
      // TODO check if right vertex is top or bottom
    }
  }

  return 0;
}

static void create_segments_from_points(const std::vector<Point> &points,
                                        std::vector<Segment> &segments) {
  for (unsigned int i = 0; i < points.size(); i++) {
    unsigned int j = i != points.size() - 1 ? i + 1 : 0;
    Segment s(points[i], points[j]);
    segments.push_back(s);
  }
}

int main() {
  std::vector<Point> points = {
      Point(0, 0), Point(2, 3), Point(5, 4), Point(4, 2), Point(5, 0),
  };
  std::vector<Segment> segments;
  create_segments_from_points(points, segments);

  calc_trapezoids(segments);

  return 0;
}
