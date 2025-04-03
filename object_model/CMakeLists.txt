cmake_minimum_required(VERSION 3.6.0)
#set(CMAKE_CXX_STANDARD 14)

find_package(Boost REQUIRED COMPONENTS container)
find_package(Qt5 REQUIRED COMPONENTS Gui Qml)

set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTORCC ON)

set(OBJECT_MODEL_ROOT_DIR ${CMAKE_CURRENT_LIST_DIR})
set(OBJECT_MODEL_INCLUDE_DIR ${CMAKE_CURRENT_LIST_DIR}/include/gui/object_model)

# Исходники
set(META_DATA
    ${OBJECT_MODEL_INCLUDE_DIR}/roles.h
    ${OBJECT_MODEL_INCLUDE_DIR}/dynamic_roles.h
        src/dynamic_roles.cpp
    ${OBJECT_MODEL_INCLUDE_DIR}/signal_timer.h
        src/signal_timer.cpp
    ${OBJECT_MODEL_INCLUDE_DIR}/property_change_listener.h
        src/property_change_listener.cpp
    ${OBJECT_MODEL_INCLUDE_DIR}/object_meta_data.h
        src/object_meta_data.cpp
    ${OBJECT_MODEL_INCLUDE_DIR}/signal_binder.h
        src/signal_binder.cpp
    ${OBJECT_MODEL_INCLUDE_DIR}/property_binder.h
        src/property_binder.cpp
)

set(MODEL_ACCESS
    ${OBJECT_MODEL_INCLUDE_DIR}/model_value.h
        src/model_value.cpp
    ${OBJECT_MODEL_INCLUDE_DIR}/model_iterator.h
        src/model_iterator.cpp
    ${OBJECT_MODEL_INCLUDE_DIR}/model_range.h
        src/model_range.cpp
    ${OBJECT_MODEL_INCLUDE_DIR}/model_access.h
)

set(OBJECT_MODEL
    ${OBJECT_MODEL_INCLUDE_DIR}/object_model_qml.h
    ${OBJECT_MODEL_INCLUDE_DIR}/abstract_object_model.h
        src/abstract_object_model.cpp
    ${OBJECT_MODEL_INCLUDE_DIR}/object_model.h
        src/object_model.cpp
    ${OBJECT_MODEL_INCLUDE_DIR}/object_model_wrapper.h
    ${OBJECT_MODEL_INCLUDE_DIR}/object_model_vector_wrapper.h
    ${OBJECT_MODEL_INCLUDE_DIR}/object_model_map_wrapper.h
)

set(SORT_FILTER_PROXY_MODEL
    ${OBJECT_MODEL_INCLUDE_DIR}/sort_filter_proxy_model.h
        src/sort_filter_proxy_model.cpp
    ${OBJECT_MODEL_INCLUDE_DIR}/abstract_filter_comparator_base.h
        src/abstract_filter_comparator_base.cpp
    ${OBJECT_MODEL_INCLUDE_DIR}/abstract_comparator.h
        src/abstract_comparator.cpp
    ${OBJECT_MODEL_INCLUDE_DIR}/comparator.h
        src/comparator.cpp
    ${OBJECT_MODEL_INCLUDE_DIR}/comparator_group.h
        src/comparator_group.cpp
    ${OBJECT_MODEL_INCLUDE_DIR}/abstract_filter.h
        src/abstract_filter.cpp
    ${OBJECT_MODEL_INCLUDE_DIR}/filter_group.h
        src/filter_group.cpp
    ${OBJECT_MODEL_INCLUDE_DIR}/comparison_filter.h
        src/comparison_filter.cpp
    ${OBJECT_MODEL_INCLUDE_DIR}/regular_expression_filter.h
        src/regular_expression_filter.cpp
    ${OBJECT_MODEL_INCLUDE_DIR}/range_filter.h
        src/range_filter.cpp
    ${OBJECT_MODEL_INCLUDE_DIR}/enumeration_filter.h
        src/enumeration_filter.cpp
    ${OBJECT_MODEL_INCLUDE_DIR}/null_object_filter.h
        src/null_object_filter.cpp
    ${OBJECT_MODEL_INCLUDE_DIR}/value_list_comparator.h
        src/value_list_comparator.cpp
    ${OBJECT_MODEL_INCLUDE_DIR}/object_list_comparator.h
        src/object_list_comparator.cpp
)

set(OBJECT_SETTINGS
    ${OBJECT_MODEL_INCLUDE_DIR}/abstract_settings.h
        src/abstract_settings.cpp
    ${OBJECT_MODEL_INCLUDE_DIR}/settings.h
        src/settings.cpp
)

source_group("sort_filter_proxy_model" FILES ${SORT_FILTER_PROXY_MODEL})
source_group("meta_data" FILES ${META_DATA})
source_group("object_model" FILES ${OBJECT_MODEL})
source_group("object_stream" FILES ${OBJECT_STREAM})
source_group("object_settings" FILES ${OBJECT_SETTINGS})
source_group("model_access" FILES ${MODEL_ACCESS})

add_library(object_modellib ${OBJECT_MODEL} ${META_DATA} ${SORT_FILTER_PROXY_MODEL} ${OBJECT_STREAM} ${OBJECT_SETTINGS} ${MODEL_ACCESS})
target_include_directories(object_modellib PRIVATE ${CMAKE_CURRENT_LIST_DIR})
target_include_directories(object_modellib PRIVATE ${OBJECT_MODEL_INCLUDE_DIR})
target_include_directories(object_modellib PUBLIC ${OBJECT_MODEL_ROOT_DIR}/include)

#NttUseConanLibOrSubmodule(log)
target_link_libraries(object_modellib PUBLIC Qt5::Gui Qt5::Qml)
#target_link_libraries(object_modellib PUBLIC ${loglib})

# В solution эта библиотека лежит в modules
set_property(TARGET object_modellib PROPERTY FOLDER "gui")
