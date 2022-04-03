#ifndef FDML_LOCATOR_HPP
#define FDML_LOCATOR_HPP

#include "fdml/config.hpp"
#include "fdml/defs.hpp"
#include "fdml/trapezoider.hpp"

#include <boost/geometry.hpp>

namespace FDML {

/**
 * @brief The locator class is used to preproccess a simple polygon room, and to query the possible positions a
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
  Locator() {}

  /**
   * @brief Init the locator with a simple polygon room
   *
   * @param scene simple polygon scene
   */
  void init(const Polygon& scene);

  /**
   * @brief Calculate all the points in the room a sensor might be after it measure d at some wall
   *
   * @param d the single measurement value
   * @param res collections of polygons representing all the possible points in the room a sensor might be at
   */
  void query(const Kernel::FT& d, std::vector<Polygon>& res) const;

  /**
   * @brief Calculate all the points in the room a sensor might be after it measured d1 in a single direction and d2
   * at the opposite direction.
   *
   * @param d1 the first measurement value
   * @param d2 the second measurement value
   * @param res collection of segments representing all the possible points in the room a sensor might be at
   */
  void query(const Kernel::FT& d1, const Kernel::FT& d2, std::vector<Segment>& res) const;
};

} // namespace FDML

#endif
