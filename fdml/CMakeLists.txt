# include(WriteCompilerDetectionHeader)
# include(CheckLibraryExists)

set(FDML_LIB_DIR "${CMAKE_BINARY_DIR}/fdml")

find_package(Python3 ${FDML_PY_MIN_VERSION} COMPONENTS Interpreter)
if (WIN32)
  set(FDML_GEN_PROPS_CMD ${Python_EXECUTABLE} ${FDML_GEN_PROPS})
else()
  set(FDML_GEN_PROPS_CMD ${FDML_GEN_PROPS})
endif()

################################################################################

# Generate version.hpp
configure_file(version.hpp.in include/fdml/version.hpp)

# The source files:
set(FDML_SRC_FILES ${FDML_SRC_FILES} src/json_utils.cpp)
set(FDML_SRC_FILES ${FDML_SRC_FILES} src/locator.cpp)
set(FDML_SRC_FILES ${FDML_SRC_FILES} src/locator_daemon.cpp)
set(FDML_SRC_FILES ${FDML_SRC_FILES} src/trapezoid.cpp)
set(FDML_SRC_FILES ${FDML_SRC_FILES} src/trapezoider.cpp)

set(FDML_HDR_FILES ${FDML_HDR_FILES} include/fdml/defs.hpp)
set(FDML_HDR_FILES ${FDML_HDR_FILES} include/fdml/locator_daemon.hpp)
set(FDML_HDR_FILES ${FDML_HDR_FILES} include/fdml/locator.hpp)
set(FDML_HDR_FILES ${FDML_HDR_FILES} include/fdml/retcode.hpp)
set(FDML_HDR_FILES ${FDML_HDR_FILES} include/fdml/trapezoider.hpp)
set(FDML_HDR_FILES ${FDML_HDR_FILES} include/fdml/trapezoid.hpp)

set(FDML_SRC_FILES ${FDML_SRC_FILES} ${FDML_HDR_FILES})
source_group(src FILES ${FDML_SRC_FILES} ${FDML_HDR_FILES})

###############################################################################

# Add library

if (BUILD_SHARED_LIBS)
  add_definitions(-DFDML_ALL_DYN_LINK)
  add_library(fdml SHARED ${FDML_SRC_FILES})
  if(CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
    set (CMAKE_SHARED_LINKER_FLAGS "-Wl,--no-undefined")
  endif()
else()
  if(CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
    set(CMAKE_CXX_FLAGS "-fPIC")
  endif()
  add_library(fdml STATIC ${FDML_SRC_FILES})
endif()

###############################################################################

# Find packages

if (FDML_USE_STATIC_LIBS)
  set_target_properties(fdml PROPERTIES LINK_SEARCH_END_STATIC 1)
endif()

find_package(Boost ${FDML_BOOST_MIN_VERSION} REQUIRED COMPONENTS system thread filesystem program_options json)

if (FDML_USE_STATIC_LIBS)
  set_target_properties(fdml PROPERTIES LINK_SEARCH_END_STATIC 1)
endif()

# Add libraries

CHECK_LIBRARY_EXISTS(m sin "" HAVE_LIB_M)
if (HAVE_LIB_M)
  set(EXTRA_LIBS ${EXTRA_LIBS} m)
endif (HAVE_LIB_M)

###############################################################################

# Add include directories
include_directories(include)
include_directories(${FDML_LIB_DIR}/include)
include_directories(${Boost_INCLUDE_DIR})

# Add defines
add_definitions(-DFDML_FDML_SOURCE)
add_definitions(-DFDML_LIB)

# if (WIN32)
#   add_definitions(/wd4251)
# else(WIN32)
#   add_definitions(-DGL_GLEXT_PROTOTYPES)
# endif(WIN32)

set_target_properties(fdml PROPERTIES
  VERSION "${CMAKE_PROJECT_VERSION}"
  SOVERSION "${CMAKE_PROJECT_VERSION_MAJOR}")

# Add links
# if (NOT WIN32)
#   set (NON_WIN32_LIBRARIES ${X11_LIBRARIES} ${X11_Xxf86vm_LIB})
#   if (APPLE)
#     set (NON_WIN32_LIBRARIES ${NON_WIN32_LIBRARIES} /opt/X11/lib/libGL.dylib)
#   endif(APPLE)
# endif(NOT WIN32)

target_link_libraries(fdml PRIVATE
  ${Boost_LIBRARIES}
  ${NON_WIN32_LIBRARIES}
  ${CMAKE_DL_LIBS}
  ${EXTRA_LIBS})

################################################################################
# Installation
################################################################################

set_property(TARGET fdml PROPERTY INSTALL_RPATH_USE_LINK_PATH TRUE)

set(FDML_FDML_INCLUDE_DIRS
  ${CMAKE_CURRENT_SOURCE_DIR}/include
  CACHE PATH "FDML source include directories")

set(FDML_PUBLIC_HEADERS "")
foreach (dir ${FDML_FDML_INCLUDE_DIRS})
  file(GLOB_RECURSE FDML_TMP ${dir}/fdml/*.hpp)
  list (APPEND FDML_PUBLIC_HEADERS ${FDML_TMP})
endforeach()

set_target_properties(fdml PROPERTIES PUBLIC_HEADER "${FDML_PUBLIC_HEADERS}")

set_target_properties(fdml PROPERTIES RUNTIME_OUTPUT_DIRECTORY_DEBUG ${CMAKE_BINARY_DIR}/debug/$<0:>)
set_target_properties(fdml PROPERTIES RUNTIME_OUTPUT_DIRECTORY_RELEASE ${CMAKE_BINARY_DIR}/release/$<0:>)

# Generate files suitable for preprocessor inclusion; these files contain
# macros to be used in source code

# write_compiler_detection_header(FILE climbingstats_compiler_detection.h
  # PREFIX ClimbingStats
  # OUTPUT_FILES_VAR support_files
  # OUTPUT_DIR compilers
  # COMPILERS GNU Clang MSVC
  # FEATURES cxx_generic_lambdas)

install(FILES
  ${support_files}
  DESTINATION include/compilers)

install(TARGETS fdml
  EXPORT FDMLTargets
  RUNTIME DESTINATION ${FDML_INSTALL_BIN_DIR} COMPONENT bin
  LIBRARY DESTINATION ${FDML_INSTALL_LIB_DIR} COMPONENT shlib
  PUBLIC_HEADER DESTINATION "${FDML_INSTALL_INC_DIR}/fdml" COMPONENT dev
  ARCHIVE DESTINATION ${FDML_INSTALL_LIB_DIR})
