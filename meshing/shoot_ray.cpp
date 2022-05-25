#include "shoot_ray.h"

FT shoot_ray(Arrangement* arr, Trap_pl& pl, Point p, FT cos_theta, FT sin_theta) {
  Kernel::Compute_squared_distance_2 squared_distance;
  FT dist(INFTY);

  // If we are out of bounds - return infinity
  /*
  auto obj = pl.locate(p);
  auto f = boost::get<Arrangement::Face_const_handle>(&obj);
  if (f && (*f)->is_unbounded())
    return dist;*/

  // Traverse all edges of the arrangement
  for (auto eit = arr->edges_begin(); eit != arr->edges_end(); ++eit) {
    Segment seg = eit->curve();
    Kernel::Ray_2 ray(p, Kernel::Vector_2(cos_theta, sin_theta));

    const auto isect = CGAL::intersection(seg, ray);
    if (isect) {
      if (const Point* q = boost::get<Point>(&*isect)) {
        FT tmp = std::sqrt((p.x() - q->x()) * (p.x() - q->x()) + (p.y() - q->y()) * (p.y() - q->y()));
        if (tmp < dist)
          dist = tmp;
      }
    }
  }

  return dist;
}