# include(WriteCompilerDetectionHeader)
# include(CheckLibraryExists)

set(FDML_LIB_DIR "${CMAKE_BINARY_DIR}/src/libs/fdml")

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
set(FDML_SRC_FILES ${FDML_SRC_FILES} lib/json_utils.cpp)
set(FDML_SRC_FILES ${FDML_SRC_FILES} lib/locator.cpp)
set(FDML_SRC_FILES ${FDML_SRC_FILES} lib/locator_daemon.cpp)
set(FDML_SRC_FILES ${FDML_SRC_FILES} lib/trapezoid.cpp)
set(FDML_SRC_FILES ${FDML_SRC_FILES} lib/trapezoider.cpp)

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
  add_library(fdml STATIC ${FDML_SRC_FILES})
endif()

###############################################################################

# Find packages

find_package(Boost ${FDML_BOOST_MIN_VERSION} REQUIRED COMPONENTS system thread filesystem program_options json)

if (FDML_USE_STATIC_LIBS)
  set_target_properties(FDML PROPERTIES LINK_SEARCH_END_STATIC 1)
endif()

# Add libraries

CHECK_LIBRARY_EXISTS(m sin "" HAVE_LIB_M)
if (HAVE_LIB_M)
  set(EXTRA_LIBS ${EXTRA_LIBS} m)
endif (HAVE_LIB_M)

###############################################################################
######## Add Python bindings
if (FDML_WITH_PYBINDINGS)
  #### Add Packages
  find_package(Python3 ${FDML_PY_MIN_VERSION} COMPONENTS Interpreter Development)
  set (FDML_PY_MAJOR_DOT_MINOR_VERSION
    "${Python3_VERSION_MAJOR}.${Python3_VERSION_MINOR}")
  set(FDML_PY_MAJOR_MINOR_VERSION
    "${Python3_VERSION_MAJOR}${Python3_VERSION_MINOR}")

  find_package(PythonLibs ${FDML_PY_MAJOR_DOT_MINOR_VERSION} EXACT REQUIRED)
  find_package(Boost ${FDML_BOOST_MIN_VERSION} REQUIRED COMPONENTS
    system thread program_options
    python${FDML_PY_MAJOR_MINOR_VERSION} numpy${FDML_PY_MAJOR_MINOR_VERSION})

  include_directories(${PYTHON_INCLUDE_DIRS})
  add_definitions(-DFDML_BUILD_PYBINDINGS)
  add_library(fdmlpy SHARED ${FDML_SOURCE_FILES}
    lib/fdml_pybindings.cpp
    lib/export_polygon.cpp
    lib/export_polygon_with_holes.cpp)
  target_link_libraries(fdmlpy PRIVATE
    fdml
    ${Boost_LIBRARIES}
    ${PYTHON_LIBRARIES}
    ${CMAKE_DL_LIBS}
    ${EXTRA_LIBS} boost_python38)
  # don't prepend wrapper library name with lib
  set_target_properties(fdmlpy PROPERTIES PREFIX "")
  set_target_properties(fdmlpy PROPERTIES OUTPUT_NAME fdml)
  set (FDMLPY_LIBRARY_SUFFIX ".so")
  if(WIN32)
    set (FDMLPY_LIBRARY_SUFFIX ".pyd")
    set_target_properties(fdmlpy PROPERTIES SUFFIX ".pyd")
  elseif(APPLE)
    set (FDMLPY_LIBRARY_SUFFIX ".so")
    set_target_properties(fdmlpy PROPERTIES BUNDLE ON)
  endif()
  # Use a generator expression to get rid of per-configuration subdirectory appended by
  # Multi-configuration generators (e.g Visual Studio)
  # https://cmake.org/cmake/help/latest/prop_tgt/LIBRARY_OUTPUT_DIRECTORY.html
  set_target_properties(fdmlpy PROPERTIES RUNTIME_OUTPUT_DIRECTORY ${FDML_LIB_DIR}/$<0:>)
  set (FDMLPY_TARGET_LINKER_FILE "fdml${FDMLPY_LIBRARY_SUFFIX}")
endif()

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
# Stubs (Type Annotation)
################################################################################
# Copy stubs
if (FDML_WITH_PYBINDINGS)
  set (FDML_SRC_STUBS_DIR "${CMAKE_CURRENT_LIST_DIR}/stubs")
  set (FDML_STUBS_SRC_FILES fdml.pyi)
  foreach(name IN LISTS FDML_STUBS_SRC_FILES)
    set (FDML_STUBS ${FDML_STUBS} ${FDML_LIB_DIR}/${name})
    configure_file(${FDML_SRC_STUBS_DIR}/${name} ${FDML_LIB_DIR}/${name})
  endforeach()
endif()

################################################################################
# Setup
################################################################################
# Copy setup
if (FDML_WITH_PYBINDINGS)
  set (FDML_SRC_SETUP_DIR "${CMAKE_CURRENT_LIST_DIR}")
  configure_file(${FDML_SRC_SETUP_DIR}/setup.py ${FDML_LIB_DIR}/setup.py)
  configure_file(${FDML_SRC_SETUP_DIR}/README.rst ${FDML_LIB_DIR}/README.rst)
  set (FDML_SETUP_FILES ${FDML_LIB_DIR}/setup.py ${FDML_LIB_DIR}/README.rst)
endif()

################################################################################
# Post build
################################################################################
# Setup application
if (FDML_WITH_PYBINDINGS)
  set (FDML_BUILD_OUTPUT_DIR "${FDML_LIB_DIR}/dist")
  set (FDML_TAR ${FDML_BUILD_OUTPUT_DIR}/FDML-1.0.tar.gz)
  set (FDML_WHL ${FDML_BUILD_OUTPUT_DIR}/FDML-1.0-py3-none-any.whl)
  set (FDML_BUILD_OUTPUT ${FDML_TAR} ${FDML_WHL})
  set (FDML_BUILD_CMD ${Python3_EXECUTABLE})
  set (FDML_BUILD_ARGS -m build ${FDML_LIB_DIR})
  set (FDML_BUILD_DEPENDS fdmlpy ${FDML_STUBS} ${FDML_SETUP_FILES})
  add_custom_command(
    OUTPUT ${FDML_BUILD_OUTPUT}
    COMMAND ${FDML_BUILD_CMD}
    ARGS ${FDML_BUILD_ARGS}
    DEPENDS ${FDML_BUILD_DEPENDS})

  add_custom_target(FDML_BUILD ALL DEPENDS ${FDML_BUILD_OUTPUT})
endif()

################################################################################
# Installation
################################################################################
set_property(TARGET fdml PROPERTY INSTALL_RPATH_USE_LINK_PATH TRUE)
if (FDML_WITH_PYBINDINGS)
  set_property(TARGET fdmlpy PROPERTY INSTALL_RPATH_USE_LINK_PATH TRUE)
endif()

set(FDML_FDML_INCLUDE_DIRS
  ${CMAKE_CURRENT_SOURCE_DIR}/include
  CACHE PATH "FDML source include directories")

set(FDML_PUBLIC_HEADERS "")
foreach (dir ${FDML_FDML_INCLUDE_DIRS})
  file(GLOB_RECURSE FDML_TMP ${dir}/fdml/*.hpp)
  list (APPEND FDML_PUBLIC_HEADERS ${FDML_TMP})
endforeach()

set_target_properties(fdml PROPERTIES PUBLIC_HEADER "${FDML_PUBLIC_HEADERS}")

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

if (FDML_WITH_PYBINDINGS)
install(FILES ${FDML_STUBS} DESTINATION ${FDML_INSTALL_PYTHON_DIR})
endif()
