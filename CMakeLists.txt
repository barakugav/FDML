cmake_minimum_required(VERSION 3.14...3.22)
project(FDML VERSION "1.0.0" LANGUAGES CXX)

# General options
option(BUILD_SHARED_LIBS "Build shared libs instead of static libs" ON)
option(FDML_WITH_PYBINDINGS "With python bindings" OFF)
option(FDML_USE_STATIC_LIBS "Link with static libraries" OFF)

# Options
set(FDML_PROJECT_AUTHORS "Barak Ugav")
set(FDML_PROJECT_EMAILS "barakugav@gmail.com")
set(FDML_PROJECT_URL "https://github.com/barakugav/FDML")
set(FDML_MODULES_REL_DIR cmake/modules)
set(FDML_MODULE_DIR ${CMAKE_SOURCE_DIR}/${FDML_MODULES_REL_DIR})
set(CMAKE_MODULE_PATH ${FDML_MODULE_DIR})
include(FDML_Macros)
include(FDML_Common)

set(default_build_type "Release")
if(NOT CMAKE_BUILD_TYPE AND NOT CMAKE_CONFIGURATION_TYPES)
  message(STATUS "Setting build type to '${default_build_type}' as none was specified.")
  set(CMAKE_BUILD_TYPE "${default_build_type}" CACHE
    STRING "Choose the type of build." FORCE)
  # Set the possible values of build type for cmake-gui
  set_property(CACHE CMAKE_BUILD_TYPE PROPERTY STRINGS
    "Debug" "Release" "MinSizeRel" "RelWithDebInfo")
endif()

message(STATUS "VERSION=${CMAKE_PROJECT_VERSION}")
message(STATUS "SOVERSION=${CMAKE_PROJECT_VERSION_MAJOR}")

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
