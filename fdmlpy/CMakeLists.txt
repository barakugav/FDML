set(FDML_LIB_DIR "${CMAKE_BINARY_DIR}/fdml")
set(FDMLPY_LIB_DIR "${CMAKE_BINARY_DIR}/fdmlpy")

find_package(Python ${FDML_PY_MIN_VERSION} COMPONENTS Interpreter Development)
find_package(nanobind CONFIG REQUIRED)

#### Add Packages
find_package(Boost ${FDML_BOOST_MIN_VERSION} REQUIRED COMPONENTS
  system thread program_options)

# Add include directories
include_directories(include)
include_directories(../fdml/include)
include_directories(${FDML_LIB_DIR}/include)
include_directories(${Boost_INCLUDE_DIR})
include_directories(${PYTHON_INCLUDE_DIRS})

# Add defines
add_definitions(-DFDML_FDMLPY_SOURCE)
add_definitions(-DFDMLPY_LIB)

nanobind_add_module(fdmlpy
  src/fdmlpy.cpp
  src/export_gmp.cpp
  src/export_polygon_2.cpp
  src/export_polygon_with_holes_2.cpp)
target_link_libraries(fdmlpy PRIVATE
  fdml
  ${Boost_LIBRARIES}
  ${PYTHON_LIBRARIES})
# don't prepend wrapper library name with lib
set_target_properties(fdmlpy PROPERTIES PREFIX "")
set_target_properties(fdmlpy PROPERTIES OUTPUT_NAME fdmlpy)
set(FDMLPY_LIBRARY_SUFFIX ${NB_SUFFIX})
set_target_properties(fdmlpy PROPERTIES RUNTIME_OUTPUT_DIRECTORY ${FDMLPY_LIB_DIR}/$<0:>)
set(FDMLPY_TARGET_LINKER_FILE "fdmlpy${FDMLPY_LIBRARY_SUFFIX}")

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
set (FDMLPY_BUILD_CMD ${Python_EXECUTABLE})
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
