# Add subdirectories
add_subdirectory(fdml)
if (FDML_WITH_PYBINDINGS)
  add_subdirectory(fdmlpy)
endif ()
