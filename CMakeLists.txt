cmake_minimum_required(VERSION 3.20)

project(maptest VERSION 0.1 LANGUAGES CXX)

option(BUILD_TESTS "Whether to build tests or not" OFF)

set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)


# We need to look for packages that are hard requirements for the code to run.
find_package(Qt6 REQUIRED COMPONENTS Core Widgets Protobuf Network)
qt_standard_project_setup()


# If on Windows:
# Setup a function to apply the windeployqt.exe tool to a given executable target.
# This copies the necessary .dll files into the executables folder.
if (WIN32 AND NOT DEFINED WINDEPLOYQT_EXECUTABLE)
    message(WARNING "Qt tool windeployqt not found. Executables might not be able to run outside Qt Creator.")
endif()
function(win32_deploy target)
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
    lib/Rendering.cpp
    lib/Rendering.h
    lib/TileCoord.h
    lib/TileCoord.cpp
    lib/Layerstyle.h
    lib/Layerstyle.cpp
    lib/TileURL.h
    lib/TileURL.cpp
    lib/networkcontroller.h
    lib/networkcontroller.cpp)
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
    main.cpp
    MapWidget.h
    MapWidget.cpp
    MapZoomControlWidget.h
    MapZoomControlWidget.cpp
    MapPanControlWidget.h
    MapPanControlWidget.cpp
    MainWindow.h
    MainWindow.cpp)
qt_add_executable(maptest
    MANUAL_FINALIZATION
    ${PROJECT_SOURCES}
    )

# Link our app to our library target.
target_link_libraries(maptest PUBLIC maplib)

# Nils: Not sure how relevant these particular commands are.
set_target_properties(maptest PROPERTIES
    ${BUNDLE_ID_OPTION}
    MACOSX_BUNDLE_BUNDLE_VERSION ${PROJECT_VERSION}
    MACOSX_BUNDLE_SHORT_VERSION_STRING ${PROJECT_VERSION_MAJOR}.${PROJECT_VERSION_MINOR}
    MACOSX_BUNDLE TRUE
    WIN32_EXECUTABLE TRUE
)

# Nils: Might not be necessary for our project.
include(GNUInstallDirs)
install(TARGETS maptest
    BUNDLE DESTINATION .
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
)

qt_finalize_executable(maptest)
# Apply the windows deployment process to our executable.
win32_deploy(maptest)

# Set up function that applies copy directory command to executable,
# to copy the contents of our "testdata" folder into where the executable lives.
add_custom_command(
    TARGET maptest POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_directory
    ${CMAKE_SOURCE_DIR}/testdata $<TARGET_FILE_DIR:maptest>/testdata
)

# Create the example command line utility example.
qt_add_executable(cmd_example cmd_example.cpp)
target_link_libraries(cmd_example PUBLIC maplib)




# All testing related code goes in here
if (BUILD_TESTS)
    # Find the necessary packages on the system.
    find_package(Qt6 REQUIRED COMPONENTS Test)

    # This is our first test executable.
    qt_add_executable(
        unit_test
        tests/unit-tests/unittesting.cpp
    )
    target_link_libraries(unit_test PUBLIC maplib Qt6::Test)

    # Make sure we deploy the correct runtime dependencies to the test.
    win32_deploy(unit_test)

    # Adds our executable to CTest.
    enable_testing()
    add_test(NAME MyTest COMMAND unit_test)
endif()
