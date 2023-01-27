set(FDML_LIB_DIR "${CMAKE_BINARY_DIR}/src/fdml")
set(FDMLPY_LIB_DIR "${CMAKE_BINARY_DIR}/src/fdmlpy")

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

# Add include directories
include_directories(include)
include_directories(../fdml/include)
include_directories(${FDML_LIB_DIR}/include)
include_directories(${Boost_INCLUDE_DIR})
include_directories(${PYTHON_INCLUDE_DIRS})

# Add defines
add_definitions(-DFDML_FDMLPY_SOURCE)
add_definitions(-DFDMLPY_LIB)

add_library(fdmlpy SHARED
  lib/fdmlpy.cpp
  lib/export_polygon_2.cpp
  lib/export_point_2.cpp
  lib/export_vector_2.cpp
  lib/export_polygon_with_holes_2.cpp)
target_link_libraries(fdmlpy PRIVATE
  fdml
  ${Boost_LIBRARIES}
  ${PYTHON_LIBRARIES}
  ${CMAKE_DL_LIBS}
  ${EXTRA_LIBS})
# don't prepend wrapper library name with lib
set_target_properties(fdmlpy PROPERTIES PREFIX "")
set_target_properties(fdmlpy PROPERTIES OUTPUT_NAME fdmlpy)
set (FDMLPY_LIBRARY_SUFFIX ".so")
if(WIN32)
  set (FDMLPY_LIBRARY_SUFFIX ".pyd")
  set_target_properties(fdmlpy PROPERTIES SUFFIX ".pyd")
elseif(APPLE)
  set (FDMLPY_LIBRARY_SUFFIX ".so")
  set_target_properties(fdmlpy PROPERTIES BUNDLE ON)
endif()
# Use a generator expression to get rid of per-configuration subdirectory
# appended by Multi-configuration generators (e.g Visual Studio)
# https://cmake.org/cmake/help/latest/prop_tgt/LIBRARY_OUTPUT_DIRECTORY.html
set_target_properties(fdmlpy PROPERTIES RUNTIME_OUTPUT_DIRECTORY ${FDMLPY_LIB_DIR}/$<0:>)
set (FDMLPY_TARGET_LINKER_FILE "fdmlpy${FDMLPY_LIBRARY_SUFFIX}")

################################################################################
# Stubs (Type Annotation)
################################################################################
# Copy stubs
set (FDMLPY_SRC_STUBS_DIR "${CMAKE_CURRENT_LIST_DIR}/stubs")
set (FDMLPY_STUBS_SRC_FILES fdmlpy.pyi)
foreach(name IN LISTS FDMLPY_STUBS_SRC_FILES)
  set (FDMLPY_STUBS ${FDMLPY_STUBS} ${FDMLPY_LIB_DIR}/${name})
  configure_file(${FDMLPY_SRC_STUBS_DIR}/${name} ${FDMLPY_LIB_DIR}/${name})
endforeach()

################################################################################
# Setup
################################################################################
# Copy setup
set (FDMLPY_SRC_SETUP_DIR "${CMAKE_CURRENT_LIST_DIR}")
configure_file(${FDMLPY_SRC_SETUP_DIR}/setup.py ${FDMLPY_LIB_DIR}/setup.py)
configure_file(${FDMLPY_SRC_SETUP_DIR}/README.rst ${FDMLPY_LIB_DIR}/README.rst)
set (FDMLPY_SETUP_FILES ${FDMLPY_LIB_DIR}/setup.py ${FDMLPY_LIB_DIR}/README.rst)

################################################################################
# Post build
################################################################################
# Setup application
set (FDMLPY_BUILD_OUTPUT_DIR "${FDMLPY_LIB_DIR}/dist")
set (FDMLPY_TAR ${FDMLPY_BUILD_OUTPUT_DIR}/fdmlpy-1.0.tar.gz)
set (FDMLPY_WHL ${FDMLPY_BUILD_OUTPUT_DIR}/fdmlpy-1.0-py3-none-any.whl)
set (FDMLPY_BUILD_OUTPUT ${FDMLPY_TAR} ${FDMLPY_WHL})
set (FDMLPY_BUILD_CMD ${Python3_EXECUTABLE})
set (FDMLPY_BUILD_ARGS -m build ${FDMLPY_LIB_DIR})
set (FDMLPY_BUILD_DEPENDS fdmlpy ${FDMLPY_STUBS} ${FDMLPY_SETUP_FILES})
add_custom_command(
  OUTPUT ${FDMLPY_BUILD_OUTPUT}
  COMMAND ${FDMLPY_BUILD_CMD}
  ARGS ${FDMLPY_BUILD_ARGS}
  DEPENDS ${FDMLPY_BUILD_DEPENDS})

add_custom_target(FDMLPY_BUILD ALL DEPENDS ${FDMLPY_BUILD_OUTPUT})

################################################################################
# Installation
################################################################################
set_property(TARGET fdmlpy PROPERTY INSTALL_RPATH_USE_LINK_PATH TRUE)

install(FILES ${FDMLPY_STUBS} DESTINATION ${FDML_INSTALL_PYTHON_DIR})
