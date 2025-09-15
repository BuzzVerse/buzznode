add_library(etl INTERFACE)

# Tell the build system that any target linking against 'etl' should
# get the 'lib/etl/include' directory in its include path.
target_include_directories(etl INTERFACE
  ${CMAKE_CURRENT_SOURCE_DIR}/etl/include
)