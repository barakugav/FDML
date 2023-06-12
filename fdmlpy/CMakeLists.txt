include(CMakePrintHelpers)

set(FDML_LIB_DIR "${CMAKE_BINARY_DIR}/fdml")
set(FDMLPY_LIB_DIR "${CMAKE_BINARY_DIR}/fdmlpy")
set(FDMLPY_PACKAGE_NAME "fdml")

find_package(Python ${FDML_PY_MIN_VERSION} COMPONENTS Interpreter Development)
set(FDML_PY_MAJOR_MINOR_VERSION "${Python_VERSION_MAJOR}${Python_VERSION_MINOR}")
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
  ${Python_LIBRARIES})
# don't prepend wrapper library name with lib
set_target_properties(fdmlpy PROPERTIES PREFIX "")
set_target_properties(fdmlpy PROPERTIES OUTPUT_NAME fdmlpy)
set(FDMLPY_LIBRARY_SUFFIX ${NB_SUFFIX})
set_target_properties(fdmlpy PROPERTIES LIBRARY_OUTPUT_DIRECTORY ${FDMLPY_LIB_DIR}/$<0:>)
set(FDMLPY_TARGET_LINKER_FILE "fdmlpy${FDMLPY_LIBRARY_SUFFIX}")
cmake_print_variables(FDMLPY_TARGET_LINKER_FILE)
set(FDML_TARGET_LINKER_FILE $<TARGET_LINKER_FILE:fdml>)
set(FDMLPY_BUILD_DEPENDS fdmlpy)

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
list(APPEND FDMLPY_BUILD_DEPENDS ${FDMLPY_STUBS})

################################################################################
# Setup
################################################################################
# Copy setup
set (FDMLPY_SRC_SETUP_DIR "${CMAKE_CURRENT_LIST_DIR}")

# The content of the source setup.py file contains a generator expression,
# which evaluates to the fdml built library full path. configure_file() does not
# evaluate generator expression. Instead, we read the content, substitute the
# normal expressions, and then use file(GENERATE...) to evaluate the generator
# expression.
# configure_file(${FDMLPY_SRC_SETUP_DIR}/setup.py ${FDMLPY_LIB_DIR}/setup.py)
file(READ ${FDMLPY_SRC_SETUP_DIR}/setup.py contents)
string(CONFIGURE "${contents}" contents @ONLY)
file(GENERATE OUTPUT ${FDMLPY_LIB_DIR}/setup.py CONTENT "${contents}")

configure_file(${FDMLPY_SRC_SETUP_DIR}/README.rst ${FDMLPY_LIB_DIR}/README.rst)
set (FDMLPY_SETUP_FILES ${FDMLPY_LIB_DIR}/setup.py ${FDMLPY_LIB_DIR}/README.rst)
list(APPEND FDMLPY_BUILD_DEPENDS ${FDMLPY_SETUP_FILES})

################################################################################
# Post build
################################################################################
# Add __init__.py
get_directory("${GMP_LIBRARIES}" GMP_LIBRARY_DIR)
get_directory("${MPFR_LIBRARIES}" MPFR_LIBRARY_DIR)
if(WIN32)
  set(FDMLPY_INIT_SRC "${CMAKE_CURRENT_LIST_DIR}/Windows/__init__.py")
else()
  set(FDMLPY_INIT_SRC "${CMAKE_CURRENT_LIST_DIR}/__init__.py")
endif()
set(FDMLPY_INIT_TRG "${FDMLPY_LIB_DIR}/__init__.py")
add_custom_command(
	OUTPUT ${FDMLPY_INIT_TRG}
        COMMAND "${CMAKE_COMMAND}"
        "-DFILE_IN=${FDMLPY_INIT_SRC}"
        "-DFILE_OUT=${FDMLPY_INIT_TRG}"
        "-DBoost_LIBRARY_DIRS=${Boost_LIBRARY_DIRS}"
        "-DGMP_LIBRARY_DIR=${GMP_LIBRARY_DIR}"
        "-DMPFR_LIBRARY_DIR=${MPFR_LIBRARY_DIR}"
        -P "${FDML_MODULE_DIR}/BuildTimeFile.cmake"
        DEPENDS ${FDMLPY_INIT_SRC}
	VERBATIM)

list(APPEND FDMLPY_BUILD_DEPENDS ${FDMLPY_INIT_TRG})

# Setup application
set (FDMLPY_BUILD_OUTPUT_DIR "${FDMLPY_LIB_DIR}/dist")

set (FDMLPY_PYTHON_TAG "cp${FDML_PY_MAJOR_MINOR_VERSION}")
set (FDMLPY_ABI_TAG "cp${FDML_PY_MAJOR_MINOR_VERSION}")
execute_process(COMMAND ${Python_EXECUTABLE} -c "import distutils;from distutils import util;print(distutils.util.get_platform().replace('-','_').replace('.','_'),end='')" OUTPUT_VARIABLE FDMLPY_PLATFORM_TAG)
set (FDMLPY_TAR ${FDMLPY_BUILD_OUTPUT_DIR}/${FDMLPY_PACKAGE_NAME}-${FDML_VERSION}.tar.gz)
set (FDMLPY_WHL ${FDMLPY_BUILD_OUTPUT_DIR}/${FDMLPY_PACKAGE_NAME}-${FDML_VERSION}-${FDMLPY_PYTHON_TAG}-${FDMLPY_ABI_TAG}-${FDMLPY_PLATFORM_TAG}.whl)
set (FDMLPY_BUILD_OUTPUT ${FDMLPY_TAR} ${FDMLPY_WHL})
set (FDMLPY_BUILD_CMD ${Python_EXECUTABLE})
set (FDMLPY_BUILD_ARGS -m build ${FDMLPY_LIB_DIR})
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
