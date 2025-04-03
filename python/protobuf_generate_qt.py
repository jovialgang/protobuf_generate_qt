"""
Скрипт для генерации C++ кода с поддержкой Qt из protobuf файлов.
Этот скрипт является точкой входа для генерации Qt-объектов из .proto файлов.
Он обрабатывает аргументы командной строки, собирает все необходимые файлы
и запускает процесс генерации кода.
"""

import proto_parser
import proto_generator

import argparse
import sys
import os


if __name__ == '__main__':
    # Создаем парсер аргументов командной строки
    arg_parser = argparse.ArgumentParser()
    # Список .proto файлов для обработки
    arg_parser.add_argument("-proto_files", action="store", dest="proto_files", default=[], nargs="*")
    # Список директорий для поиска импортируемых .proto файлов
    arg_parser.add_argument("-import_dirs", action="store", dest="import_dirs", default=[], nargs="*")
    # Путь для сохранения сгенерированных файлов
    arg_parser.add_argument("-output", action="store", dest="output_path", default="", nargs=1)

    # Пример аргументов для тестирования (закомментирован)
    #in_args = ['-proto_files', 'D:/work/qml_modules_tester/tester/proto/ip_endpoint.proto', 'D:/work/qml_modules_tester/tester/proto/coordinates.proto', 'D:/work/qml_modules_tester/tester/proto/property_state.proto', 'D:/work/qml_modules_tester/tester/proto/random_access_channel_events.proto', 'D:/work/qml_modules_tester/tester/proto/severity.proto', 'D:/work/qml_modules_tester/tester/proto/spotbeam.proto', 'D:/work/qml_modules_tester/tester/proto/thuraya_events.proto', '-import_dirs', 'D:/work/qml_modules_tester/tester/proto', '-output', 'D:/work/qml_modules_tester/output_64/tester/proto'];
    # Получаем аргументы из командной строки
    in_args = sys.argv[1:]
    args = arg_parser.parse_args(in_args)

    print("Running protobuf generate Qt")
    print(sys.argv[1:])

    # Нормализуем пути к .proto файлам
    proto_files = [os.path.normpath(path) for path in args.proto_files]

    # Список дополнительных .proto файлов из директорий импорта
    import_files = []

    # Собираем все .proto файлы из директорий импорта
    for import_dir in args.import_dirs:
        for file in os.listdir(import_dir):
            if file.endswith(".proto"):
                file_path = os.path.normpath(os.path.join(import_dir, file))
                if file_path not in proto_files:
                    import_files.append(file_path)

    # Нормализуем путь для сохранения сгенерированных файлов
    output_path = os.path.normpath(args.output_path[0])

    # Создаем парсер protobuf и запускаем генерацию кода
    proto_parser = proto_parser.Parser(proto_files + [proto_parser.get_well_known_types_path()] + import_files)
    proto_generator.generate_qt_object_files(proto_parser, proto_files, output_path)
