cmake_minimum_required(VERSION 3.1...3.22)
project(Localization_with_surface_meshing)

find_package(CGAL REQUIRED COMPONENTS Core)
find_package(Boost REQUIRED COMPONENTS program_options)

add_executable( meshing
  ${CMAKE_CURRENT_SOURCE_DIR}/meshing.cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/read_input.cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/shoot_ray.cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/single_measurement.cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/meshing_options.cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/utils.cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/manifold_intersection.cpp
)


add_to_cached_list(CGAL_EXECUTABLE_TARGETS)
target_link_libraries(meshing PRIVATE CGAL::CGAL)
target_link_libraries(meshing PRIVATE ${Boost_LIBRARIES})


# create a target per cppfile
# file(
#   GLOB cppfiles
#   RELATIVE ${CMAKE_CURRENT_SOURCE_DIR}
#   ${CMAKE_CURRENT_SOURCE_DIR}/*.cpp)
# foreach(cppfile ${cppfiles})
#   create_single_source_cgal_program("${cppfile}")
#   target_link_libraries(${Boost_LIBRARIES})
# endforeach()
