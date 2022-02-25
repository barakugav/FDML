#include "defs.h"
#include <boost/geometry.hpp>
#include <trapezoider.h>

#ifndef __LOCALIZATOR_H__
#define __LOCALIZATOR_H__

/**
 * @brief The localizator class is used to preproccess a simple polygon room, and to query the possible positions a
 * sensor might be in the room given one or two measeraments.
 *
 */
class Localizator {
  private:
	typedef boost::geometry::model::point<Kernel::FT, 1, boost::geometry::cs::cartesian> TrapezoidRTreePoint;
	typedef boost::geometry::model::box<TrapezoidRTreePoint> TrapezoidRTreeSegment;
	typedef boost::geometry::index::linear<3> TrapezoidRTreeParams;
	typedef std::pair<TrapezoidRTreeSegment, Trapezoid::ID> TrapezoidRTreeValue;
	typedef boost::geometry::index::rtree<TrapezoidRTreeValue, TrapezoidRTreeParams> TrapezoidRTree;

	/* Trapezoider object used to calculate all trapezoids of the room */
	Trapezoider trapezoider;
	/* Map containing all the trapezoids of the room */
	std::map<Trapezoid::ID, Trapezoid> trapezoids;
	/* Trapezoids sorted by their max opening. Used for output sensitive calculation of single measurement queries */
	std::vector<Trapezoid::ID> sorted_by_max;
	/* Trapezoids in an interval tree, each interval is the min and max opening of a trapezoid. Used for output
	 * sensitive calculation of two measurements queries */
	TrapezoidRTree rtree;

  public:
	Localizator() {}

	/**
	 * @brief Init the localizator with a simple polygon room
	 *
	 * @param points list of points representing a polygon room
	 */
	void init(const std::vector<Point> &points);

	/**
	 * @brief Calculate all the points in the room a sensor might be given a single measurement
	 *
	 * @param d the single measurement value
	 * @param res collections of polygons representing all the possible points in the room a sensor might be in
	 */
	void query(const Kernel::FT &d, std::vector<Polygon> &res);

	// TODO change output type
	void query(const Kernel::FT &d1, const Kernel::FT &d2, Arrangement &res);
};

#endif
