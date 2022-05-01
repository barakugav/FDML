// #include <boost/optional/optional_io.hpp>

#include <CGAL/Surface_mesh_default_triangulation_3.h>
#include <CGAL/Complex_2_in_triangulation_3.h>
#include <CGAL/make_surface_mesh.h>
#include <CGAL/Implicit_surface_3.h>
#include <CGAL/IO/facets_in_complex_2_to_triangle_mesh.h>
#include <CGAL/Surface_mesh.h>
#include <CGAL/squared_distance_2.h>

#include <CGAL/Exact_predicates_inexact_constructions_kernel.h>
#include <CGAL/Arr_non_caching_segment_traits_2.h>
#include <CGAL/Arr_trapezoid_ric_point_location.h>
#include <CGAL/Arrangement_2.h>
#include <CGAL/intersections.h>

#include <math.h>
#include <fstream>
#include <string>
#include <vector>

// default triangulation for Surface_mesher
typedef CGAL::Surface_mesh_default_triangulation_3 Tr;
typedef CGAL::Complex_2_in_triangulation_3<Tr> C2t3;
typedef Tr::Geom_traits GT;
typedef GT::Sphere_3 Sphere_3;
typedef GT::Point_3 Point_3;
typedef GT::FT FT;
typedef FT (*Function)(Point_3);
typedef CGAL::Implicit_surface_3<GT, Function> Surface_3;
typedef CGAL::Surface_mesh<Point_3> Surface_mesh;

typedef CGAL::Exact_predicates_inexact_constructions_kernel   Kernel;
typedef Kernel::FT                                          Number_type;
typedef CGAL::Arr_non_caching_segment_traits_2<Kernel>      Traits;
typedef Traits::Point_2                                     Point;
typedef Traits::X_monotone_curve_2                          Segment;
typedef CGAL::Arrangement_2<Traits>                         Arrangement;
typedef Arrangement::Vertex_handle                          Vertex_handle;
typedef Arrangement::Halfedge_handle                        Halfedge_handle;
typedef Arrangement::Face_handle                            Face_handle;
typedef CGAL::Arr_trapezoid_ric_point_location<Arrangement>	Trap_pl;

#define PI 3.14159265
#define INFTY 1000.0

FT sphere_function (Point_3 p) {
  const FT x2=p.x()*p.x(), y2=p.y()*p.y(), z2=p.z()*p.z();
  return x2+y2+z2-1;
}

void load_poly_to_arrangement(std::string& filename, Arrangement* arr)
{
	std::vector<Point> vertices;
	std::vector<Segment> segments;

	std::ifstream in(filename);
	while (in.good())
	{
		float x, y;
		in >> x >> y;
		
		Point pt(x, y);	
		vertices.push_back(pt);
	}

	for (int i = 0; i < vertices.size(); ++i)
	{
		Point p = vertices[i];
		Point q = vertices[(i+1) % vertices.size()];

		Segment s(p,q);
		segments.push_back(s);
	}

	CGAL::insert(*arr, segments.begin(), segments.end());
}

FT shoot_ray(Arrangement* arr, Trap_pl& pl, Point p, FT cos_theta, FT sin_theta)
{
	Kernel::Compute_squared_distance_2 squared_distance;
	FT dist(INFTY);

	// If we are out of bounds - return infinity
	auto obj = pl.locate(p);
	auto f = boost::get<Arrangement::Face_const_handle>(&obj);
	if (f && (*f)->is_unbounded())
		return dist;
	
	// Traverse all edges of the arrangement
	for (auto eit = arr->edges_begin(); eit != arr->edges_end(); ++eit)
	{
		Segment s = eit->curve();
		
		// If when rotating in (pi/2 - theta) we are in the corresponding x_range
		if (
			(
				s.source().x() * sin_theta - s.source().y() * cos_theta <= 
			 	p.x() * sin_theta -  p.y() * cos_theta
			) &&
			(
				p.x() * sin_theta -  p.y() * cos_theta <=
				s.target().x() * sin_theta - s.target().y() * cos_theta
			) ||
			(
				s.target().x() * sin_theta - s.target().y() * cos_theta <= 
			 	p.x() * sin_theta -  p.y() * cos_theta
			) &&
			(
				p.x() * sin_theta -  p.y() * cos_theta <=
				s.source().x() * sin_theta - s.source().y() * cos_theta
			)
		)
		{
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

int main() 
{
	// Load polygon
	Arrangement arr;
	std::string filename("in/scene01.poly");
	load_poly_to_arrangement(filename, &arr);
	Trap_pl pl(arr);
	
	FT d1 = 1/6.0;
	FT d2 = 2/6.0;
	FT alpha = PI / 2;

	// Generate the 2D-complex in 3D-Delaunay triangulation
	Tr tr1, tr2, tr3;
	C2t3 c2t3_1(tr1), c2t3_2(tr2), c2t3_3(tr3);

	// Generate the implicit surface and make surface mesh to c2t3
	Surface_3 surface1([&arr, &pl, d1](Point_3 p) {
			FT theta = p.z() * 2 * PI;
			if (theta < 0 || theta > 2 * PI) return INFTY;
			return shoot_ray(&arr, pl, Point(p.x(), p.y()), cos(theta), sin(theta)) - d1;
		}, Sphere_3(CGAL::ORIGIN, 2.)); 
	Surface_3 surface2([&arr, &pl, d2, alpha](Point_3 p) {
			FT theta = p.z() * 2 * PI + alpha;
			if (theta < 0 || theta > 2 * PI) return INFTY;
			return shoot_ray(&arr, pl, Point(p.x(), p.y()), cos(theta), sin(theta)) - d2;
		}, Sphere_3(CGAL::ORIGIN, 2.)); 
	Surface_3 surface3([&arr, &pl, d1, d2, alpha](Point_3 p) {
			FT theta = p.z() * 2 * PI + alpha;
			if (theta < 0 || theta > 2 * PI) return INFTY;
			
			FT theta_plus = p.z() * 2 * PI + alpha;
			if (theta_plus < 0 || theta_plus > 2 * PI) return INFTY;
			
			FT f_d1 = shoot_ray(&arr, pl, Point(p.x(), p.y()), cos(theta), sin(theta)) - d1;
			FT f_d2 = shoot_ray(&arr, pl, Point(p.x(), p.y()), cos(theta_plus), sin(theta_plus)) - d2;

			return (f_d1 * f_d1) + (f_d2 * f_d2) - 0.015;
		}, Sphere_3(CGAL::ORIGIN, 2.)); 

	CGAL::Surface_mesh_default_criteria_3<Tr> criteria(30., 0.04, 0.04);
	// CGAL::make_surface_mesh(c2t3_1, surface1, criteria, CGAL::Non_manifold_tag());
	// CGAL::make_surface_mesh(c2t3_2, surface2, criteria, CGAL::Non_manifold_tag());
	CGAL::make_surface_mesh(c2t3_3, surface3, criteria, CGAL::Non_manifold_tag());

	// Convert the c2t3 mesh to a mesh
	Surface_mesh sm1, sm2, sm3;
	// CGAL::facets_in_complex_2_to_triangle_mesh(c2t3_1, sm1);
	// CGAL::facets_in_complex_2_to_triangle_mesh(c2t3_2, sm2);
	CGAL::facets_in_complex_2_to_triangle_mesh(c2t3_3, sm3);

	// Export to off file
	// std::ofstream out1("out/scene01_1.off");
	// out1 << sm1 << std::endl;
	// std::ofstream out2("out/scene01_2.off");
	// out2 << sm2 << std::endl;
	std::ofstream out3("out/scene01_3.off");
	out3 << sm3 << std::endl;
}