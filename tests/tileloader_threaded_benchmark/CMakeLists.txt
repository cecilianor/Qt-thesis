# We had some issues with reaching path-length limits on Windows.
# This helps solve it.
if (WIN32)
  set(CMAKE_OBJECT_PATH_MAX 500)
endif ()

add_executable(tileloader_threaded_benchmark tileloader_threaded_benchmark.cpp)
target_link_libraries(tileloader_threaded_benchmark PUBLIC maplib Qt6::Test)
set(TEST_RESOURCES_ROOT "resources")

file(GLOB_RECURSE tile_files "${TEST_RESOURCES_ROOT}/*.mvt")

qt_add_resources(tileloader_threaded_benchmark "tileloader_threaded_benchmark_resources"
    PREFIX "/"
    BASE ${TEST_RESOURCES_ROOT}
    FILES
    ${tile_files}
)
