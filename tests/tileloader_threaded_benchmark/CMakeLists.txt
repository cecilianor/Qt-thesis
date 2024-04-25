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
    ${TEST_RESOURCES_ROOT}/z3x0y0.mvt
    ${TEST_RESOURCES_ROOT}/z3x0y1.mvt
    ${TEST_RESOURCES_ROOT}/z3x0y2.mvt
    ${TEST_RESOURCES_ROOT}/z3x0y3.mvt
    ${TEST_RESOURCES_ROOT}/z3x0y4.mvt
    ${TEST_RESOURCES_ROOT}/z3x0y5.mvt
    ${TEST_RESOURCES_ROOT}/z3x0y6.mvt
    ${TEST_RESOURCES_ROOT}/z3x0y7.mvt
    ${TEST_RESOURCES_ROOT}/z3x1y0.mvt
    ${TEST_RESOURCES_ROOT}/z3x1y1.mvt
    ${TEST_RESOURCES_ROOT}/z3x1y2.mvt
    ${TEST_RESOURCES_ROOT}/z3x1y3.mvt
    ${TEST_RESOURCES_ROOT}/z3x1y4.mvt
    ${TEST_RESOURCES_ROOT}/z3x1y5.mvt
    ${TEST_RESOURCES_ROOT}/z3x1y6.mvt
    ${TEST_RESOURCES_ROOT}/z3x1y7.mvt
    ${TEST_RESOURCES_ROOT}/z3x2y0.mvt
    ${TEST_RESOURCES_ROOT}/z3x2y1.mvt
    ${TEST_RESOURCES_ROOT}/z3x2y2.mvt
    ${TEST_RESOURCES_ROOT}/z3x2y3.mvt
    ${TEST_RESOURCES_ROOT}/z3x2y4.mvt
    ${TEST_RESOURCES_ROOT}/z3x2y5.mvt
    ${TEST_RESOURCES_ROOT}/z3x2y6.mvt
    ${TEST_RESOURCES_ROOT}/z3x2y7.mvt
    ${TEST_RESOURCES_ROOT}/z3x3y0.mvt
    ${TEST_RESOURCES_ROOT}/z3x3y1.mvt
    ${TEST_RESOURCES_ROOT}/z3x3y2.mvt
    ${TEST_RESOURCES_ROOT}/z3x3y3.mvt
    ${TEST_RESOURCES_ROOT}/z3x3y4.mvt
    ${TEST_RESOURCES_ROOT}/z3x3y5.mvt
    ${TEST_RESOURCES_ROOT}/z3x3y6.mvt
    ${TEST_RESOURCES_ROOT}/z3x3y7.mvt
)
