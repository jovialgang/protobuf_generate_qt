# CMake модуль для генерации C++ кода из protobuf файлов с поддержкой Qt
# Этот модуль предоставляет функции для создания библиотек, которые интегрируют
# protobuf сообщения с Qt моделями и QML

cmake_minimum_required(VERSION 3.20)

# Определяет имя библиотеки gRPC в зависимости от доступных целей
function(GetGrpcLibName)
    if(TARGET CONAN_PKG::gRPC)
        set(GRPC_LIB_NAME CONAN_PKG::gRPC PARENT_SCOPE)
    elseif(TARGET CONAN_PKG::grpc)
        set(GRPC_LIB_NAME CONAN_PKG::grpc PARENT_SCOPE)
    else()
        set(GRPC_LIB_NAME gRPClib PARENT_SCOPE)
    endif()
endfunction()

# Создает статическую библиотеку из protobuf файлов
# Параметры:
#   GENERATE_GRPC - если установлен, генерирует также gRPC код
#   LIB_NAME - имя создаваемой библиотеки
#   PROTO_FILES - список .proto файлов для обработки
function(add_protobuf_generated_library)
    cmake_parse_arguments(THIS 
        "GENERATE_GRPC" 
        "LIB_NAME"
        "PROTO_FILES"
        ${ARGN})

    find_package(Protobuf REQUIRED)

    if(NOT DEFINED THIS_PROTO_FILES)
        message(SEND_ERROR "Error: add_protobuf_library() called without any proto files")
        return()
    endif()

    # Генерация стандартных protobuf файлов (pb.cc/pb.h)
    PROTOBUF_GENERATE_CPP(${THIS_LIB_NAME}_PROTOBUF_CPP_FILES ${THIS_LIB_NAME}_PROTOBUF_HEADER_FILES ${THIS_PROTO_FILES})

    # Создание библиотеки с поддержкой gRPC если требуется
    if(${THIS_GENERATE_GRPC})
        GetGrpcLibName() 
        GRPC_GENERATE_CPP(${THIS_LIB_NAME}_GRPC_CPP_FILES ${THIS_LIB_NAME}_GRPC_HEADER_FILES ${THIS_PROTO_FILES})

        add_library(${THIS_LIB_NAME} STATIC ${THIS_PROTO_FILES} ${${THIS_LIB_NAME}_PROTOBUF_CPP_FILES} ${${THIS_LIB_NAME}_PROTOBUF_HEADER_FILES}
                    ${${THIS_LIB_NAME}_GRPC_CPP_FILES} ${${THIS_LIB_NAME}_GRPC_HEADER_FILES})
        target_link_libraries(${THIS_LIB_NAME} PUBLIC ${GRPC_LIB_NAME})
        target_compile_features(${THIS_LIB_NAME} PRIVATE cxx_std_17)
    else()
        add_library(${THIS_LIB_NAME} STATIC ${THIS_PROTO_FILES} ${${THIS_LIB_NAME}_PROTOBUF_CPP_FILES} ${${THIS_LIB_NAME}_PROTOBUF_HEADER_FILES})
        target_link_libraries(${THIS_LIB_NAME} PUBLIC ${Protobuf_LIBRARIES})
        target_compile_features(${THIS_LIB_NAME} PRIVATE cxx_std_17)
    endif()
    
    # Настройка предкомпиляции заголовков и директорий включения
    target_precompile_headers(${THIS_LIB_NAME} PRIVATE ${${THIS_LIB_NAME}_PROTOBUF_HEADER_FILES})
    target_include_directories(${THIS_LIB_NAME} PUBLIC ${CMAKE_CURRENT_BINARY_DIR})
    
    # Отключение предупреждений для Windows
    if(WIN32)
        target_compile_options(${THIS_LIB_NAME} PUBLIC "-wd4244")    # warning C4244: 'initializing': conversion from 'int64_t' to 'size_t', possible loss of data
        target_compile_options(${THIS_LIB_NAME} PUBLIC "-wd4100")    # warning C4100: 'status': unreferenced formal parameter
        target_compile_options(${THIS_LIB_NAME} PUBLIC "-wd4125")    # warning C4125: decimal digit terminates octal escape sequence
        target_compile_definitions(${THIS_LIB_NAME} PRIVATE -D_SILENCE_CXX17_ITERATOR_BASE_CLASS_DEPRECATION_WARNING)
    endif()
endfunction()

# Создает Qt-библиотеку из protobuf файлов с поддержкой QML
# Параметры:
#   GENERATE_GRPC - если установлен, генерирует также gRPC код
#   LIB_NAME - имя protobuf библиотеки
#   QT_LIB_NAME - имя Qt библиотеки
#   PROTO_FILES - список .proto файлов для обработки
function(add_protobuf_generated_qt_library)
    cmake_parse_arguments(THIS 
        "GENERATE_GRPC" 
        "LIB_NAME;QT_LIB_NAME"
        "PROTO_FILES"
        ${ARGN})

    find_package(Protobuf REQUIRED)

    # Включение автоматической обработки Qt метаданных
    set(CMAKE_AUTOMOC ON)
    set(CMAKE_AUTOUIC ON)
    set(CMAKE_AUTORCC ON)

    # Генерация Qt-оберток для protobuf типов
    protobuf_generate_qt(SRCS ${THIS_QT_LIB_NAME}_QT_CPP_FILES HDRS ${THIS_QT_LIB_NAME}_QT_H_FILES PROTO_FILES ${THIS_PROTO_FILES})

    # Создание Qt библиотеки и настройка зависимостей
    add_library(${THIS_QT_LIB_NAME} STATIC ${${THIS_QT_LIB_NAME}_QT_CPP_FILES} ${${THIS_QT_LIB_NAME}_QT_H_FILES})
    target_compile_features(${THIS_QT_LIB_NAME} PRIVATE cxx_std_17)
    target_include_directories(${THIS_QT_LIB_NAME} PUBLIC ${CMAKE_CURRENT_BINARY_DIR})

    # Настройка зависимостей в зависимости от наличия gRPC
    if(${THIS_GENERATE_GRPC})
        GetGrpcLibName()
        add_protobuf_generated_qt_common_library(THIS_GENERATE_GRPC)
        target_link_libraries(${THIS_QT_LIB_NAME} PUBLIC ${GRPC_LIB_NAME})
    else()
        add_protobuf_generated_qt_common_library() 
        target_link_libraries(${THIS_QT_LIB_NAME} PUBLIC ${Protobuf_LIBRARIES})
    endif()
     
    # Настройка предкомпиляции заголовков и зависимостей
    target_precompile_headers(${THIS_QT_LIB_NAME} PRIVATE ${${THIS_QT_LIB_NAME}_QT_H_FILES})
    target_link_libraries(${THIS_QT_LIB_NAME} PUBLIC Qt5::Qml)
    target_link_libraries(${THIS_QT_LIB_NAME} PUBLIC object_modellib)
    target_link_libraries(${THIS_QT_LIB_NAME} PUBLIC ${PROTOBUF_QT_COMMON_LIB_NAME})
    target_link_libraries(${THIS_QT_LIB_NAME} PUBLIC ${THIS_LIB_NAME})

    # Отключение предупреждений для Windows
    if(WIN32)
        target_compile_options(${THIS_QT_LIB_NAME} PUBLIC "-wd4127")    # warning C4127: conditional expression is constant
        target_compile_options(${THIS_QT_LIB_NAME} PUBLIC "-wd4702")    # warning C4702: unreachable code
        target_compile_options(${THIS_QT_LIB_NAME} PUBLIC "-wd4100")    # warning C4100: 'parent': unreferenced formal parameter
        target_compile_definitions(${THIS_QT_LIB_NAME} PRIVATE -D_SILENCE_CXX17_ITERATOR_BASE_CLASS_DEPRECATION_WARNING)
    endif()
endfunction()

# Создает обе библиотеки (protobuf и Qt) за один вызов
# Параметры:
#   GENERATE_GRPC - если установлен, генерирует также gRPC код
#   LIB_NAME - имя protobuf библиотеки
#   QT_LIB_NAME - имя Qt библиотеки
#   PROTO_FILES - список .proto файлов для обработки
function(add_protobuf_generated_library_with_qt)
    cmake_parse_arguments(THIS 
        "GENERATE_GRPC" 
        "LIB_NAME;QT_LIB_NAME"
        "PROTO_FILES" 
         ${ARGN})

    # Подготовка аргументов для вызова функций
    set(THIS_ARGUMENTS LIB_NAME ${THIS_LIB_NAME} QT_LIB_NAME ${THIS_QT_LIB_NAME} PROTO_FILES ${THIS_PROTO_FILES})
    if(${THIS_GENERATE_GRPC})
        list(APPEND THIS_ARGUMENTS "GENERATE_GRPC")
    endif()

    # Создание обеих библиотек
    add_protobuf_generated_library(${THIS_ARGUMENTS})
    add_protobuf_generated_qt_library(${THIS_ARGUMENTS})
endfunction()

# Генерирует Qt-обертки для protobuf типов с помощью Python скрипта
# Параметры:
#   SRCS - выходной параметр для сгенерированных .cpp файлов
#   HDRS - выходной параметр для сгенерированных .h файлов
#   PROTO_FILES - список .proto файлов для обработки
function(protobuf_generate_qt)
    cmake_parse_arguments(THIS 
        "" 
        ""
        "SRCS;HDRS;PROTO_FILES" 
         ${ARGN})

    find_package(Python REQUIRED COMPONENTS Interpreter)

    if(NOT DEFINED THIS_PROTO_FILES)
        message(SEND_ERROR "Error: PROTOBUF_GENERATE_QT() called without any proto files")
        return()
    endif()
        
    set(SCRIPT_NAME protobuf_generate_qt.py)

    # Поиск Python скрипта для генерации
    if(NOT DEFINED PROTOBUF_GENERATE_QT_SCRIPT_PATH)
        set(PROTOBUF_GENERATE_QT_SCRIPT_FULL_PATH "${CMAKE_CURRENT_FUNCTION_LIST_DIR}/python/${SCRIPT_NAME}")
        if(EXISTS ${PROTOBUF_GENERATE_QT_SCRIPT_FULL_PATH})
            set(PROTOBUF_GENERATE_QT_SCRIPT_PATH "${PROTOBUF_GENERATE_QT_SCRIPT_FULL_PATH}")
            message(VERBOSE "Protobuf generate qt script path: ${PROTOBUF_GENERATE_QT_SCRIPT_PATH}")
            set(PROTOBUF_GENERATE_QT_SCRIPT_PATH "${PROTOBUF_GENERATE_QT_SCRIPT_PATH}" CACHE FILEPATH "protobuf generate qt python script path")
        endif()
    endif()
    if(NOT DEFINED PROTOBUF_GENERATE_QT_SCRIPT_PATH)
        message(FATAL_ERROR "Unable to find protobuf generate qt python script (${SCRIPT_NAME})")
    endif()

    # Настройка директорий для поиска .proto файлов
    if(DEFINED PROTOBUF_IMPORT_DIRS AND NOT DEFINED Protobuf_IMPORT_DIRS)
        set(Protobuf_IMPORT_DIRS "${PROTOBUF_IMPORT_DIRS}")
    endif()
    foreach(DIR ${Protobuf_IMPORT_DIRS})
        get_filename_component(DIR_PATH_ABS ${DIR} ABSOLUTE)
        list(FIND PROTOBUF_IMPORT_PATHS ${DIR_PATH_ABS} CONTAINS_ALREADY)
        if(${CONTAINS_ALREADY} EQUAL -1)
            list(APPEND PROTOBUF_IMPORT_PATHS ${DIR_PATH_ABS})
        endif()
    endforeach()

    # Подготовка путей для выходных файлов
    set(${THIS_SRCS})
    set(${THIS_HDRS})
    set(PROTO_PATH_ABS)

    # Формирование списков входных и выходных файлов
    foreach(FILE ${THIS_PROTO_FILES})
        get_filename_component(FILE_PATH ${FILE} ABSOLUTE)
        get_filename_component(FILE_NAME ${FILE} NAME_WE)

        list(APPEND PROTO_PATH_ABS "${FILE_PATH}")
        list(APPEND ${THIS_SRCS} "${CMAKE_CURRENT_BINARY_DIR}/${FILE_NAME}_qt_pb.cpp")
        list(APPEND ${THIS_HDRS} "${CMAKE_CURRENT_BINARY_DIR}/${FILE_NAME}_qt_pb.h")
    endforeach()

    # Запуск Python скрипта для генерации кода
    add_custom_command(
        OUTPUT ${${THIS_HDRS}} ${${THIS_SRCS}}
        COMMENT "Generating protobuf Qt wrapper types"
        COMMAND ${Python_EXECUTABLE}
        ARGS ${PROTOBUF_GENERATE_QT_SCRIPT_PATH} -proto_files ${PROTO_PATH_ABS} -import_dirs ${PROTOBUF_IMPORT_PATHS} -output ${CMAKE_CURRENT_BINARY_DIR}
        DEPENDS ${THIS_PROTO_FILES}
        VERBATIM
    )
    
    # Установка свойств сгенерированных файлов
    set_source_files_properties(${${THIS_HDRS}} ${${THIS_SRCS}} PROPERTIES GENERATED TRUE)
    set(${THIS_SRCS} ${${THIS_SRCS}} PARENT_SCOPE)
    set(${THIS_HDRS} ${${THIS_HDRS}} PARENT_SCOPE)
endfunction()

# Создает общую библиотеку с Qt-типами для protobuf
# Параметры:
#   GENERATE_GRPC - если установлен, генерирует также gRPC код
function(add_protobuf_generated_qt_common_library)
    cmake_parse_arguments(THIS 
        "GENEARATE_GRPC" 
        ""
        "" 
        ${ARGN})

    find_package(Protobuf REQUIRED)

    # Включение автоматической обработки Qt метаданных
    set(CMAKE_AUTOMOC ON)
    set(CMAKE_AUTOUIC ON)
    set(CMAKE_AUTORCC ON)

    # Установка имени общей библиотеки
    set(PROTOBUF_QT_COMMON_LIB_NAME "protobuf_qt_commonlib")
    set(PROTOBUF_QT_COMMON_LIB_NAME "protobuf_qt_commonlib" PARENT_SCOPE)

    # Создание библиотеки только если она еще не существует
    if(NOT TARGET ${PROTOBUF_QT_COMMON_LIB_NAME})
        # Генерация общих типов
        protobuf_generate_qt_common_types(SRCS PROTOBUF_QT_COMMON_LIB_NAME_SRCS HDRS PROTOBUF_QT_COMMON_LIB_NAME_HDRS) 
        
        # Создание библиотеки и настройка зависимостей
        add_library(${PROTOBUF_QT_COMMON_LIB_NAME} STATIC ${PROTOBUF_QT_COMMON_LIB_NAME_SRCS} ${PROTOBUF_QT_COMMON_LIB_NAME_HDRS})
        target_compile_features(${PROTOBUF_QT_COMMON_LIB_NAME} PRIVATE cxx_std_17)
        target_include_directories(${PROTOBUF_QT_COMMON_LIB_NAME} PUBLIC ${CMAKE_CURRENT_BINARY_DIR})

        # Настройка зависимостей в зависимости от наличия gRPC
        if(${THIS_GENERATE_GRPC})
            GetGrpcLibName()
            target_link_libraries(${PROTOBUF_QT_COMMON_LIB_NAME} PUBLIC ${GRPC_LIB_NAME})
        else()
            target_link_libraries(${PROTOBUF_QT_COMMON_LIB_NAME} PUBLIC ${Protobuf_LIBRARIES})
        endif()
        
        # Добавление зависимостей Qt
        target_link_libraries(${PROTOBUF_QT_COMMON_LIB_NAME} PUBLIC Qt5::Qml)
        target_link_libraries(${PROTOBUF_QT_COMMON_LIB_NAME} PUBLIC object_modellib)
        
        # Отключение предупреждений для Windows
        if(WIN32)
            target_compile_options(${PROTOBUF_QT_COMMON_LIB_NAME} PUBLIC "-wd4127")    # warning C4127: conditional expression is constant
            target_compile_options(${PROTOBUF_QT_COMMON_LIB_NAME} PUBLIC "-wd4702")    # warning C4702: unreachable code
            target_compile_options(${PROTOBUF_QT_COMMON_LIB_NAME} PUBLIC "-wd4100")    # warning C4100: 'parent': unreferenced formal parameter
            target_compile_definitions(${PROTOBUF_QT_COMMON_LIB_NAME} PRIVATE -D_SILENCE_CXX17_ITERATOR_BASE_CLASS_DEPRECATION_WARNING)
        endif()
    endif()
endfunction()

# Генерирует общие Qt-типы для protobuf с помощью Python скрипта
# Параметры:
#   SRCS - выходной параметр для сгенерированных .cpp файлов
#   HDRS - выходной параметр для сгенерированных .h файлов
function(protobuf_generate_qt_common_types)
    cmake_parse_arguments(THIS 
        "" 
        ""
        "SRCS;HDRS" 
        ${ARGN})

    find_package(Python REQUIRED COMPONENTS Interpreter)

    set(SCRIPT_NAME protobuf_generate_qt_common_types.py)

    # Поиск Python скрипта для генерации общих типов
    if(NOT DEFINED PROTOBUF_GENERATE_QT_COMMON_TYPES_SCRIPT_PATH)
        set(PROTOBUF_GENERATE_QT_COMMON_TYPES_SCRIPT_FULL_PATH "${CMAKE_CURRENT_FUNCTION_LIST_DIR}/python/${SCRIPT_NAME}")
        if(EXISTS ${PROTOBUF_GENERATE_QT_COMMON_TYPES_SCRIPT_FULL_PATH})
            set(PROTOBUF_GENERATE_QT_COMMON_TYPES_SCRIPT_PATH "${PROTOBUF_GENERATE_QT_COMMON_TYPES_SCRIPT_FULL_PATH}")
            message(VERBOSE "Protobuf generate qt script path: ${PROTOBUF_GENERATE_QT_COMMON_TYPES_SCRIPT_PATH}")
            set(PROTOBUF_GENERATE_QT_COMMON_TYPES_SCRIPT_PATH "${PROTOBUF_GENERATE_QT_COMMON_TYPES_SCRIPT_PATH}" CACHE FILEPATH "protobuf generate qt python script path")
        endif()
    endif()
    if(NOT DEFINED PROTOBUF_GENERATE_QT_COMMON_TYPES_SCRIPT_PATH)
        message(FATAL_ERROR "Unable to find protobuf generate qt python script (${SCRIPT_NAME})")
    endif()

    set(${THIS_SRCS})
    set(${THIS_HDRS})
    
    # Assign _qt_pb.(cpp/h) file's paths to corresponding THIS_SRCS/THIS_HDRS.
    list(APPEND ${THIS_SRCS} "${CMAKE_CURRENT_BINARY_DIR}/data_type_models_qt_pb.cpp")
    list(APPEND ${THIS_HDRS} "${CMAKE_CURRENT_BINARY_DIR}/data_type_models_qt_pb.h")
    list(APPEND ${THIS_SRCS} "${CMAKE_CURRENT_BINARY_DIR}/well_known_types_qt_pb.cpp")
    list(APPEND ${THIS_HDRS} "${CMAKE_CURRENT_BINARY_DIR}/well_known_types_qt_pb.h")

    # Execute python script with specified path to .proto files
    add_custom_command(
        OUTPUT ${${THIS_HDRS}} ${${THIS_SRCS}}
        COMMENT "Generating protobuf Qt common types"
        COMMAND ${Python_EXECUTABLE}
        ARGS ${PROTOBUF_GENERATE_QT_COMMON_TYPES_SCRIPT_PATH} -output ${CMAKE_CURRENT_BINARY_DIR}
        VERBATIM
    )

    # THIS_SRCS/THIS_HDRS values will be available inside the caller after this function is completed
    set_source_files_properties(${${THIS_HDRS}} ${${THIS_SRCS}} PROPERTIES GENERATED TRUE)
    set(${THIS_SRCS} ${${THIS_SRCS}} PARENT_SCOPE)
    set(${THIS_HDRS} ${${THIS_HDRS}} PARENT_SCOPE)
endfunction()