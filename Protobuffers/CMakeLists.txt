cmake_minimum_required(VERSION 3.14)

project(Protobuffers LANGUAGES CXX)

set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(Qt6 REQUIRED COMPONENTS Core Protobuf Grpc Widgets Network)

qt_standard_project_setup()

add_executable(Protobuffers
  main.cpp
)

qt_add_protobuf(Protobuffers
    PROTO_FILES
    vector_tile.proto
)


target_link_libraries(Protobuffers PRIVATE
    Qt6::Core
    Qt6::Protobuf
    Qt6::Widgets
    Qt6::Network
)

include(GNUInstallDirs)
install(TARGETS Protobuffers
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
)
