#include "manifold_intersection.h"

void manifold_intersection(Surface_mesh& M_1, Surface_mesh& M_2, Surface_mesh& M_isect, Arrangement& arr, Trap_pl& pl,
                           DeltaCube initial_cube, FT delta, FT epsilon) {
    std::vector<DeltaCube> Q; // A queue of pending cubes to split;
    Q.push_back(initial_cube);

    Tree tree_1(faces(M_1).first, faces(M_1).second, M_1);
    Tree tree_2(faces(M_2).first, faces(M_2).second, M_2);

    while (Q.size()) {
        DeltaCube curr_cube = Q.back();
        Q.pop_back();

        if (tree_1.any_intersected_primitive(curr_cube.to_bbox_3()) &&
            tree_2.any_intersected_primitive(curr_cube.to_bbox_3())) { // Intersect stuff
            if (curr_cube.size() > delta)
                curr_cube.split(Q);
            else {
                Point_3 midpoint = curr_cube.midpoint();
                // bool z_okay = 0 <= midpoint.z() && midpoint.z() <= 1 / (2 * M_PI);

                // if (!z_okay)
                //     continue;

                FT dist(INFTY);
                Point p(midpoint.x(), midpoint.y());
                for (auto eit = arr.edges_begin(); eit != arr.edges_end(); ++eit) {
                    Segment seg = eit->curve();
                    FT tmp = std::sqrt(CGAL::squared_distance(seg, p));
                    if (tmp < dist)
                        dist = tmp;
                }
                if (dist < 4 * delta)
                    continue;

                auto obj1 = pl.locate(Point(curr_cube.bottom_left.x(), curr_cube.bottom_left.y()));
                auto obj2 = pl.locate(Point(curr_cube.top_right.x(), curr_cube.top_right.y()));
                auto obj3 = pl.locate(Point(curr_cube.bottom_right().x(), curr_cube.bottom_right().y()));
                auto obj4 = pl.locate(Point(curr_cube.top_left().x(), curr_cube.top_left().y()));
                auto f1 = boost::get<Arrangement::Face_const_handle>(&obj1);
                auto f2 = boost::get<Arrangement::Face_const_handle>(&obj2);
                auto f3 = boost::get<Arrangement::Face_const_handle>(&obj3);
                auto f4 = boost::get<Arrangement::Face_const_handle>(&obj4);
                bool bottom_left_inside_room = !f1 || !(*f1)->is_unbounded();
                bool top_right_inside_room = !f2 || !(*f2)->is_unbounded();
                bool bottom_right_inside_room = !f3 || !(*f3)->is_unbounded();
                bool top_left_inside_room = !f4 || !(*f4)->is_unbounded();
                if (!bottom_left_inside_room || !top_right_inside_room || !bottom_right_inside_room ||
                    !top_left_inside_room)
                    continue;

                curr_cube.to_surface_mesh(M_isect);
            }
        }
    }
}

DeltaCube::DeltaCube(Point_3 bottom_left, Point_3 top_right) {
    this->bottom_left = bottom_left;
    this->top_right = top_right;
}

FT DeltaCube::size() {
    return top_right.x() - bottom_left.x();
}

Bbox_3 DeltaCube::to_bbox_3() {
    return Bbox_3(bottom_left.x(), bottom_left.y(), bottom_left.z(), top_right.x(), top_right.y(), top_right.z());
}

void DeltaCube::to_surface_mesh(Surface_mesh& sm) {
    // Bottom Face
    Vertex_descriptor p1 = sm.add_vertex(Point_3(bottom_left.x(), bottom_left.y(), bottom_left.z()));
    Vertex_descriptor p2 = sm.add_vertex(Point_3(top_right.x(), bottom_left.y(), bottom_left.z()));
    Vertex_descriptor p3 = sm.add_vertex(Point_3(top_right.x(), top_right.y(), bottom_left.z()));
    Vertex_descriptor p4 = sm.add_vertex(Point_3(bottom_left.x(), top_right.y(), bottom_left.z()));
    sm.add_face(p1, p2, p3, p4);

    // Top Face
    p1 = sm.add_vertex(Point_3(bottom_left.x(), bottom_left.y(), top_right.z()));
    p2 = sm.add_vertex(Point_3(top_right.x(), bottom_left.y(), top_right.z()));
    p3 = sm.add_vertex(Point_3(top_right.x(), top_right.y(), top_right.z()));
    p4 = sm.add_vertex(Point_3(bottom_left.x(), top_right.y(), top_right.z()));
    sm.add_face(p1, p2, p3, p4);

    // Front Face
    p1 = sm.add_vertex(Point_3(bottom_left.x(), bottom_left.y(), bottom_left.z()));
    p2 = sm.add_vertex(Point_3(top_right.x(), bottom_left.y(), bottom_left.z()));
    p3 = sm.add_vertex(Point_3(top_right.x(), bottom_left.y(), top_right.z()));
    p4 = sm.add_vertex(Point_3(bottom_left.x(), bottom_left.y(), top_right.z()));
    sm.add_face(p1, p2, p3, p4);

    // Back Face
    p1 = sm.add_vertex(Point_3(bottom_left.x(), top_right.y(), bottom_left.z()));
    p2 = sm.add_vertex(Point_3(top_right.x(), top_right.y(), bottom_left.z()));
    p3 = sm.add_vertex(Point_3(top_right.x(), top_right.y(), top_right.z()));
    p4 = sm.add_vertex(Point_3(bottom_left.x(), top_right.y(), top_right.z()));
    sm.add_face(p1, p2, p3, p4);

    // Right Face
    p1 = sm.add_vertex(Point_3(top_right.x(), bottom_left.y(), bottom_left.z()));
    p2 = sm.add_vertex(Point_3(top_right.x(), top_right.y(), bottom_left.z()));
    p3 = sm.add_vertex(Point_3(top_right.x(), top_right.y(), top_right.z()));
    p4 = sm.add_vertex(Point_3(top_right.x(), bottom_left.y(), top_right.z()));
    sm.add_face(p1, p2, p3, p4);

    // Left Face
    p1 = sm.add_vertex(Point_3(bottom_left.x(), bottom_left.y(), bottom_left.z()));
    p2 = sm.add_vertex(Point_3(bottom_left.x(), top_right.y(), bottom_left.z()));
    p3 = sm.add_vertex(Point_3(bottom_left.x(), top_right.y(), top_right.z()));
    p4 = sm.add_vertex(Point_3(bottom_left.x(), bottom_left.y(), top_right.z()));
    sm.add_face(p1, p2, p3, p4);
}

void DeltaCube::split(std::vector<DeltaCube>& list) {
    Point_3 center = this->midpoint();

    list.push_back(DeltaCube(Point_3(bottom_left.x(), bottom_left.y(), bottom_left.z()),
                             Point_3(center.x(), center.y(), center.z())));
    list.push_back(DeltaCube(Point_3(center.x(), bottom_left.y(), bottom_left.z()),
                             Point_3(top_right.x(), center.y(), center.z())));
    list.push_back(DeltaCube(Point_3(bottom_left.x(), center.y(), bottom_left.z()),
                             Point_3(center.x(), top_right.y(), center.z())));
    list.push_back(
        DeltaCube(Point_3(center.x(), center.y(), bottom_left.z()), Point_3(top_right.x(), top_right.y(), center.z())));
    list.push_back(DeltaCube(Point_3(bottom_left.x(), bottom_left.y(), center.z()),
                             Point_3(center.x(), center.y(), top_right.z())));
    list.push_back(
        DeltaCube(Point_3(center.x(), bottom_left.y(), center.z()), Point_3(top_right.x(), center.y(), top_right.z())));
    list.push_back(
        DeltaCube(Point_3(bottom_left.x(), center.y(), center.z()), Point_3(center.x(), top_right.y(), top_right.z())));
    list.push_back(
        DeltaCube(Point_3(center.x(), center.y(), center.z()), Point_3(top_right.x(), top_right.y(), top_right.z())));
}

Point_3 DeltaCube::midpoint() const {
    return Point_3((bottom_left.x() + top_right.x()) / FT(2), (bottom_left.y() + top_right.y()) / FT(2),
                   (bottom_left.z() + top_right.z()) / FT(2));
}

Point_3 DeltaCube::bottom_right() const {
    return Point_3(top_right.x(), bottom_left.y(), bottom_left.z());
}
Point_3 DeltaCube::top_left() const {
    return Point_3(bottom_left.x(), top_right.y(), top_right.z());
}