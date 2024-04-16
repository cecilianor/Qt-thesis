# We had some issues with reaching path-length limits on Windows.
# This helps solve it.
if (WIN32)
  set(CMAKE_OBJECT_PATH_MAX 500)
endif ()

add_executable(tileloader_threaded_benchmark tileloader_threaded_benchmark.cpp)
target_link_libraries(tileloader_threaded_benchmark PUBLIC maplib Qt6::Test)
set(TEST_RESOURCES_ROOT "resources")
qt_add_resources(tileloader_threaded_benchmark "tileloader_threaded_benchmark_resources"
    PREFIX "/"
    BASE ${TEST_RESOURCES_ROOT}
    FILES
    ${TEST_RESOURCES_ROOT}/z2x0y0.mvt
    ${TEST_RESOURCES_ROOT}/z2x0y1.mvt
    ${TEST_RESOURCES_ROOT}/z2x0y2.mvt
    ${TEST_RESOURCES_ROOT}/z2x0y3.mvt
    ${TEST_RESOURCES_ROOT}/z2x1y0.mvt
    ${TEST_RESOURCES_ROOT}/z2x1y1.mvt
    ${TEST_RESOURCES_ROOT}/z2x1y2.mvt
    ${TEST_RESOURCES_ROOT}/z2x1y3.mvt
    ${TEST_RESOURCES_ROOT}/z2x2y0.mvt
    ${TEST_RESOURCES_ROOT}/z2x2y1.mvt
    ${TEST_RESOURCES_ROOT}/z2x2y2.mvt
    ${TEST_RESOURCES_ROOT}/z2x2y3.mvt
    ${TEST_RESOURCES_ROOT}/z2x3y0.mvt
    ${TEST_RESOURCES_ROOT}/z2x3y1.mvt
    ${TEST_RESOURCES_ROOT}/z2x3y2.mvt
    ${TEST_RESOURCES_ROOT}/z2x3y3.mvt
    ${TEST_RESOURCES_ROOT}/z2x0y0.png
    ${TEST_RESOURCES_ROOT}/z2x0y1.png
    ${TEST_RESOURCES_ROOT}/z2x0y2.png
    ${TEST_RESOURCES_ROOT}/z2x0y3.png
    ${TEST_RESOURCES_ROOT}/z2x1y0.png
    ${TEST_RESOURCES_ROOT}/z2x1y1.png
    ${TEST_RESOURCES_ROOT}/z2x1y2.png
    ${TEST_RESOURCES_ROOT}/z2x1y3.png
    ${TEST_RESOURCES_ROOT}/z2x2y0.png
    ${TEST_RESOURCES_ROOT}/z2x2y1.png
    ${TEST_RESOURCES_ROOT}/z2x2y2.png
    ${TEST_RESOURCES_ROOT}/z2x2y3.png
    ${TEST_RESOURCES_ROOT}/z2x3y0.png
    ${TEST_RESOURCES_ROOT}/z2x3y1.png
    ${TEST_RESOURCES_ROOT}/z2x3y2.png
    ${TEST_RESOURCES_ROOT}/z2x3y3.png
)
