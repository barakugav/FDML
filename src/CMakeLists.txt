set(FDML_MAJOR_VERSION 0)
set(FDML_MINOR_VERSION 1)
set(FDML_PATCH_VERSION 0)
set(FDML_VERSION ${FDML_MAJOR_VERSION}.${FDML_MINOR_VERSION}.${FDML_PATCH_VERSION})

# Use C++17 for this directory and its sub-directories.
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED TRUE)

if (CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
  add_compile_options(-fno-builtin)
endif()

# if (FDML_USE_STATIC_LIBS)
#   if(CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
#     set(CMAKE_FIND_LIBRARY_SUFFIXES ".a")
#   endif()
# endif()

# Set minimal required versions
set(FDML_PY_MIN_VERSION 3.8)
set(FDML_BOOST_MIN_VERSION 1.75.0)

# Find CGAL package
find_package(CGAL REQUIRED COMPONENTS Core)
if (NOT CGAL_FOUND)
  message(STATUS "This library cannot be compiled, as it requires CGAL.")
  return()
endif()
include(${CGAL_USE_FILE})

if (FDML_USE_STATIC_LIBS)
  set(Boost_USE_STATIC_LIBS ON)
  set(CMAKE_EXE_LINKER_FLAGS "-static-libgcc -static-libstdc++")
else()
  set(Boost_USE_STATIC_LIBS OFF)
endif()

# Add some compiler options

# Use dynamic link for all FDML libraries
add_definitions(-DFDML_ALL_DYN_LINK)

# Installation
if (FDML_WIN32_CMAKE_ON_CYGWIN)
  exec_program(cygpath ARGS -w "${CMAKE_INSTALL_PREFIX}"
               OUTPUT_VARIABLE CMAKE_INSTALL_PREFIX2)
  file(TO_CMAKE_PATH ${CMAKE_INSTALL_PREFIX2} CMAKE_INSTALL_PREFIX)
endif()

# Offer the user the choice of overriding the installation directories
set (FDML_INSTALL_LIB_DIR lib CACHE PATH
  "Installation directory for libraries")
set (FDML_INSTALL_BIN_DIR bin CACHE PATH
  "Installation directory for executables")
set (FDML_INSTALL_INC_DIR include CACHE PATH
  "Installation directory for header files")
if(WIN32 AND NOT CYGWIN)
  set(FDML_DEF_INSTALL_CMAKE_DIR cmake)
else()
  set(FDML_DEF_INSTALL_CMAKE_DIR lib/cmake/fdml)
endif()
set(FDML_INSTALL_CMAKE_DIR ${FDML_DEF_INSTALL_CMAKE_DIR} CACHE PATH
  "Installation directory for CMake files")
set(FDML_INSTALL_PYTHON_DIR
  "${FDML_INSTALL_LIB_DIR}/Python${Python3_VERSION_MAJOR}.${Python3_VERSION_MINOR}/dist-packages"
    CACHE PATH "Installation directory for Python files")

# Make relative paths absolute (needed later on)
foreach(p LIB BIN INC CMAKE PYTHON)
  set(var FDML_INSTALL_${p}_DIR)
  if(NOT IS_ABSOLUTE "${${var}}")
    set(${var} "${CMAKE_INSTALL_PREFIX}/${${var}}")
  endif()
endforeach()

# Add subdirectories
add_subdirectory(fdml)
if (FDML_WITH_PYBINDINGS)
  add_subdirectory(fdmlpy)
endif()
add_subdirectory(cmds)

# Add all targets to the build-tree export set
# todo: replace with the executable name
# set(FDML_TARGETS prep)
export(TARGETS ${FDML_TARGETS}
       FILE "${PROJECT_BINARY_DIR}/FDMLTargets.cmake")

# Export the package for use from the build-tree
# (this registers the build-tree with a global CMake-registry)
export(PACKAGE FDML)

# Create the FDMLConfig.cmake and FDMLConfigVersion files
file(RELATIVE_PATH REL_INCLUDE_DIR "${FDML_INSTALL_CMAKE_DIR}"
   "${FDML_INSTALL_INC_DIR}")
file(RELATIVE_PATH REL_BIN_DIR "${FDML_INSTALL_CMAKE_DIR}"
   "${FDML_INSTALL_BIN_DIR}")

# ... for the build tree
# set(CONF_INCLUDE_DIRS "${PROJECT_SOURCE_DIR}" "${PROJECT_BINARY_DIR}")
set(CONF_INCLUDE_DIRS
  "${FDML_FDML_INCLUDE_DIRS}"
  "${FDML_BOOST_INCLUDE_DIRS}")
configure_file(FDMLConfig.cmake.in
  "${PROJECT_BINARY_DIR}/FDMLConfig.cmake" @ONLY)

# ... for the install tree
set(CONF_INCLUDE_DIRS "\${FDML_CMAKE_DIR}/${REL_INCLUDE_DIR}")
configure_file(FDMLConfig.cmake.in
  "${PROJECT_BINARY_DIR}${CMAKE_FILES_DIRECTORY}/FDMLConfig.cmake" @ONLY)

# ... for both
configure_file(FDMLConfigVersion.cmake.in
  "${PROJECT_BINARY_DIR}/FDMLConfigVersion.cmake" @ONLY)

# Install the FDMLConfig.cmake and FDMLConfigVersion.cmake
install(FILES
  "${PROJECT_BINARY_DIR}${CMAKE_FILES_DIRECTORY}/FDMLConfig.cmake"
  "${PROJECT_BINARY_DIR}/FDMLConfigVersion.cmake"
  DESTINATION "${FDML_INSTALL_CMAKE_DIR}" COMPONENT dev)

# Install the export set for use with the install-tree
install(EXPORT FDMLTargets
  DESTINATION "${FDML_INSTALL_CMAKE_DIR}" COMPONENT dev)
