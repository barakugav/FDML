#include "defs.h"
#include <boost/geometry.hpp>
#include <trapezoider.h>

#ifndef __LOCALIZATOR_H__
#define __LOCALIZATOR_H__

class Localizator {
  private:
	typedef boost::geometry::model::point<Kernel::FT, 1, boost::geometry::cs::cartesian> TrapezoidRTreePoint;
	typedef boost::geometry::model::box<TrapezoidRTreePoint> TrapezoidRTreeSegment;
	typedef boost::geometry::index::linear<3> TrapezoidRTreeParams;
	typedef std::pair<TrapezoidRTreeSegment, TrapezoidID> TrapezoidRTreeValue;
	typedef boost::geometry::index::rtree<TrapezoidRTreeValue, TrapezoidRTreeParams> TrapezoidRTree;

	Trapezoider trapezoider;
	std::map<TrapezoidID, Trapezoid> trapezoids;
	std::vector<TrapezoidID> sorted_by_max;
	TrapezoidRTree rtree;

  public:
	Localizator() {}
	void init(const std::vector<Point> &points);
	void query(const Kernel::FT &d, std::vector<Polygon> &res);
	void query(const Kernel::FT &d1, const Kernel::FT &d2, Arrangement &res);
};

#endif
