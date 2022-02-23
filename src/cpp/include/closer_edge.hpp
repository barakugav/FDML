#include "defs.h"
#include <CGAL/Visibility_2/visibility_utils.h>

#ifndef __CLOSER_EDGE_HPP__
#define __CLOSER_EDGE_HPP__

class Closer_edge : public CGAL::cpp98::binary_function<Halfedge, Halfedge, bool> {
	typedef Halfedge EH;
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

#endif
