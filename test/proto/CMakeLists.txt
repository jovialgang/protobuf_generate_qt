cmake_minimum_required(VERSION 3.20)

set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTORCC ON)

set(PROTO_FILES
    test.proto
    common.proto
)

find_package(Qt5 REQUIRED COMPONENTS Qml)

add_protobuf_generated_library_with_qt(LIB_NAME protocol_testlib QT_LIB_NAME protocol_qt_testlib PROTO_FILES ${PROTO_FILES})

set_property(TARGET protocol_testlib PROPERTY CXX_STANDARD 17)
set_property(TARGET protocol_qt_testlib PROPERTY CXX_STANDARD 17)

target_link_libraries(protobuf_generate_qt_testlib PUBLIC protocol_testlib)
target_link_libraries(protobuf_generate_qt_testlib PUBLIC protocol_qt_testlib)

set_property(TARGET protocol_testlib PROPERTY FOLDER "tests/gui")
set_property(TARGET protocol_qt_testlib PROPERTY FOLDER "tests/gui")