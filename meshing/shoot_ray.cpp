#include "shoot_ray.h"

FT shoot_ray(Arrangement* arr, Trap_pl& pl, Point p, FT cos_theta, FT sin_theta) {
  Kernel::Compute_squared_distance_2 squared_distance;
  FT dist(INFTY);

  // If we are out of bounds - return infinity
  auto obj = pl.locate(p);
  auto f = boost::get<Arrangement::Face_const_handle>(&obj);
  if (f && (*f)->is_unbounded())
    return dist;

  // Traverse all edges of the arrangement
  for (auto eit = arr->edges_begin(); eit != arr->edges_end(); ++eit) {
    Segment s = eit->curve();

    // If when rotating in (pi/2 - theta) we are in the corresponding x_range
    if ((s.source().x() * sin_theta - s.source().y() * cos_theta <= p.x() * sin_theta - p.y() * cos_theta) &&
            (p.x() * sin_theta - p.y() * cos_theta <= s.target().x() * sin_theta - s.target().y() * cos_theta) ||
        (s.target().x() * sin_theta - s.target().y() * cos_theta <= p.x() * sin_theta - p.y() * cos_theta) &&
            (p.x() * sin_theta - p.y() * cos_theta <= s.source().x() * sin_theta - s.source().y() * cos_theta)) {
      // Compute the ray result in the rotated space
      FT p_x = s.source().x() * sin_theta - s.source().y() * cos_theta;
      FT p_y = s.source().x() * cos_theta + s.source().y() * sin_theta;
      FT q_x = s.target().x() * sin_theta - s.target().y() * cos_theta;
      FT q_y = s.target().x() * cos_theta + s.target().y() * sin_theta;
      FT v_x = p.x() * sin_theta - p.y() * cos_theta;
      FT v_y = p.x() * cos_theta + p.y() * sin_theta;

      FT m = (q_y - p_y) / (q_x - p_x);
      FT b = p_y - m * p_x;

      FT tmp = m * v_x + b - v_y;

      // If we are above and better than what is already found - update
      if (tmp > 0 && tmp < dist)
        dist = tmp;
    }
  }

  return dist;
}