cmake_minimum_required(VERSION 3.20)
project(PackageTest CXX)

include(../protobuf_generate_qt.cmake)

# Добавляем подпроект с protobuf-схемами
add_subdirectory(proto)

# Файлы QML
set(QML_SOURCES
        qml/TestWindow.qml
        qml/SingleInput.qml
        qml/OneOfInput.qml
        qml/OptionalInput.qml
        qml/RepeatedTable.qml
        qml/TableAddItem.qml
)

# Исходные файлы проекта
set(SOURCES
        filesystemqml.h
        filesystemqml.cpp
        main.cpp
        qtquickcontrols2.conf
        main.qrc
)

# Подключаем Protobuf
find_package(Protobuf REQUIRED)

# Создаём исполняемый файл
add_executable(trivial_test WIN32 ${SOURCES} ${QML_SOURCES})

# Подключаем общую библиотеку с зависимостями
target_link_libraries(trivial_test PUBLIC CommonLib)
target_link_libraries(trivial_test PUBLIC protocol_qt_test_packlib)

