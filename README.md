# Модуль для генерирования прото-библиотек
Набор функций, генерирующих библиотеки для работы с прото-файлами

Protocol Buffers to Qt Generator автоматически генерирует Qt-совместимые C++ классы из .proto файлов, решая проблему интеграции между Protocol Buffers и Qt фреймворком. Библиотека устраняет необходимость ручного создания промежуточных адаптеров, которые требуются для обеспечения совместимости между классами сериализации protobuf и QObject-наследниками с поддержкой Q_PROPERTY и QML.

Инструмент анализирует .proto файлы и генерирует соответствующие .h/.cpp файлы с полнофункциональными Qt-классами, полностью интегрируясь с системой сборки CMake для автоматического отслеживания изменений. Библиотека эффективно применяется в встраиваемых системах, мобильных приложениях, индустриальном ПО и финтех-решениях, где требуется интеграция структурированных данных с пользовательскими интерфейсами. Особенно ценна для крупных проектов, где ручная реализация адаптеров может потребовать десятки тысяч строк кода.

Зависимости: protobuf, Qml, ObjectModel
## TL;DR
Пример использования функций для генерации прото-библиотек
```C++
/*CMakeLists.txt*/

set(_NTT_PROTOBUF_GENERATE_QT_MODULE_DIR "${CMAKE_CURRENT_LIST_DIR}")

set(PROTO_FILES
    first_message.proto
    second_message.proto
)

set(QT_PROTO_FILES
    data_interchange.proto
)

// Указываем, что нужно искать прото-файл в текущей директории
set(Protobuf_IMPORT_DIRS ${Protobuf_IMPORT_DIRS} ".")

// Генерируем c++ (pb.h/pb.cc) библиотеку для взаимодействия с прото-файлами, генерируем GRPC
add_protobuf_generated_library(LIB_NAME message_lib PROTO_FILES ${PROTO_FILES} GENERATE_GRPC) 

// Генерируем qt-c++ (qt_pb.h/qt_pb.cc) библиотеку для взаимодействия qt с прото-файлами, не генерируем GRPC 
add_protobuf_generated_library_with_qt(LIB_NAME interchange_protocollib QT_LIB_NAME interchange_protocollib_qt PROTO_FILES ${QT_PROTO_FILES})

/*cplusplus.cpp*/

#include "message_protocollib.pb.h"

/*qt.cpp*/

#include "interchange_protocollib_qt_pb.h"

/*линковка*/

target_link_libraries(cplusplus PUBLIC message_protocollib)
target_link_libraries(qt PUBLIC interchange_protocollib)
```
