add_executable(tile_parsing_benchmark tile_parsing_benchmark.cpp)
target_link_libraries(tile_parsing_benchmark PUBLIC maplib Qt6::Test)
set(TEST_RESOURCES_ROOT "resources")
qt_add_resources(tile_parsing_benchmark "unitTestResources_TileParsingBenchmark"
    PREFIX "/"
    BASE ${TEST_RESOURCES_ROOT}
    FILES
    ${TEST_RESOURCES_ROOT}/z1x0y0.mvt
    ${TEST_RESOURCES_ROOT}/z1x0y1.mvt
    ${TEST_RESOURCES_ROOT}/z1x1y0.mvt
    ${TEST_RESOURCES_ROOT}/z1x1y1.mvt
)
