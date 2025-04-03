cmake_minimum_required(VERSION 2.8.12)

if (NOT OBJECT_MODEL_LOCAL_TESTS_ENABLED)
    project(PackageTest CXX)

    include(${CMAKE_BINARY_DIR}/conanbuildinfo.cmake)
    conan_basic_setup(TARGETS)

    include(ntt_cmake_tools)
    NttAddDebugInfoToReleaseConfiguration()
endif()

find_package(Qt5 REQUIRED COMPONENTS Gui Qml)

set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTORCC ON)

set(SOURCES
    main.cpp
    test_object.h
)

add_executable(trivial_test WIN32 ${SOURCES})
target_include_directories(trivial_test PUBLIC ${CMAKE_CURRENT_BINARY_DIR})

if (OBJECT_MODEL_LOCAL_TESTS_ENABLED)
    add_compile_definitions(OBJECT_MODEL_LOCAL_TESTS_ENABLED)
    target_link_libraries(trivial_test PUBLIC object_modellib)
else()
   target_link_libraries(trivial_test PUBLIC CONAN_PKG::object_model)
endif()
target_link_libraries(trivial_test PUBLIC Qt5::Gui)
target_link_libraries(trivial_test PUBLIC Qt5::Qml)
if(WIN32)
    NttCopyBinaryDependencies(TARGET trivial_test BINARIES QT)
endif()
