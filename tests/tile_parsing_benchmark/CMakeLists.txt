add_executable(tile_parsing_benchmark tile_parsing_benchmark.cpp)
target_link_libraries(tile_parsing_benchmark PUBLIC maplib Qt6::Test)
set(TEST_RESOURCES_ROOT "resources")
qt_add_resources(tile_parsing_benchmark "unitTestResources_TileParsingBenchmark"
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
)
