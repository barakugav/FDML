#ifndef FDML_LOCATOR_HPP
#define FDML_LOCATOR_HPP

#include "fdml/config.hpp"
#include "fdml/defs.hpp"
#include "fdml/trapezoider.hpp"

#include <boost/geometry.hpp>

namespace FDML {

/**
 * @brief The locator class is used to preproccess a polygon room, and to query the possible positions a
 * sensor might be in the room given one or two measeraments.
 *
 */
class FDML_FDML_DECL Locator {
private:
  struct TrapezoidOpening {
    Kernel::FT min;
    Kernel::FT max;
    TrapezoidOpening(const Kernel::FT& min, const Kernel::FT& max) : min(min), max(max){};
  };

  typedef boost::geometry::model::point<Kernel::FT, 1, boost::geometry::cs::cartesian> TrapezoidRTreePoint;
  typedef boost::geometry::model::box<TrapezoidRTreePoint> TrapezoidRTreeSegment;
  typedef boost::geometry::index::linear<3> TrapezoidRTreeParams;
  typedef std::pair<TrapezoidRTreeSegment, Trapezoid::ID> TrapezoidRTreeValue;
  typedef boost::geometry::index::rtree<TrapezoidRTreeValue, TrapezoidRTreeParams> TrapezoidRTree;

  /* Trapezoider object used to calculate and store all trapezoids of the room */
  Trapezoider trapezoider;
  /* Max and min opening per trapezoid */
  std::vector<TrapezoidOpening> openings;

  /* Trapezoids sorted by their max opening. Used for output sensitive calculation of single measurement queries */
  std::vector<Trapezoid::ID> sorted_by_max;
  /* Trapezoids in an interval tree, each interval is the min and max opening of a trapezoid. Used for output
   * sensitive calculation of two measurements queries
   */
  TrapezoidRTree rtree;

public:
  /* A result entry struct from a single measurement query. The struct represent the possible area in the 2D space a
   * sensor might be in the scene and measure the query distance at a specific edge. */
  struct Res1d {
    /* The edge the sensor might measure */
    std::pair<Point, Point> edge;
    /* The area representing the position in the 2D space a sensor might be and measure the query distance of measuring
     * the edge */
    Polygon pos;

    Res1d(const std::pair<Point, Point>& edge, const Polygon& pos) : edge(edge), pos(pos) {}
  };

  /* A result entry struct from a double measurement query. The struct represent the possible positions in the 2D space
   * a sensor might be in the scene and measure the first query distance d1 at a first edge and the second query
   * distance d2 at a second edge. */
  struct Res2d {
    /* The edge the sensor might measure d1 distance to */
    std::pair<Point, Point> edge1;
    /* The edge the sensor might measure d2 distance to */
    std::pair<Point, Point> edge2;
    /* A collection of segments representing the positions a sensor might be and measure d1,d2 at edge1,edge2
     * respectively */
    std::vector<Segment> pos;

    Res2d(const std::pair<Point, Point>& edge1, const std::pair<Point, Point>& edge2, const std::vector<Segment>& pos)
        : edge1(edge1), edge2(edge2), pos(pos) {}
  };

public:
  Locator() {}

  /**
   * @brief Init the locator with a polygon room
   *
   * @param scene polygon scene
   */
  void init(const Polygon_with_holes& scene);

  /**
   * @brief Calculate all the points in the room a sensor might be after it measure d at some wall
   *
   * @param d the single measurement value
   * @return collection of result entries, each representing possible positions a sensor might be and measure distance
   * d at a specific edge
   */
  std::vector<Res1d> query(const Kernel::FT& d) const;

  /**
   * @brief Calculate all the points in the room a sensor might be after it measured d1 in a single direction and d2
   * at the opposite direction.
   *
   * @param d1 the first measurement value
   * @param d2 the second measurement value
   * @return collection of result entries, each representing possible positions a sensor might be and measure d1,d2 at
   * some specific edges e1,e2
   */
  std::vector<Res2d> query(const Kernel::FT& d1, const Kernel::FT& d2) const;
};

} // namespace FDML

#endif
