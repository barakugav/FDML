cmake_minimum_required(VERSION 3.1...3.22)
project(Localization_with_surface_meshing)

set( CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/bin )
set( CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/lib )

find_package(CGAL REQUIRED COMPONENTS Core)
find_package(Boost REQUIRED COMPONENTS program_options)
find_package(TBB REQUIRED)


set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED TRUE)


# add_executable( meshing
#   ${CMAKE_CURRENT_SOURCE_DIR}/src/main.cpp
#   ${CMAKE_CURRENT_SOURCE_DIR}/src/read_input.cpp
#   ${CMAKE_CURRENT_SOURCE_DIR}/src/shoot_ray.cpp
#   ${CMAKE_CURRENT_SOURCE_DIR}/src/single_measurement.cpp
#   ${CMAKE_CURRENT_SOURCE_DIR}/src/meshing_options.cpp
#   ${CMAKE_CURRENT_SOURCE_DIR}/src/utils.cpp
#   ${CMAKE_CURRENT_SOURCE_DIR}/src/manifold_intersection.cpp
#   # ${CMAKE_CURRENT_SOURCE_DIR}/marching_cubes_33_c_library/libMC33.c
#   # ${CMAKE_CURRENT_SOURCE_DIR}/marching_cubes_33_c_library/marching_cubes_33.c
#   # ${CMAKE_CURRENT_SOURCE_DIR}/marching_cubes_33_c_library/MC33_util_grd.c
# )

add_executable( mia
  ${CMAKE_CURRENT_SOURCE_DIR}/src/mia.cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/src/read_input.cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/src/shoot_ray.cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/src/single_measurement.cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/src/meshing_options.cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/src/utils.cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/src/manifold_intersection.cpp
)


# add_to_cached_list(CGAL_EXECUTABLE_TARGETS)
# target_include_directories(meshing PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/include)
# target_link_libraries(meshing PRIVATE CGAL::CGAL)
# target_link_libraries(meshing PRIVATE ${Boost_LIBRARIES})
# target_link_libraries(meshing PRIVATE TBB::tbb TBB::tbbmalloc)

target_include_directories(mia PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/include)
target_link_libraries(mia PRIVATE CGAL::CGAL)
target_link_libraries(mia PRIVATE ${Boost_LIBRARIES})
target_link_libraries(mia PRIVATE TBB::tbb TBB::tbbmalloc)