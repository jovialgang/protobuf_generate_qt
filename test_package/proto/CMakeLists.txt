cmake_minimum_required(VERSION 3.20)

set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTORCC ON)

set(PROTO_FILES
    test.proto
)

find_package(Qt5 REQUIRED COMPONENTS Qml)

add_protobuf_generated_library_with_qt(LIB_NAME protocol_test_packlib QT_LIB_NAME protocol_qt_test_packlib PROTO_FILES ${PROTO_FILES})

set_property(TARGET protocol_test_packlib PROPERTY CXX_STANDARD 17)
set_property(TARGET protocol_qt_test_packlib PROPERTY CXX_STANDARD 17)

set_property(TARGET protocol_test_packlib PROPERTY FOLDER "trivial_test_common")
set_property(TARGET protocol_qt_test_packlib PROPERTY FOLDER "trivial_test_common")



