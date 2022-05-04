#include "read_input.h"

void load_poly_to_arrangement(std::string& filename, Arrangement* arr) {
  std::vector<Point> vertices;
  std::vector<Segment> segments;

  std::ifstream in(filename);
  while (in.good()) {
    float x, y;
    in >> x >> y;

    Point pt(x, y);
    vertices.push_back(pt);
  }

  for (int i = 0; i < vertices.size(); ++i) {
    Point p = vertices[i];
    Point q = vertices[(i + 1) % vertices.size()];

    Segment s(p, q);
    segments.push_back(s);
  }
  CGAL::insert(*arr, segments.begin(), segments.end());
}