#include "fdml/trapezoid.hpp"
#include "fdml/internal/utils.hpp"

#include <CGAL/Boolean_set_operations_2.h>
#include <CGAL/Boolean_set_operations_2/Gps_polygon_validation.h>
#include <CGAL/Polygon_with_holes_2.h>
#include <CGAL/enum.h>

namespace FDML {

const Direction Trapezoid::ANGLE_NONE(0, 0);

Trapezoid::Trapezoid(Trapezoid::ID id, Halfedge top_edge, Halfedge bottom_edge, Vertex left_vertex, Vertex right_vertex)
    : id(id), top_edge(top_edge), bottom_edge(bottom_edge), left_vertex(left_vertex), right_vertex(right_vertex),
      angle_begin(Trapezoid::ANGLE_NONE), angle_end(Trapezoid::ANGLE_NONE) {}

Trapezoid::ID Trapezoid::get_id() const {
  return id;
}

/* rotate a direction by a given angle (radians) */
template <typename _Direction> static _Direction rotate(const _Direction& d, double r) {
  return d.transform(Kernel::Aff_transformation_2(CGAL::Rotation(), std::sin(r), std::cos(r)));
}

static Direction get_mid_angle(Direction angle_begin, Direction angle_end) {
  auto v_begin = Utils::normalize(angle_begin.vector()), v_end = Utils::normalize(angle_end.vector());
  double angle_between = std::acos(CGAL::to_double(v_begin * v_end));
  assert(angle_between != 0);
  double a_mid = angle_between / 2;
  return rotate(v_begin, a_mid).direction();
}

/* Calculate which of an edge endpoint is "left" and "right" relative to some direction */
static void calc_edge_left_right_vertices(const Halfedge& edge, const Direction& dir, Point& left, Point& right) {
  Point p1 = edge->source()->point(), p2 = edge->target()->point();
  if (Line({0, 0}, dir).oriented_side({(p1.x() - p2.x()) / 2, (p1.y() - p2.y()) / 2}) == CGAL::ON_POSITIVE_SIDE) {
    left = p1;
    right = p2;
  } else {
    left = p2;
    right = p1;
  }
}

Polygon Trapezoid::get_bounds_2d() const {
  auto v_mid = get_mid_angle(angle_begin, angle_end);

  /* Calculate left and right vertices of the top edge relative to the trapezoid's direction */
  Point top_left, top_right;
  calc_edge_left_right_vertices(top_edge, v_mid, top_left, top_right);

  /* Calculate left and right vertices of the bottom edge relative to the trapezoid's direction */
  Point bottom_left, bottom_right;
  calc_edge_left_right_vertices(bottom_edge, v_mid, bottom_left, bottom_right);

  /* construct the bounds polygon. Might used only 3 vertices if top and bottom edge share a vertex */
  std::vector<Point> points;
  if (top_left == bottom_left)
    points = {top_right, top_left, bottom_right};
  else if (top_right == bottom_right)
    points = {top_right, top_left, bottom_left};
  else
    points = {top_right, top_left, bottom_left, bottom_right};
  Polygon bounds(points.begin(), points.end());

  CGAL::Gps_default_traits<Polygon>::Traits traits;
  assert(CGAL::has_valid_orientation_polygon(bounds, traits));
  return bounds;
}

/* calculate the intersection point of two lines */
static Point intersection(const Line& l1, const Line& l2) {
  auto res = CGAL::intersection(l1, l2);
  assert(!res->empty());
  return boost::get<Point>(res.get());
}

/* We use a polygon approximation to repsent the complex curves of the result. These defines determine the percision of
 * the approximation. All approximations are done by discretizing the angle. The defines below define how many discrete
 * step will be used in a 2*PI angle, relative approximation will be used for other angles. */
static const unsigned int ARC_APPX_POINTS_NUM = 360;
static const unsigned int CONCHOID_APPX_POINTS_NUM = 360;
static const unsigned int ELLIPSE_APPX_POINTS_NUM = 360;

static Direction edge_direction(const Halfedge& edge) {
  auto s = edge->source()->point(), t = edge->target()->point();
  return Direction(t.x() - s.x(), t.y() - s.y());
}

std::vector<Polygon> Trapezoid::calc_result_m1(const Kernel::FT& d) const {
  if (d <= 0)
    throw std::invalid_argument("distance measurement must be positive.");
  fdml_debugln("[Trapezoid] calculating single measurement result...");
  /* oriante angles relative to the top edge */
  Direction a_begin = -angle_begin, a_end = -angle_end;
  assert(Line({0, 0}, a_begin).oriented_side({a_end.dx(), a_end.dy()}) == CGAL::ON_POSITIVE_SIDE);
  Direction top_edge_direction = edge_direction(top_edge);

  /* Calculate the trapezoid bounds. Will be used to intersect each result entry. */
  const Polygon trapezoid_bounds = get_bounds_2d();
  fdml_debugln("\ttrapezoid bounds (" << trapezoid_bounds << ')');

  /* calculate the mid angle, which is perpendicular to the top edge, and use it to split the trapezoid angle
   * interval into 2 to ensure simple polygon output for each result entry. */
  Direction mid_angle = top_edge_direction.perpendicular(CGAL::LEFT_TURN);
  bool begin_before_mid = Line({0, 0}, mid_angle).oriented_side({a_begin.dx(), a_begin.dy()}) == CGAL::ON_NEGATIVE_SIDE;
  bool end_after_mid = Line({0, 0}, mid_angle).oriented_side({a_end.dx(), a_end.dy()}) == CGAL::ON_POSITIVE_SIDE;
  Direction angle_intervals[2][2] = {{begin_before_mid ? a_begin : mid_angle, end_after_mid ? mid_angle : a_end},
                                     {begin_before_mid ? mid_angle : a_begin, end_after_mid ? a_end : mid_angle}};

  fdml_debugln("\ttop edge (" << top_edge->curve() << ") bottom edge (" << bottom_edge->curve() << ')');

  std::vector<Polygon> res;

  /* calculate result for each angle interval */
  for (unsigned int internal_idx = 0; internal_idx < 2; internal_idx++) {
    auto& angle_interval = angle_intervals[internal_idx];
    bool before_mid = internal_idx == 0;
    auto i_begin = angle_interval[0], i_end = angle_interval[1];
    if (i_begin == i_end)
      continue; /* ignore if the angle interval is empty */

    fdml_debugln("\tangle interval [" << i_begin << ", " << i_end << ']');
    auto top_edge_line = top_edge->curve().line();
    auto v_begin = Utils::normalize(i_begin.vector()), v_end = Utils::normalize(i_end.vector());
    double angle_between = std::acos(CGAL::to_double(v_begin * v_end));
    assert(angle_between != 0);
    fdml_debugln("\tv_begin(" << v_begin << ") v_end(" << v_end << ')');

    /* calculate the points representing the curves in both sides of the trapezoid */
    std::vector<Point> left_points, right_points;
    const auto LEFT = 0, RIGHT = 1;
    for (auto side : {LEFT, RIGHT}) {
      auto vertex = (side == LEFT ? left_vertex : right_vertex)->point();
      auto& points = side == LEFT ? left_points : right_points;

      if (top_edge_line.has_on(vertex)) {
        /* Arc curve */
        fdml_debugln("\tcurve " << (side == LEFT ? "left" : "right") << " is arc");
        Point begin = vertex + v_begin * d;
        Point end = vertex + v_end * d;
        unsigned int appx_num = (unsigned int)((std::abs(angle_between) / (M_PI * 2)) * ARC_APPX_POINTS_NUM);

        /* approximate all points of the curve by used angle steps */
        points.push_back(begin);
        for (unsigned int i = 1; i < appx_num; i++) {
          Direction dir = rotate(i_begin, i * angle_between / appx_num);
          points.emplace_back(vertex + Utils::normalize(dir.vector()) * d);
        }
        points.push_back(end);
        fdml_debugln("\t\tO(" << vertex << ") r(" << d << ") B(" << begin << ") E(" << end << ')');

      } else {
        /* Conchoid curve */
        fdml_debugln("\tcurve " << (side == LEFT ? "left" : "right") << " is conchoid");
        Point begin = intersection(top_edge_line, Line(vertex, i_begin)) + v_begin * d;
        Point end = intersection(top_edge_line, Line(vertex, i_end)) + v_end * d;
        unsigned int appx_num = (unsigned int)((std::abs(angle_between) / (M_PI * 2)) * CONCHOID_APPX_POINTS_NUM);

        /* approximate all points of the curve by used angle steps */
        points.push_back(begin);
        for (unsigned int i = 1; i < appx_num; i++) {
          Direction dir = rotate(i_begin, i * angle_between / appx_num);
          points.push_back(intersection(top_edge_line, Line(vertex, dir)) + Utils::normalize(dir.vector()) * d);
        }
        points.push_back(end);

        fdml_debugln("\t\tO(" << vertex << ") r(" << d << ") B(" << begin << ") E(" << end << ')');
      }

      fdml_debug("\t\tcurve points:");
      for (auto& p : left_points)
        fdml_debug(" (" << p << ')');
      fdml_debugln("");
    }

    /* construct a simple polygon from the two approximated curves */
    Polygon res_unbounded;
    if (before_mid) {
      /* avoid points duplication if the two curves share start/end vertices */
      auto left_begin = left_points.begin();
      if (*left_begin == right_points.front())
        ++left_begin;
      auto right_begin = right_points.rbegin();
      if (*right_begin == left_points.back())
        right_begin++;
      res_unbounded.insert(res_unbounded.vertices_end(), left_begin, left_points.end());
      res_unbounded.insert(res_unbounded.vertices_end(), right_begin, right_points.rend());
    } else {
      /* avoid points duplication if the two curves share start/end vertices */
      auto left_begin = left_points.rbegin();
      if (*left_begin == right_points.back())
        ++left_begin;
      auto right_begin = right_points.begin();
      if (*right_begin == left_points.front())
        right_begin++;
      res_unbounded.insert(res_unbounded.vertices_end(), left_begin, left_points.rend());
      res_unbounded.insert(res_unbounded.vertices_end(), right_begin, right_points.end());
    }

    /* intersect the result polygon with the trapezoids bound and add the result to the output */
    std::vector<Polygon_with_holes> res_bounded;
    CGAL::intersection(trapezoid_bounds, res_unbounded, std::back_inserter(res_bounded));
    fdml_debugln("\ttrapezoid result:");
    for (const auto& res_cell : res_bounded) {
      assert(res_cell.number_of_holes() == 0);
      res.push_back(res_cell.outer_boundary());
      fdml_debugln("\t\t" << res_cell.outer_boundary());
    }
  }

  return res;
}

static double atan2(const Kernel::FT& y, const Kernel::FT& x) {
  double z = std::atan2(CGAL::to_double(y), CGAL::to_double(x));
  if (z < 0)
    z += 2 * M_PI;
  assert(0 <= z && z <= 2 * M_PI);
  return z;
}

std::vector<Segment> Trapezoid::calc_result_m2(const Kernel::FT& d1, const Kernel::FT& d2) const {
  if (d1 <= 0 || d2 <= 0)
    throw std::invalid_argument("distance measurements must be positive.");
  fdml_debugln("[Trapezoid] calculating double measurement result...");
  Line top_line = top_edge->curve().line();
  Line bottom_line = bottom_edge->curve().line();

  std::vector<Segment> res;

  if (CGAL::do_intersect(top_line, bottom_line)) { /* top and bottom edges are not parallel */
    Point inter_point = intersection(top_line, bottom_line);
    /* angle range between angle_begin and angle_end */
    double angle_range =
        std::acos(CGAL::to_double(Utils::normalize(angle_begin.vector()) * Utils::normalize(angle_end.vector())));
    assert(angle_range != 0);
    Direction top_line_dir = -edge_direction(top_edge), bottom_line_dir = edge_direction(bottom_edge);
    double bottom_line_angle = atan2(bottom_line_dir.dy(), bottom_line_dir.dx());
    double a_begin = atan2(angle_begin.dy(), angle_begin.dx());
    /* angle between top and bottom edges */
    double angle_between = std::acos(
        CGAL::to_double(Utils::normalize(bottom_line_dir.vector()) * Utils::normalize(top_line_dir.vector())));

    /* calc the direction from the intersection point to the middle of top edge. use with k */
    auto top_s = top_edge->source()->point(), top_t = top_edge->target()->point();
    auto k_dir = Utils::normalize(
        Vector((top_s.x() + top_t.x()) / 2 - inter_point.x(), (top_s.y() + top_t.y()) / 2 - inter_point.y()));

    Point prev;
    bool prev_valid = false;
    unsigned int appx_num = (unsigned int)((std::abs(angle_range) / (M_PI * 2)) * ELLIPSE_APPX_POINTS_NUM);
    for (unsigned int i = 0; i <= appx_num; i++) {
      double a = i * angle_range / appx_num;
      Direction dir = rotate(angle_begin, a);
      double t = a_begin + a - bottom_line_angle;
      /* distance of measure point in top edge from intersection point */
      Kernel::FT k = (d1 + d2) * std::sin(t) / std::sin(angle_between);
      auto k_squared = k * k;

      Kernel::FT k_limits_squared[2];
      const auto LEFT = 0, RIGHT = 1;
      for (auto side : {LEFT, RIGHT}) {
        auto vertex = (side == LEFT ? left_vertex : right_vertex)->point();
        auto measure_point = top_line.has_on(vertex) ? vertex : intersection(top_line, Line(vertex, dir));
        k_limits_squared[side] = CGAL::squared_distance(inter_point, measure_point);
      }
      if (k_limits_squared[0] > k_limits_squared[1])
        std::swap(k_limits_squared[0], k_limits_squared[1]);
      if (!(k_limits_squared[0] <= k_squared && k_squared <= k_limits_squared[1])) {
        prev_valid = false;
        continue;
      }

      auto measure_point = inter_point + k_dir * k;
      Point res_point = measure_point + Utils::normalize((-dir).vector()) * d1;
      if (Line(bottom_edge->source()->point(), bottom_line_dir).oriented_side(res_point) == CGAL::ON_NEGATIVE_SIDE) {
        prev_valid = false;
        continue;
      }

      if (prev_valid)
        res.emplace_back(prev, res_point);
      prev = res_point;
      prev_valid = true;
    }

  } else { /* top and bottom are parallel */
    auto lines_dis = CGAL::approximate_sqrt(CGAL::squared_distance(top_line, bottom_line));
    Direction bottom_line_dir = edge_direction(bottom_edge);
    double local_angle = std::asin(CGAL::to_double(lines_dis / (d1 + d2)));
    for (double angle : {local_angle, M_PI - local_angle}) {
      assert(0 <= angle && angle <= M_PI);
      auto dir = rotate(bottom_line_dir, angle);
      if (!dir.counterclockwise_in_between(angle_begin, angle_end))
        continue;

      Point points[2];
      const auto LEFT = 0, RIGHT = 1;
      for (auto side : {LEFT, RIGHT}) {
        auto vertex = (side == LEFT ? left_vertex : right_vertex)->point();
        auto measure_point = top_line.has_on(vertex) ? vertex : intersection(top_line, Line(vertex, dir));
        points[side] = measure_point + Utils::normalize((-dir).vector()) * d1;
      }
      res.emplace_back(points[0], points[1]);
    }
  }

  return res;
}

void Trapezoid::calc_min_max_openings(Kernel::FT& opening_min, Kernel::FT& opening_max) const {
  /* for any fixed angle, the opening function is a affine function, and therefore monotonically increasing or
   * decreasing as a function x. Therefore, to calculate the minimum or the maximum of the opening function we only
   * need to consider the values at the end of the x valid interval of the functions, these are the x values defined
   * by the left and right limiting vertices. */

  auto calc_arc_opening = [this](const Point& vertex, const Direction& angle) {
    Line bottom_line = bottom_edge->curve().line();
    if (bottom_line.has_on(vertex))
      return (Kernel::FT)0;
    Line opening_line = Line(vertex, angle);
    Point inter = intersection(bottom_line, opening_line);
    Kernel::FT xd = vertex.x() - inter.x(), xy = vertex.y() - inter.y();
    return CGAL::approximate_sqrt(xd * xd + xy * xy);
  };
  auto calc_conchoid_opening = [this](const Point& vertex, const Direction& angle) {
    Line top_line = top_edge->curve().line();
    Line bottom_line = bottom_edge->curve().line();
    Line opening_line = Line(vertex, angle);
    Point inter1 = top_line.has_on(vertex) ? vertex : intersection(top_line, opening_line);
    Point inter2 = bottom_line.has_on(vertex) ? vertex : intersection(bottom_line, opening_line);
    Kernel::FT xd = inter1.x() - inter2.x(), xy = inter1.y() - inter2.y();
    return CGAL::approximate_sqrt(xd * xd + xy * xy);
  };
  auto angle_between = [](double low, double high) {
    double r = high - low;
    r = low < high ? r : r + 2 * M_PI;
    assert(r >= 0);
    return r;
  };
  auto dir_to_angle = [](const Direction& dir) { return atan2(dir.dy(), dir.dx()); };
  for (unsigned int side = 0; side < 2; side++) {
    Point limit_vertex = (side == 0 ? left_vertex : right_vertex)->point();
    Kernel::FT max, min;

    if (top_edge->curve().line().has_on(limit_vertex)) {
      /* Arc */
      /* for an arc curve of a limiting vertex, the minimum is always achieved at the angle perpendicular to the
       * bottom edge, but it may not be included in the trapezoid angle interval, and we consider the interval
       * limits as well. The maximum will always be one of the angle interval limits. */
      Kernel::FT m1 = calc_arc_opening(limit_vertex, -angle_begin);
      Kernel::FT m2 = calc_arc_opening(limit_vertex, -angle_end);
      max = CGAL::max(m1, m2);

      Point bottom_left, bottom_right;
      calc_edge_left_right_vertices(bottom_edge, get_mid_angle(angle_begin, angle_end), bottom_left, bottom_right);
      Direction perp = Direction(bottom_right.x() - bottom_left.x(), bottom_right.y() - bottom_left.y())
                           .perpendicular(CGAL::LEFT_TURN);
      if (perp.counterclockwise_in_between(angle_begin, angle_end))
        min = calc_arc_opening(limit_vertex, -perp);
      else
        min = CGAL::min(m1, m2);

    } else {
      /* Conchoid */
      /* for a conchoid curve of a limiting vertex, i failed to calculate analytically the minimum point, and
       * therefore forced to search numerically on the (i think) convex function. The maximum will always be one of the
       * angle interval limits. */
      Kernel::FT m1 = calc_conchoid_opening(limit_vertex, -angle_begin);
      Kernel::FT m2 = calc_conchoid_opening(limit_vertex, -angle_end);
      min = CGAL::min(m1, m2);
      max = CGAL::max(m1, m2);

      double a_low = dir_to_angle(angle_begin), a_high = dir_to_angle(angle_end);
      const double PRECISION = 0.01;
      for (unsigned int iter_num = 0; angle_between(a_low, a_high) > PRECISION; iter_num++) {
        double a = angle_between(a_low, a_high);
        double mid1 = a * 1 / 3, mid2 = a * 2 / 3;
        Kernel::FT mid1_min_opening = calc_conchoid_opening(limit_vertex, rotate(angle_begin, mid1));
        Kernel::FT mid2_min_opening = calc_conchoid_opening(limit_vertex, rotate(angle_begin, mid2));
        if (mid1_min_opening <= mid2_min_opening)
          a_high = a_low + mid2;
        else
          a_low = a_low + mid1;
        if (iter_num == 1000) {
          fdml_errln("Failed to converge on the minimum opening...");
          break;
        }
      }
      double a = angle_between(a_low, a_high) / 2;
      min = CGAL::min(min, calc_conchoid_opening(limit_vertex, rotate(angle_begin, a)));
    }

    if (side == 0) {
      opening_min = min;
      opening_max = max;
    } else {
      opening_min = CGAL::min(opening_min, min);
      opening_max = CGAL::max(opening_max, max);
    }
  }
}

} // namespace FDML
