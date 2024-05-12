
qt_add_executable(tileloader_test tileloader.cpp)
qt_add_resources(tileloader_test "TileLoader_unittest_resources"
    FILES
    loadTileFromCache_fails_on_broken_file/file.mvt
    loadTileFromCache_parses_cached_file_successfully/file.mvt
)
target_link_libraries(tileloader_test PUBLIC maplib Qt6::Test)

deploy_runtime_dependencies_if_win32(tileloader_test)

add_test(NAME TileLoaderTest COMMAND tileloader_test)
