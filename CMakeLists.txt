cmake_minimum_required(VERSION 3.20)
project(protobuf_generate_qt CXX)

# Устанавливаем стандарт C++
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Включаем автоматическую обработку Qt
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTORCC ON)

# Пути для выходных файлов
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)

# Включаем внешний CMake-файл

# Подключаем зависимости
find_package(Protobuf REQUIRED)
find_package(absl REQUIRED)
find_package(GTest REQUIRED)
find_package(Qt5 REQUIRED COMPONENTS Core Widgets Quick QuickControls2 Qml Test)

# Добавляем include-директории
include_directories(${Protobuf_INCLUDE_DIRS} ${absl_INCLUDE_DIRS})

# Подключаем сабмодули
add_subdirectory(object_model)
add_subdirectory(test_package)
add_subdirectory(test)

# Общая библиотека с зависимостями
add_library(CommonLib INTERFACE)
target_include_directories(CommonLib INTERFACE ${Protobuf_INCLUDE_DIRS} ${absl_INCLUDE_DIRS})
target_link_libraries(CommonLib INTERFACE
        Qt5::Core Qt5::Quick Qt5::QuickControls2 Qt5::Test
        object_modellib
        ${Protobuf_LIBRARIES}
        absl::base absl::log absl::log_internal_check_op absl::log_internal_message
        absl::log_severity absl::raw_logging_internal absl::spinlock_wait
        GTest::GTest GTest::Main
)
