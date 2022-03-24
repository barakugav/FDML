cmake_minimum_required(VERSION 3.14)
project(FDML)
if(POLICY CLS0011)
  cmake_policy(SET CLS0011 NEW)
endif()

# Define options
option(BUILD_SHARED_LIBS "Build shared libs instead of static libs" ON)
option(FDML_TRACE "Trace" OFF)
option(FDML_WITH_PYBINDINGS "With python bindings" OFF)
option(FDML_WITH_CLASSIFY "With python classify" OFF)
option(FDML_WITH_NN "With python nearest neihgbor" OFF)
option(FDML_USE_STATIC_LIBS "Link with static libraries" OFF)
option(FDML_WITH_VISUAL "Draw the scene" ON)
# SONAME, SOVERSION
#
# The rule is that each new release or bug fix release should increse the soversion.
#
# SOVERSION is $(SONAME_VERSION).$(SOVERSION_MINOR).$(SOVERSION_RELEASE)
#
# - If the binary interface of libraries do not change from previous release
#   (example: most bug fix releases), increase SOVERSION_RELEASE (this third number).
# - If the binary interface is changed, but remains compatible with
#   previous release (example: only addition of new functions), then increase
#   SOVERSION_MINOR (second number) and set SOVERSION_RELEASE to 0.
# - If the binary interface is changed in an incompatible way to previous
#   release, then increase the SONAME_VERSION, and set the two other
#   numbers to 0.
#
# SOVERSION history:
#   FDML-0.1  : 0.1.0
#
set(FDML_SONAME_VERSION "0")
set(FDML_SOVERSION "0.1.0" )
message(STATUS "FDML_SONAME_VERSION=${FDML_SONAME_VERSION}")
message(STATUS "FDML_SOVERSION=${FDML_SOVERSION}")

# General
set(FDML_MODULES_REL_DIR cmake/modules)
set(CMAKE_MODULE_PATH ${CMAKE_SOURCE_DIR}/${FDML_MODULES_REL_DIR})
include(FDML_Macros)
include(FDML_Common)

# Add src subdirectory

add_subdirectory(src)

include (InstallRequiredSystemLibraries)
#set (CPACK_RESOURCE_FILE_LICENSE
#     "${CMAKE_CURRENT_SOURCE_DIR}/License.txt")
set(CPACK_GENERATOR TGZ)
set(CPACK_PACKAGE_VENDOR "Barak")
set(CPACK_PACKAGE_VERSION_MAJOR "0")
set(CPACK_PACKAGE_VERSION_MINOR "1")
set(CPACK_PACKAGE_VERSION_PATCH "0")
include(CPack)
