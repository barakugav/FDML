#include <CGAL/Arr_segment_traits_2.h>
#include <CGAL/Arr_vertical_decomposition_2.h>
#include <CGAL/Arrangement_2.h>
#include <CGAL/Rotational_sweep_visibility_2.h>
#include <CGAL/basic.h>
#include <iostream>
#include <set>

typedef CGAL::Exact_predicates_exact_constructions_kernel Kernel;
typedef CGAL::Segment_2<Kernel> Segment;
typedef Kernel::Point_2 Point;
typedef CGAL::Arr_segment_traits_2<Kernel> Traits;
typedef CGAL::Arrangement_2<Traits> Arrangement;

typedef Arrangement::Vertex_const_handle Vertex_const_handle;
typedef std::pair<Vertex_const_handle, std::pair<CGAL::Object, CGAL::Object>>
    Vert_decomp_entry;
typedef std::list<Vert_decomp_entry> Vert_decomp_list;

typedef Arrangement::Halfedge_const_handle Halfedge_const_handle;
typedef Arrangement::Face_const_handle Face_const_handle;

typedef CGAL::Rotational_sweep_visibility_2<Arrangement> Rotational_sweep;

#define ARR_SIZE(a) (sizeof(a) / sizeof(a[0]))

static int calc_trapezoids_old() {
  Segment segments[] = {
      Segment(Point(0, 0), Point(3, 3)), Segment(Point(3, 3), Point(6, 0)),
      Segment(Point(2, 0), Point(5, 3)), Segment(Point(5, 3), Point(8, 0)),
      Segment(Point(8, 0), Point(0, 0))};
  Arrangement arr;
  insert(arr, segments, segments + ARR_SIZE(segments));

  // Polygon_vertical_decomposition d;
  // d.init();

  // d(arr);
  // // Polygon_vertical_decomposition::vertical_decomposition(arr)

  Vert_decomp_list vd_list;
  CGAL::decompose(arr, std::back_inserter(vd_list));

  std::cout << "Hello World! size = " << vd_list.size() << std::endl;

  //   Print the results.
  for (auto it = vd_list.begin(); it != vd_list.end(); ++it) {
    std::cout << "Vertex (" << it->first->point() << ") : ";
    // Vertex_const_handle vh;
    // Halfedge_const_handle hh;
    // Face_const_handle fh;
    // std::cout << " feature below: ";
    // if (hh.assign(it->first))
    // if (CGAL::assign(hh, it->first))
    // std::cout << '[' << hh->curve() << ']';
    // else if (CGAL::assign(vh, it->first))
    // 	std::cout << '(' << vh->point() << ')';
    // else if (CGAL::assign(fh, it->first))
    // 	std::cout << "NONE";
    // else
    // 	std::cout << "EMPTY";
    // std::cout << "   feature above: ";
    // if (CGAL::assign(hh, it->second))
    // 	std::cout << '[' << hh->curve() << "]\n";
    // else if (CGAL::assign(vh, it->second))
    // 	std::cout << '(' << vh->point() << ")\n";
    // else if (CGAL::assign(fh, it->second))
    // 	std::cout << "NONE\n";
    // else
    // 	std::cout << "EMPTY\n";
  }

  // boost::OutputIterator oi;
  // CGAL::decompose(arrm oi);

  return 0;
}

// typedef CGAL::Lazy_exact_nt<typename Exact_kernel::FT>  FT;

#include <CGAL/Visibility_2/visibility_utils.h>

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
      //          const Point_2& p1 = s1,
      //                   p2 = t2,
      //                   c = s2;
      int vt1 = vtype(t1, s1), vt2 = vtype(t1, t2);
      if (vt1 != vt2)
        return vt1 > vt2;
      else
        return (CGAL::Visibility_2::orientation_2(geom_traits, s2, t2, s1) ==
                CGAL::Visibility_2::orientation_2(geom_traits, s2, t2, q));
    }

    if (e1->source() == e2->target()) {
      //            const Point_2& p1 = t1,
      //                     p2 = s2,
      //                     c = s1;
      int vt1 = vtype(s1, t1), vt2 = vtype(s1, s2);
      if (vt1 != vt2)
        return vt1 > vt2;
      else
        return (CGAL::Visibility_2::orientation_2(geom_traits, s1, s2, t1) ==
                CGAL::Visibility_2::orientation_2(geom_traits, s1, s2, q));
    }

    if (e1->target() == e2->target()) {
      //              const Point_2& p1 = s1,
      //                       p2 = s2,
      //                       c = t1;
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

class Event {
public:
  Arrangement::Vertex_const_handle v1;
  Arrangement::Vertex_const_handle v2;
  Kernel::FT angle;

  Event(Arrangement::Vertex_const_handle v1,
        Arrangement::Vertex_const_handle v2, Kernel::FT angle)
      : v1(v1), v2(v2), angle(angle) {}
};

class Trapezoid {
public:
  Kernel::FT angle_begin;
  Kernel::FT angle_end;
  Arrangement::Halfedge_const_handle top_edge;
  Arrangement::Halfedge_const_handle bottom_edge;
  Arrangement::Vertex_const_handle left_vertex;
  Arrangement::Vertex_const_handle right_vertex;
};

static int calc_trapezoids(std::vector<Segment> segments) {
  Arrangement arr;
  insert(arr, segments.begin(), segments.end());

  std::cout << "arr.number_of_vertices() = " << arr.number_of_vertices()
            << std::endl;

  std::vector<Event> events;
  events.reserve(arr.number_of_vertices() * arr.number_of_vertices());
  for (auto vit1 = arr.vertices_begin(); vit1 != arr.vertices_end(); ++vit1) {
    Point v1 = vit1->point();
    for (auto vit2 = arr.vertices_begin(); vit2 != arr.vertices_end(); ++vit2) {
      Point v2 = vit2->point();
      if (v1 == v2)
        continue;

      Kernel::FT angle;
      if (v2.hx() != v1.hx())
        angle = (v2.hy() - v1.hy()) / (v2.hx() - v1.hx());
      else
        angle = v1.hy() > v2.hy() ? 10000 : -10000; // TODO define

      Event event(vit1, vit2, angle);
      events.push_back(event);
    }
  }

  // Sort events by their angle
  sort(events.begin(), events.end(),
       [](const Event &e1, const Event &e2) { return e1.angle < e2.angle; });

  for (auto it = events.begin(); it != events.end(); ++it)
    std::cout << "(" << it->v1->point() << ")"
              << "(" << it->v2->point() << ") " << it->angle << std::endl;

  struct VertexData {
    Trapezoid *top_left_trapezoid;
    Trapezoid *top_right_trapezoid;
    Trapezoid *bottom_left_trapezoid;
    Trapezoid *bottom_right_trapezoid;
    std::set<Arrangement::Halfedge_const_handle, Closer_edge> ray_edges;
    VertexData() {}
    VertexData(Point &v, const Arrangement::Geometry_traits_2 *geom_traits) {
      top_left_trapezoid = top_right_trapezoid = NULL;
      bottom_left_trapezoid = bottom_right_trapezoid = NULL;
      ray_edges = std::set<Arrangement::Halfedge_const_handle, Closer_edge>(
          Closer_edge(geom_traits, v));
    }
  };

  // auto point_hash = [](const Point &p) {
  //   auto x = p.hx().exact(), y = p.hy().exact();
  //   std::hash<decltype(x)> h;
  //   return h(x) ^ h(y);
  // };
  // std::unordered_map<Point, VertexData, decltype(point_hash)> vtrapezoids(
  //     arr.number_of_vertices(), point_hash);

  std::vector<Trapezoid> trapezoids;
  std::unordered_map<Arrangement::Vertex_const_handle, VertexData> vtrapezoids(
      arr.number_of_vertices());
  for (auto vit = arr.vertices_begin(); vit != arr.vertices_end(); ++vit) {
    if (vtrapezoids.find(vit) != vtrapezoids.end())
      std::cout << "more than one vertex with same location" << std::endl;
    vtrapezoids[vit] = VertexData(vit->point(), arr.geometry_traits());
  }

  // TODO perform first vertical decomposition and init vtrapezoids

  for (auto event = events.begin(); event != events.end(); ++event) {
    auto ray_edges = &vtrapezoids.find(event->v1)->second.ray_edges;
    auto closest_edge_orig = *ray_edges->begin();

    // Maintaine ray_edges
    auto v_edge = event->v1->incident_halfedges();
    auto v_edges_end = v_edge;
    do {
      if (ray_edges->find(v_edge) == ray_edges->end())
        ray_edges->insert(v_edge);
      else
        ray_edges->erase(v_edge);
    } while (++v_edge != v_edges_end);

    auto closest_edge = *ray_edges->begin();
    if (closest_edge_orig == closest_edge)
      continue; // Closest edge didn't changed

    auto current_angle = event->angle;
    auto bottom_event_vertex_data = &vtrapezoids.find(event->v1)->second;
    auto top_event_vertex_data = &vtrapezoids.find(event->v2)->second;
    Trapezoid *left = top_event_vertex_data->bottom_left_trapezoid;
    Trapezoid *mid = bottom_event_vertex_data->top_left_trapezoid;
    Trapezoid *right = bottom_event_vertex_data->top_right_trapezoid;

    if (mid) {
      mid->angle_end = current_angle;
      Trapezoid nt;
      nt.top_edge = closest_edge;
      nt.bottom_edge = right->bottom_edge;
      nt.left_vertex = event->v1;
      nt.right_vertex = event->v2;
      // TODO
    }
    if (left) {
      left->angle_end = current_angle;
      Trapezoid nt;
      nt.top_edge = left->top_edge;
      nt.bottom_edge = left->bottom_edge;
      nt.left_vertex = left->left_vertex;
      nt.right_vertex = event->v1;
      // TODO
    }
    if (right) {
      right->angle_end = current_angle;
      Trapezoid nt;
      nt.angle_begin = current_angle;
      nt.top_edge = right->top_edge;
      nt.bottom_edge = right->bottom_edge;
      nt.left_vertex = event->v2;
      nt.right_vertex = right->right_vertex;
      trapezoids.push_back(nt);
      // TODO get a pointer to the inserted element
      // v2_data->bottom_right_trapezoid = &trapezoids[last_elm];
      // TODO check if left vertex is top or bottom
      // vtrapezoids.find(nt.right_vertex)->second.top_left_trapezoid =
      // &trapezoids[last_elm];
      // TODO check if right vertex is top or bottom
    }
  }

  Vert_decomp_list vd_list;
  CGAL::decompose(arr, std::back_inserter(vd_list));

  std::cout << "Hello World! size = " << vd_list.size() << std::endl;

  //   Print the results.
  for (auto it = vd_list.begin(); it != vd_list.end(); ++it) {
    std::cout << "Vertex (" << it->first->point() << ") : ";
    // Vertex_const_handle vh;
    // Halfedge_const_handle hh;
    // Face_const_handle fh;
    // std::cout << " feature below: ";
    // if (hh.assign(it->first))
    // if (CGAL::assign(hh, it->first))
    // std::cout << '[' << hh->curve() << ']';
    // else if (CGAL::assign(vh, it->first))
    // 	std::cout << '(' << vh->point() << ')';
    // else if (CGAL::assign(fh, it->first))
    // 	std::cout << "NONE";
    // else
    // 	std::cout << "EMPTY";
    // std::cout << "   feature above: ";
    // if (CGAL::assign(hh, it->second))
    // 	std::cout << '[' << hh->curve() << "]\n";
    // else if (CGAL::assign(vh, it->second))
    // 	std::cout << '(' << vh->point() << ")\n";
    // else if (CGAL::assign(fh, it->second))
    // 	std::cout << "NONE\n";
    // else
    // 	std::cout << "EMPTY\n";
  }

  return 0;
}

int main() {
  std::cout << "Hello World!" << std::endl;

  std::vector<Segment> segments = {
      Segment(Point(0, 0), Point(3, 3)), Segment(Point(3, 3), Point(6, 0)),
      Segment(Point(2, 0), Point(5, 3)), Segment(Point(5, 3), Point(8, 0)),
      Segment(Point(8, 0), Point(0, 0))};

  calc_trapezoids(segments);

  return 0;
}
