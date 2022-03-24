#ifndef FDML_TRAPEZOIDER_HPP
#define FDML_TRAPEZOIDER_HPP

#include "fdml/defs.hpp"
#include "fdml/internal/closer_edge.hpp"
#include "fdml/trapezoid.hpp"

namespace FDML {

/**
 * @brief The Trapezoider class is an object used to calculate all trapezoids within a given polygon simple room. It
 * should be held in memory as long as the trapezoids are used, as the trapezoids reference to the stored arrangement.
 */
class Trapezoider {
private:
  typedef std::vector<Trapezoid> TrapezoidContainer;

public:
  typedef TrapezoidContainer::const_iterator TrapezoidIterator;

private:
  /* Struct containing all the data associated with a vertex during the parallel rotational sweep */
  struct VertexData {
    Trapezoid::ID top_left_trapezoid;
    Trapezoid::ID top_right_trapezoid;
    Trapezoid::ID bottom_left_trapezoid;
    Trapezoid::ID bottom_right_trapezoid;
    std::set<Halfedge, Closer_edge> ray_edges;
    VertexData() {}
    VertexData(Point &v, const Arrangement::Geometry_traits_2 *geom_traits);
  };

  /* Arrangement of the simple polygon room, built from the input points */
  Arrangement arr;
  /* The calculated trapezoids within the room, indexed by their id */
  TrapezoidContainer trapezoids;
  /* map containing the data associated with each vertex during the parallel rotational sweep */
  std::unordered_map<Vertex, VertexData> vertices_data;

public:
  Trapezoider() {}
  /**
   * @brief Calculates all the trapezoids that exists in the given room
   *
   * @param scene simple polygon scene
   */
  void calc_trapezoids(const Polygon &scene);

  TrapezoidIterator trapezoids_begin() const;
  TrapezoidIterator trapezoids_end() const;
  size_t number_of_trapezoids() const;
  TrapezoidIterator get_trapezoid(Trapezoid::ID id) const;

private:
  Trapezoid::ID create_trapezoid(const Halfedge &top_edge, const Halfedge &bottom_edge, const Vertex &left_vertex,
                                 const Vertex &right_vertex);
  void finalize_trapezoid(const Trapezoid &trapezoid);
  void init_trapezoids_with_regular_vertical_decomposition();
  void calc_trapezoids_with_rotational_sweep();
};

} // namespace FDML

#endif
