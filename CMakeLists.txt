cmake_minimum_required(VERSION 3.16)

set(CMAKE_AUTOMOC ON)

project(mbtest  LANGUAGES CXX)

find_package(Qt6 REQUIRED COMPONENTS Core Protobuf Grpc Widgets Network)

qt_standard_project_setup()

set(PROJECT_SOURCES
        main.cpp
        vectortile.h vectortile.cpp
        tilerenderrules.h tilerenderrules.cpp
        tilerenderer.h tilerenderer.cpp
)

qt_add_executable(mbtest
    ${PROJECT_SOURCES}
)

qt_add_resources(mbtest "testData"
    PREFIX "/"
    FILES
    testData/tiles.json
    testData/z8x135y181.mvt
    testData/z12x2170y1190.mvt
    testData/z12x2170y2905.mvt
)

qt_add_protobuf(mbtest
    PROTO_FILES
    vector_tile.proto
)

target_link_libraries(mbtest PRIVATE
    Qt6::Core
    Qt6::Protobuf
    Qt6::Widgets
    Qt6::Network
)
