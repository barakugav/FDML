cmake_minimum_required(VERSION 3.1...3.15)

project( fdml )
set( TOOL_TOP ${CMAKE_CURRENT_SOURCE_DIR} )
set( FDML_TOP ${CMAKE_CURRENT_SOURCE_DIR}/../../ )
set( LIBS_TOP ${FDML_TOP}/lib )

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

add_executable( fdml_daemon
  ${FDML_TOP}/src/trapezoid.cpp
  ${FDML_TOP}/src/trapezoider.cpp
  ${FDML_TOP}/src/localizator.cpp
  ${FDML_TOP}/src/json_utils.cpp
  ${FDML_TOP}/src/localizator_daemon.cpp
  ${TOOL_TOP}/fdml_daemon.cpp
)
include_directories ( fdml_daemon PUBLIC ${FDML_TOP}/include/ )

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
