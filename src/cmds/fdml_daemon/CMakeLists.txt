# Add source files
set(FDML_DAEMON_SOURCE_FILES ${FDML_DAEMON_SOURCE_FILES} fdml_daemon.cpp)

###############################################################################

add_executable(fdml_daemon ${FDML_DAEMON_SOURCE_FILES})

###############################################################################

# Find packages

if (FDML_USE_STATIC_LIBS)
  set_target_properties(fdml_daemon PROPERTIES LINK_SEARCH_START_STATIC 1)
endif()

################################################################################
######## Add Packages
# Find required Boost components
find_package(Boost ${FDML_BOOST_MIN_VERSION} REQUIRED COMPONENTS
  system program_options json)

if (FDML_USE_STATIC_LIBS)
  set_target_properties(fdml_daemon PROPERTIES LINK_SEARCH_END_STATIC 1)
endif()

################################################################################

# Add definitions

if (BUILD_SHARED_LIBS)
  add_definitions(-DFDML_ALL_DYN_LINK)
endif()

# Add defines
# if (NOT WIN32)
#   add_definitions(-DGL_GLEXT_PROTOTYPES)
# endif (NOT WIN32)

# Add include dirs

include_directories(../../libs/fdml/include)
include_directories(${CMAKE_BINARY_DIR}/src/libs/fdml/include)
include_directories(${Boost_INCLUDE_DIR})

# Link
target_link_directories(fdml_daemon PRIVATE ${Boost_LIBRARY_DIR})
if (FDML_USE_STATIC_LIBS)
  set(CMAKE_EXE_LINKER_FLAGS "-static")
endif()
target_link_libraries(fdml_daemon PRIVATE
  fdml
  ${Boost_LIBRARIES})

if (NOT FDML_USE_STATIC_LIBS)
  set_property(TARGET fdml_daemon PROPERTY INSTALL_RPATH_USE_LINK_PATH TRUE)
  set(CMAKE_SKIP_BUILD_RPATH TRUE)
endif()

install(TARGETS fdml_daemon
  EXPORT FDMLTargets
  RUNTIME DESTINATION ${FDML_INSTALL_BIN_DIR}
  LIBRARY DESTINATION ${FDML_INSTALL_LIB_DIR}
  ARCHIVE DESTINATION ${FDML_INSTALL_LIB_DIR})
