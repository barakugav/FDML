# Created by the script cgal_create_CMakeLists
# This is the CMake script for compiling a set of CGAL applications.

cmake_minimum_required(VERSION 3.1...3.15)

project( robo_local )


# CGAL and its components
find_package( CGAL QUIET COMPONENTS  )

if ( NOT CGAL_FOUND )
  message(STATUS "This project requires the CGAL library, and will not be compiled.")
  return()
endif()


# Boost and its components
find_package( Boost REQUIRED )

if ( NOT Boost_FOUND )
  message(STATUS "This project requires the Boost library, and will not be compiled.")
  return()
endif()


set(CPP_SRC "src/cpp/src")

# == robo_local_daemon ==

include_directories ( robo_local_daemon PRIVATE lib/boost_1_78_0/ )
include_directories ( robo_local_daemon PUBLIC src/cpp/include/ )
include_directories ( robo_local_daemon PRIVATE src/cpp/src/ )

add_executable( robo_local_daemon
  ${CPP_SRC}/trapezoider.cpp
  ${CPP_SRC}/localizator.cpp
  ${CPP_SRC}/json_utils.cpp
  ${CPP_SRC}/localizator_daemon.cpp
)

add_to_cached_list( CGAL_EXECUTABLE_TARGETS robo_local_daemon )

add_library(boost_json STATIC lib/boost_1_78_0/libs/json/src/src.cpp )
target_link_libraries(robo_local_daemon PRIVATE boost_system )
target_link_libraries(robo_local_daemon PRIVATE boost_json )
target_link_libraries(robo_local_daemon PRIVATE boost_program_options )
target_link_libraries(robo_local_daemon PRIVATE CGAL::CGAL )


# == test ==
# include_directories ( test PUBLIC src/cpp/include/ )
# add_executable( test
#   ${CPP_SRC}/trapezoider.cpp
#   ${CPP_SRC}/localizator.cpp
#   ${CPP_SRC}/test.cpp
# )
# add_to_cached_list( CGAL_EXECUTABLE_TARGETS test )
# target_link_libraries(test PRIVATE CGAL::CGAL )