cmake_minimum_required(VERSION 2.8.12)

find_package(Qt5 REQUIRED COMPONENTS Gui Qml Test)

set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTORCC ON)

# Исходники
set(SOURCES
    main.cpp
    object_model_test.cpp
    sort_filter_test.cpp
)

# Заголовочные файлы
set(HEADERS
    test_object.h
)

NttUseConanLibOrSubmodule(gtest log)

# Результатом сборки будет исполняемый файл из SOURCES + HEADERS
add_executable(object_modellib_tests ${SOURCES} ${HEADERS})

target_link_libraries(object_modellib_tests PRIVATE 
    object_modellib 
    ${gtestlib}
    ${loglib}
    Qt5::Gui
    Qt5::Qml
    Qt5::Test
)

if(WIN32)
    NttUseConanLibOrSubmodule(application)
    target_link_libraries(object_modellib_tests PRIVATE 
    ${applicationlib}
)
endif()

# В solution эта библиотека лежит в tests/gui
set_property(TARGET object_modellib_tests PROPERTY FOLDER "tests/gui")

NttAddGoogleTest(NAME object_modellib_tests TIMEOUT 600)
NttWinDeployQt(TARGET object_modellib_tests)
