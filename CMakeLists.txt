cmake_minimum_required(VERSION 3.20)

project(qt-map-thesis VERSION 0.1 LANGUAGES CXX)

option(BUILD_TESTS "Whether to build tests or not" OFF)

set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
# We need to look for packages that are hard requirements for the code to run.
find_package(Qt6 REQUIRED COMPONENTS Core Widgets Protobuf ProtobufTools Network)
qt_standard_project_setup()


# If on Windows:
# Setup a function to apply the windeployqt.exe tool to a given executable target.
# This copies the necessary .dll files into the executables folder.
if (WIN32 AND NOT DEFINED WINDEPLOYQT_EXECUTABLE)
    message(WARNING "Qt tool windeployqt not found. Executables might not be able to run outside Qt Creator.")
endif()
function(deploy_runtime_dependencies_if_win32 target)
    if (DEFINED WINDEPLOYQT_EXECUTABLE)
        add_custom_command(TARGET ${target} POST_BUILD
            COMMAND "${WINDEPLOYQT_EXECUTABLE}"
            ARGS --no-translations --no-compiler-runtime --no-system-d3d-compiler --no-system-dxc-compiler --no-opengl-sw $<TARGET_FILE:${target}>
            COMMENT "Running windeployqt on target..."
        )
    endif()
endfunction()

# Define the main library that holds most of our functionality.
# This is a static library that will be linked to our other executables.
qt_add_library(maplib STATIC
    lib/VectorTiles.cpp
    lib/VectorTiles.h
    lib/Rendering.h
    lib/Rendering.cpp
    lib/Rendering_Line.cpp
    lib/Rendering_Math.cpp
    lib/Rendering_Polygon.cpp
    lib/Rendering_Text.cpp
    lib/TileCoord.h
    lib/TileCoord.cpp
    lib/Layerstyle.h
    lib/Layerstyle.cpp
    lib/TileLoader.h
    lib/TileLoader.cpp
    lib/Evaluator.h
    lib/Evaluator.cpp
    lib/Utilities.h
    lib/Utilities.cpp
    lib/RequestTilesResult.h
    lib/layerStyle_Background.cpp)

# Qt containers by default don't include asserts (i.e out of bounds checks) in their containers
# under the RelWithDebInfo config. We enforce that they do to help us catch bugs.
if(CMAKE_BUILD_TYPE STREQUAL "RelWithDebInfo" OR CMAKE_BUILD_TYPE STREQUAL "DEBUG")
    target_compile_definitions(maplib PUBLIC QT_FORCE_ASSERTS)
endif()

qt_add_protobuf(maplib
    PROTO_FILES
        lib/vector_tile.proto
)
# Link our "include" folder that contains the heades files
target_include_directories(maplib PUBLIC "lib")
# Link our library to the Qt6 components.
target_link_libraries(maplib PUBLIC Qt6::Widgets Qt6::Protobuf Qt6::Network)

# Setup the primary QWidgets application
set(PROJECT_SOURCES
    app/main.cpp
    app/MapWidget.h
    app/MapWidget.cpp
    app/MapZoomControlWidget.h
    app/MapZoomControlWidget.cpp
    app/MapPanControlWidget.h
    app/MapPanControlWidget.cpp
    app/MapCoordControlWidget.h
    app/MapCoordControlWidget.cpp
    app/MapRenderSettingsWidget.h
    app/MapRenderSettingsWidget.cpp
    app/MainWindow.h
    app/MainWindow.cpp)

qt_add_executable(application
    MANUAL_FINALIZATION
    ${PROJECT_SOURCES})

# Link our app to our library target.
target_link_libraries(application PUBLIC maplib)

# Nils: Not sure how relevant these particular commands are.
set_target_properties(application PROPERTIES
    ${BUNDLE_ID_OPTION}
    MACOSX_BUNDLE_BUNDLE_VERSION ${PROJECT_VERSION}
    MACOSX_BUNDLE_SHORT_VERSION_STRING ${PROJECT_VERSION_MAJOR}.${PROJECT_VERSION_MINOR}
    MACOSX_BUNDLE TRUE
    WIN32_EXECUTABLE TRUE
)

# Nils: Might not be necessary for our project.
include(GNUInstallDirs)
install(TARGETS application
    BUNDLE DESTINATION .
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
)

qt_finalize_executable(application)
# Apply the windows deployment process to our executable.
deploy_runtime_dependencies_if_win32(application)

# Create the example command line utility example.
qt_add_executable(cmd_example cmd_example.cpp)
target_link_libraries(cmd_example PUBLIC maplib)

# All testing related code goes in here
if (BUILD_TESTS)
    set(CMAKE_AUTOMOC ON)
    # Find the necessary packages on the system.
    find_package(Qt6 REQUIRED COMPONENTS Test)

    # Add the first test executable: tileloader test
    qt_add_executable(tileloader_test tests/unit-tests/unittesting_tileloader.cpp)
    qt_add_resources(tileloader_test "unitTestResourcesTileLoader"
        PREFIX "/"
        FILES
        unitTestResources/loadTileFromCache_fails_on_broken_file/file.mvt
        unitTestResources/loadTileFromCache_fails_on_broken_file/file.png
        unitTestResources/loadTileFromCache_parses_cached_file_successfully/file.mvt
        unitTestResources/loadTileFromCache_parses_cached_file_successfully/file.png
    )
    target_link_libraries(tileloader_test PUBLIC maplib Qt6::Test)


    # Add the second executable: rendering unit test
    qt_add_executable(render_test tests/unit-tests/unittesting_rendering.cpp)
    qt_add_resources(render_test "unitTestResourcesRendering")
    target_link_libraries(render_test PUBLIC maplib Qt6::Test)

    # Add the third executable: layerstyle test
    qt_add_executable(layerstyle_test tests/unit-tests/unittesting_layerstyle.cpp)
    qt_add_resources(layerstyle_test "unitTestResourcesLayerStyle"
        PREFIX "/"
        FILES
        unitTestResources/styleTest.json
    )
    target_link_libraries(layerstyle_test PUBLIC maplib Qt6::Test)


    # Add the fourth executable: vectortile test
    qt_add_executable(vectortile_test tests/unit-tests/unittesting_vectortile.cpp)
    qt_add_resources(vectortile_test "unitTestResourcesVectorTile"
        PREFIX "/"
        FILES
        unitTestResources/000testTile.pbf
    )
    target_link_libraries(vectortile_test PUBLIC maplib Qt6::Test)

    # Add the fifth executable: evaluator test
    qt_add_executable(evaluator_test tests/unit-tests/unittest_evaluator.cpp)
    qt_add_resources(evaluator_test "unitTestResourcesEvaluator"
        PREFIX "/"
        FILES
        unitTestResources/expressionTest.json
        )
    target_link_libraries(evaluator_test PUBLIC maplib Qt6::Test)

    # Make sure we deploy the correct runtime dependencies to the test.
    deploy_runtime_dependencies_if_win32(tileloader_test)
    deploy_runtime_dependencies_if_win32(render_test)
    deploy_runtime_dependencies_if_win32(layerstyle_test)
    deploy_runtime_dependencies_if_win32(vectortile_test)
    deploy_runtime_dependencies_if_win32(evaluator_test)

    # Add executables to CTest.
    enable_testing()
    add_test(NAME TileLoaderTest COMMAND tileloader_test)
    add_test(NAME RenderTest COMMAND render_test)
    add_test(NAME LayerStyleTest COMMAND layerstyle_test)
    add_test(NAME VectorTileTest COMMAND vectortile_test)
    add_test(NAME EvaluatorTest COMMAND evaluator_test)
endif()
