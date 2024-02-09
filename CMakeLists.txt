cmake_minimum_required(VERSION 3.20)

project(maptest VERSION 0.1 LANGUAGES CXX)

set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(Qt6 REQUIRED COMPONENTS Core Widgets Protobuf)
qt_standard_project_setup()



qt_add_library(maplib STATIC
    lib/VectorTiles.cpp
    lib/VectorTiles.h
    lib/Rendering.cpp
    lib/Rendering.h
    lib/TileCoord.h)
qt_add_protobuf(maplib
    PROTO_FILES
        lib/vector_tile.proto
)
target_include_directories(maplib PUBLIC "lib")
target_link_libraries(maplib PUBLIC Qt6::Widgets Qt6::Protobuf)



set(PROJECT_SOURCES
    main.cpp
    MapWidget.h
    MapWidget.cpp)
qt_add_executable(maptest
    MANUAL_FINALIZATION
    ${PROJECT_SOURCES}
)

target_link_libraries(maptest PUBLIC maplib)

set_target_properties(maptest PROPERTIES
    ${BUNDLE_ID_OPTION}
    MACOSX_BUNDLE_BUNDLE_VERSION ${PROJECT_VERSION}
    MACOSX_BUNDLE_SHORT_VERSION_STRING ${PROJECT_VERSION_MAJOR}.${PROJECT_VERSION_MINOR}
    MACOSX_BUNDLE TRUE
    WIN32_EXECUTABLE TRUE
)

include(GNUInstallDirs)
install(TARGETS maptest
    BUNDLE DESTINATION .
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
)

qt_finalize_executable(maptest)


qt_add_executable(cmd_example cmd_example.cpp)
target_link_libraries(cmd_example PUBLIC maplib)
