cmake_minimum_required(VERSION 3.9.3)

include(../protobuf_generate_qt.cmake)

set(BASE_TEST_FILES
    qt_core_test.h
    qt_core_test.cpp
    data_test.h
)
source_group("base" FILES ${BASE_TEST_FILES})

set(TEST_MODELS_FILES
    generated_test_enum_model.cpp
    generated_ip_endpoint_model.cpp
)
source_group("models" FILES ${TEST_MODELS_FILES})

set(TEST_OBJECTS_FILES
    generated_ip_endpoint_object.cpp
    generated_test_object.cpp
)
source_group("objects" FILES ${TEST_OBJECTS_FILES})

# Исходники
set(SOURCE_TEST_FILES
    main.cpp
    ${BASE_TEST_FILES}
    ${TEST_MODELS_FILES}
    ${TEST_OBJECTS_FILES}
)


add_executable(protobuf_generate_qt_testlib ${SOURCE_TEST_FILES})

target_link_libraries(protobuf_generate_qt_testlib PUBLIC CommonLib)


add_subdirectory(proto)



