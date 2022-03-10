cmake_minimum_required(VERSION 3.1...3.15)

project( squere_room )
set( EXAMPLE_TOP ${CMAKE_CURRENT_SOURCE_DIR} )
set( FDML_TOP ${CMAKE_CURRENT_SOURCE_DIR}/../.. )

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
  find_package( Boost REQUIRED COMPONENTS system )
endif ( WIN32 )
if ( NOT Boost_FOUND )
  message(STATUS "This project requires the Boost library, and will not be compiled.")
  return()
endif()

add_executable( squere_room
  ${FDML_TOP}/src/trapezoid.cpp
  ${FDML_TOP}/src/trapezoider.cpp
  ${FDML_TOP}/src/localizator.cpp
  ${EXAMPLE_TOP}/squere_room.cpp
)
include_directories ( squere_room ${FDML_TOP}/include/ )

add_to_cached_list( CGAL_EXECUTABLE_TARGETS fdml_daemon )
target_link_libraries(squere_room PRIVATE CGAL::CGAL )
if ( UNIX )
  target_link_libraries( squere_room PRIVATE boost_system )
endif ( UNIX )
if ( WIN32 )
  target_link_libraries( squere_room PRIVATE Boost::system )
endif ( WIN32 )
if ( MSVC )
  target_compile_options( squere_room PRIVATE /W3 /WX)
else ( MSVC )
  target_compile_options( squere_room PRIVATE -Wall -Wextra -Wpedantic -Werror)
endif ( MSVC )
