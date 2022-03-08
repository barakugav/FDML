# Created by the script cgal_create_CMakeLists
# This is the CMake script for compiling a set of CGAL applications.

cmake_minimum_required(VERSION 3.1...3.15)

project( fdml )
set( PROJECT_TOP ${CMAKE_CURRENT_SOURCE_DIR} )
set( LIBS_TOP ${PROJECT_TOP}/lib )
set( CPP_SRC ${PROJECT_TOP}/src/cpp )

# CGAL and its components
find_package( CGAL QUIET COMPONENTS  )
if ( NOT CGAL_FOUND )
  message(STATUS "This project requires the CGAL library, and will not be compiled.")
  return()
endif()

# Boost and its components
if ( UNIX )
  find_package( Boost REQUIRED )
endif ( UNIX )
if ( WIN32 )
  find_package( Boost REQUIRED COMPONENTS system json program_options )
endif ( WIN32 )
if ( NOT Boost_FOUND )
  message(STATUS "This project requires the Boost library, and will not be compiled.")
  return()
endif()

# fdml_daemon
add_executable( fdml_daemon
  ${CPP_SRC}/src/trapezoid.cpp
  ${CPP_SRC}/src/trapezoider.cpp
  ${CPP_SRC}/src/localizator.cpp
  ${CPP_SRC}/src/json_utils.cpp
  ${CPP_SRC}/src/localizator_daemon.cpp
  ${CPP_SRC}/bin/fdml_daemon.cpp
)
include_directories ( fdml_daemon PUBLIC ${CPP_SRC}/include/ )
include_directories ( fdml_daemon PRIVATE ${CPP_SRC}/src/ )

add_to_cached_list( CGAL_EXECUTABLE_TARGETS fdml_daemon )
target_link_libraries(fdml_daemon PRIVATE CGAL::CGAL )
if ( UNIX )
  target_link_libraries( fdml_daemon PRIVATE boost_system )
  target_link_libraries( fdml_daemon PRIVATE boost_json )
  target_link_libraries( fdml_daemon PRIVATE boost_program_options )
  include_directories( fdml_daemon PRIVATE ${LIBS_TOP}/boost_1_78_0/ )
  add_library( boost_json STATIC ${LIBS_TOP}/boost_1_78_0/libs/json/src/src.cpp )
endif ( UNIX )
if ( WIN32 )
  target_link_libraries( fdml_daemon PRIVATE Boost::system )
  target_link_libraries( fdml_daemon PRIVATE Boost::json )
  target_link_libraries( fdml_daemon PRIVATE Boost::program_options )
endif ( WIN32 )
if ( MSVC )
  target_compile_options( fdml_daemon PRIVATE /W3 /WX)
else ( MSVC )
  target_compile_options( fdml_daemon PRIVATE -Wall -Wextra -Wpedantic -Werror)
endif ( MSVC )
