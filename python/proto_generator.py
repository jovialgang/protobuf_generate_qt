# Генератор C++ кода для интеграции protobuf с Qt
# Этот модуль отвечает за генерацию C++ классов, которые оборачивают protobuf сообщения
# в Qt-объекты с поддержкой сигналов, слотов и свойств

from typing import Tuple

import subprocess
import typing
import os
import functools
import sys
import collections
import shutil

import case_converter
import proto_generator_printer as printer
import proto_parser


class DataTypeModelGenerator:
    """
    Генератор моделей для повторяющихся типов данных
    Создает Qt-модели для работы с массивами protobuf типов
    """
    def __init__(self, repeated_data_types):
        self.repeated_data_types = repeated_data_types
        self.name = "data_type_models"
        self.includes = ("#include <cstdint>\n"
                         "#include <google/protobuf/repeated_field.h>\n")
        self.classes = ""
        self.initialize = ""
        self.definitions = ""
        self.implementations = ""
        self.namespace_name = "models"
        self.package_name = "Proto.Models"

    def run(self) -> Tuple[str, str]:
        """
        Запускает генерацию кода
        Returns:
            Tuple[str, str]: Содержимое .h и .cpp файлов
        """
        self.process_data_types()
        h_file = printer.print_h_file(self.name, self.namespace_name, self.includes, self.classes, self.definitions)
        cpp_file = printer.print_cpp_file(self.name, self.namespace_name, self.implementations, self.initialize)
        return h_file, cpp_file

    def process_data_types(self) -> None:
        """
        Обрабатывает все повторяющиеся типы данных
        Для каждого типа создает Qt-модель с методами доступа
        """
        for data_type in self.repeated_data_types:
            type_name = case_converter.to_pascal(data_type + "Model")
            repeated_field_type = "google::protobuf::RepeatedPtrField" if data_type in proto_parser.string_types else "google::protobuf::RepeatedField"
            cpp_data_type = proto_parser.cpp_type_names[data_type]
            qt_type = proto_parser.qt_type_names[data_type]
            get_val = printer.print_model_get_string_val() if data_type in proto_parser.string_types else printer.print_model_get_val() if cpp_data_type == qt_type else \
                printer.print_model_get_cast_val(qt_type)
            set_val = printer.print_model_set_string_val() if data_type in proto_parser.string_types else printer.print_model_set_val(
                cpp_data_type) if cpp_data_type == qt_type else \
                printer.print_model_set_cast_val(qt_type, cpp_data_type)

            properties = printer.print_read_only_property("int", "count", "rowCount", "rowCount", "")

            self.classes += printer.print_forward_class_definition(type_name)
            self.definitions += printer.print_model_class_h(type_name, cpp_data_type, repeated_field_type, properties)
            self.implementations += printer.print_model_class_cpp(type_name, cpp_data_type, repeated_field_type, get_val, set_val)
            self.initialize += printer.print_qml_register_uncreatable_type(type_name, self.package_name)


class GeneratorQtObjectFile:
    """
    Генератор Qt-объектов из protobuf файла
    Создает C++ классы, которые оборачивают protobuf сообщения в Qt-объекты
    """
    def __init__(self, file):
        self.file = file
        self.name = file.name.replace(".proto", "")
        self.includes = ""
        self.initialize = ""
        self.classes = ""
        self.using_enums = ""
        self.definitions = ""
        self.implementations = ""
        self.package_name = ".".join([case_converter.to_pascal(name) for name in file.package.split(".")])
        self.namespace_name = file.namespace_name()

    def run(self) -> Tuple[str, str]:
        """
        Запускает генерацию кода
        Returns:
            Tuple[str, str]: Содержимое .h и .cpp файлов
        """
        self.process_enums()
        self.process_messages()
        self.prepare()
        return (printer.print_h_file(self.name, self.namespace_name, self.includes, self.classes, self.definitions),
                printer.print_cpp_file(self.name, self.namespace_name, self.implementations, self.initialize))

    def process_enums(self) -> None:
        """
        Обрабатывает все перечисления из protobuf файла
        Создает Qt-классы для работы с enum
        """
        for enum in self.file.enums.values():
            generated_enum = generate_qt_enum_type(enum)
            enum.qt_full_name = f"{generated_enum[0]}::Enum"
            self.initialize += printer.print_qml_register_metatype(generated_enum[0], self.package_name)
            self.definitions += generated_enum[1]

            if enum.has_repeated_instances:
                generated_model = generate_qt_enum_model_type(enum)
                self.initialize += printer.print_qml_register_uncreatable_type(generated_model[0], self.package_name)
                self.classes += printer.print_forward_class_definition(generated_model[0])
                self.definitions += generated_model[1]
                self.implementations += generated_model[2]

    def process_messages(self) -> None:
        """
        Обрабатывает все сообщения из protobuf файла
        Создает Qt-классы для работы с сообщениями
        """
        for message in self.file.messages.values():
            generator_qt_message_object_type = GeneratorQtMessageObjectType(message)
            generate_object = generator_qt_message_object_type.run()
            self.initialize += printer.print_qml_register_type(generate_object[0], self.package_name)
            self.classes += printer.print_forward_class_definition(generate_object[0])
            self.definitions += generate_object[1]
            self.implementations += generate_object[2]
            if message.has_repeated_instances:
                generated_model = generate_qt_message_model_type(message)
                self.initialize += printer.print_qml_register_uncreatable_type(generated_model[0], self.package_name)
                self.classes += printer.print_forward_class_definition(generated_model[0])
                self.definitions += generated_model[1]
                self.implementations += generated_model[2]

    def prepare(self) -> None:
        """
        Подготавливает включения и инициализацию
        Добавляет необходимые заголовочные файлы и регистрацию типов в QML
        """
        if self.file.is_well_known_types:
            self.includes = printer.WELL_KNOWN_TYPES_INCLUDES
        else:
            self.includes = printer.print_include(self.file.name.replace(".proto", ".pb.h"))

        self.includes += printer.print_include("data_type_models_qt_pb.h")
        if not self.file.is_well_known_types:
            self.includes += printer.print_include("well_known_types_qt_pb.h")

        self.initialize += printer.print_qml_register_file("data_type_models_qt_pb")
        self.initialize += printer.print_qml_register_file("well_known_types_qt_pb")
        for include in self.file.imports:
            if "google/" not in include:
                self.includes += printer.print_include(include.replace(".proto", "_qt_pb.h"))
                self.initialize += printer.print_qml_register_file(include.replace(".proto", "_qt_pb"))


def get_clang_format_paths() -> typing.Tuple[str, ...]:
    """
    Возвращает список путей для поиска clang-format
    Returns:
        typle[str, ...]: Пути по которым можно найти clang-format
    """
    clang_format_paths = (
        "clang-format",
        "C:/Program Files (x86)/Microsoft Visual Studio/2019/Community/VC/Tools/Llvm/bin/clang-format.exe",
        "C:/Program Files (x86)/Microsoft Visual Studio/2019/Community/VC/Tools/Llvm/x64/bin/clang-format.exe",
        "C:/Program Files (x86)/Microsoft Visual Studio/2019/Enterprise/VC/Tools/Llvm/bin/clang-format.exe",
        "C:/Program Files (x86)/Microsoft Visual Studio/2019/Enterprise/VC/Tools/Llvm/x64/bin/clang-format.exe",
    )
    return clang_format_paths


def find_clang_format_impl(clang_format_paths: typing.List[str]) -> typing.Optional[str]:
    """
    Ищет исполняемый файл clang-format по указанным путям
    Args:
        clang_format_paths: Список путей для поиска

    Returns:
        typing.Optional[str]: Путь к найденному clang-format или None
    """
    system_path = shutil.which("clang-format")
    if system_path is not None:
        return system_path

    for clang_format_path in clang_format_paths:
        if os.path.exists(clang_format_path):
            return clang_format_path


"""
Для версий Python до 3.9 functools.cache отсутствует
"""
if sys.version_info.major == 3 and sys.version_info.minor > 8:
    @functools.cache
    def find_clang_format(clang_format_paths: typing.List[str]) -> typing.Optional[str]:
        return find_clang_format_impl(clang_format_paths)
else:
    def find_clang_format(clang_format_paths: typing.List[str]) -> typing.Optional[str]:
        return find_clang_format_impl(clang_format_paths)


def run_clang_format(clang_format_args: typing.List[str]) -> None:
    """
    Запускает clang-format для форматирования сгенерированных файлов
    Args:
        clang_format_args: Аргументы для запуска clang-format
    """
    if not len(clang_format_args):
        print("Length of clang format args is ZERO")
        return

    clang_format_path = find_clang_format(get_clang_format_paths())
    if clang_format_path:
        clang_format_args[0] = clang_format_path
        try:
            subprocess.run(clang_format_args)
        except FileNotFoundError:
            print(f"Can not run clang-format by path: {clang_format_path}")
        except Exception as e:
            # Не делаем трагедии из проблем с форматированием
            print(f"{clang_format_path} failed with an error: {e}")
    else:
        print("Can not find clang format")


def save_generated_file(file_path: str, content: str, encoding: str = "UTF8") -> None:
    """
    Сохраняет сгенерированный код в файл
    Args:
        file_path: Путь к файлу для сохранения
        content: Содержимое файла
        encoding: Кодировка файла
    """
    with open(file_path, "w", encoding=encoding) as file:
        file.write(content)


def generate_qt_object_files(parser: proto_parser.Parser, file_paths: typing.List[str], save_path: str) -> None:
    """
    Генерирует Qt-объекты из protobuf файлов
    Args:
        parser: Парсер protobuf файлов
        file_paths: Список путей к .proto файлам
        save_path: Путь для сохранения сгенерированных файлов
    """
    clang_format_args = ["clang-format", "-i"]

    if not save_path:
        return

    for file_path in file_paths:
        file = parser.files[file_path]
        generator_qt_object = GeneratorQtObjectFile(file)
        gen_header_content, gen_source_content = generator_qt_object.run()
        file_name = file.name.replace(".proto", "_qt_pb")

        h_file_path = f"{save_path}/{file_name}.h"
        cpp_file_path = f"{save_path}/{file_name}.cpp"

        clang_format_args.append(h_file_path)
        clang_format_args.append(cpp_file_path)

        save_generated_file(h_file_path, gen_header_content)
        save_generated_file(cpp_file_path, gen_source_content)

    run_clang_format(clang_format_args)


def generate_data_type_models(save_path: str, repeated_data_types: typing.Set[str]) -> None:
    """
    Генерирует модели для повторяющихся типов данных
    Args:
        save_path: Путь для сохранения сгенерированных файлов
        repeated_data_types: Множество типов данных, которые могут повторяться
    """
    generator_data_type_models = DataTypeModelGenerator(repeated_data_types)
    h_file, cpp_file = generator_data_type_models.run()

    if save_path:
        h_file_path = f"{save_path}/{generator_data_type_models.name}_qt_pb.h"
        cpp_file_path = f"{save_path}/{generator_data_type_models.name}_qt_pb.cpp"
        clang_format_args = ["clang-format", "-i", h_file_path, cpp_file_path]

        save_generated_file(h_file_path, h_file)
        save_generated_file(cpp_file_path, cpp_file)

        run_clang_format(clang_format_args)


def generate_qt_enum_type(enum: proto_parser.Enum) -> typing.Tuple[str, str]:
    type_name = f"{case_converter.to_pascal(enum.class_name())}Enum"
    values = ""
    for value in enum.values:
        name = f"{enum.class_name()}_{value}" if enum.message else value
        values += printer.print_enum_value(enum.cpp_name(), name, case_converter.to_upper_snake(value))
    values = values[:-2]
    return type_name, printer.print_enum_namespace(type_name, values)


def generate_qt_enum_model_type(enum: proto_parser.Enum) -> typing.Tuple[str, str, str]:
    type_name = f"{case_converter.to_pascal(enum.class_name())}Model"
    data_type = "int"
    qt_type = f"{case_converter.to_pascal(enum.class_name())}Enum::Enum"

    get_val = printer.print_model_get_cast_val(qt_type)
    set_val = printer.print_model_set_cast_val(qt_type, data_type)

    return (type_name,
            printer.print_model_class_h(type_name, data_type, "google::protobuf::RepeatedField"),
            printer.print_model_class_cpp(type_name, data_type, "google::protobuf::RepeatedField", get_val, set_val))


def make_field_type_name(field_: proto_parser.Field, class_type_name_: str) -> str:
    """
    Создает имя типа поля в формате C++
    Args:
        field_: Поле protobuf
        class_type_name_: Имя класса типа
    Returns:
        str: Имя типа поля
    """
    return (f"{field_.type_namespace_name()}{case_converter.to_pascal(class_type_name_)}"
            f"{'Model' if field_.is_repeated() else 'Object'}")


def make_field_type_name_ptr(field_type_name_: str) -> str:
    """
    Создает имя указателя на тип поля
    Args:
        field_type_name_: Имя типа поля
    Returns:
        str: Имя указателя на тип поля
    """
    return f"{field_type_name_}*"


def make_field_variable_name(field_: proto_parser.Field) -> str:
    """
    Создает имя переменной поля
    Args:
        field_: Поле protobuf
    Returns:
        str: Имя переменной поля
    """
    return f"{field_.name}_"


def make_proto_data_type(field_: proto_parser.Field) -> str:
    """
    Создает тип данных protobuf для поля
    Args:
        field_: Поле protobuf
    Returns:
        str: Тип данных protobuf
    """
    if field_.is_repeated():
        return (f"{'google::protobuf::RepeatedField' if field_.is_of_enum_type() or field_.is_of_primitive_type() else 'google::protobuf::RepeatedPtrField'}"
                f"<{'int' if field_.is_of_enum_type() else field_.cpp_type_name()}>")
    else:
        return field_.cpp_type_name()


class GeneratorQtMessageObjectType:
    """
    Генератор Qt-объектов для protobuf сообщений
    Создает C++ классы, которые оборачивают protobuf сообщения в Qt-объекты
    """
    def __init__(self, message: proto_parser.Message):
        """
        Инициализирует генератор
        Args:
            message: Protobuf сообщение
        """
        self.message = message
        self.type_name = f"{case_converter.to_pascal(message.class_name())}Object"
        self.members = ""
        self.function_definitions = ""
        self.slot_function_definitions = ""  # используется в one of
        self.private_function_definitions = ""  # используется в one of
        self.function_implementations = ""
        self.notifiers = ""
        self.properties = ""
        self.initialize = ""
        self.sync_members = ""
        self.sync_signals = ""
        self.enums = ""
        self.check_members = ""
        self.check_signals = "\n\n"
        self.property_id = 0
        self.using = ""

        self.fields = sorted(list(message.fields.values()))

    def run(self) -> Tuple[str, typing.Any, typing.Any]:
        """
        Запускает генерацию кода
        Returns:
            Tuple[str, typing.Any, typing.Any]: Имя типа, определения и реализации
        """
        self.process_fields()
        self.process_one_ofs()
        return (
            self.type_name,
            printer.print_object_class_h(
                    self.type_name,
                    self.message.cpp_name(),
                    self.properties,
                    self.enums,
                    self.function_definitions,
                    self.private_function_definitions,
                    self.slot_function_definitions,
                    self.notifiers,
                    self.members,
                    self.property_id,
                    self.using
                ),
            printer.print_object_class_cpp(
                    self.type_name,
                    self.message.cpp_name(),
                    self.sync_members,
                    self.sync_signals,
                    self.check_members + self.check_signals,
                    self.initialize,
                    self.function_implementations
            )
        )

    def prepare_process_object(self, field, function_name, field_type_name_ptr, field_variable_name, proto_data_lvalue, proto_data_rvalue):
        """
        Подготавливает обработку объекта
        Args:
            field: Поле protobuf
            function_name: Имя функции
            field_type_name_ptr: Имя указателя на тип поля
            field_variable_name: Имя переменной поля
            proto_data_lvalue: Тип данных protobuf для lvalue
            proto_data_rvalue: Тип данных protobuf для rvalue
        """
        self.members += printer.print_member(field_type_name_ptr, field_variable_name, "nullptr", "mutable")

        if field.is_repeated() and not field.is_of_message_type():
            self.sync_members += printer.print_repeated_sync(field.name, field_variable_name)
        else:
            self.sync_members += printer.print_object_sync(field.name, field_variable_name)
            self.check_members += printer.print_object_check(field.name, field_variable_name)

        self.function_definitions += printer.print_getter_h(field_type_name_ptr, function_name)
        self.function_definitions += printer.print_setter_h(proto_data_lvalue, function_name)
        self.function_definitions += printer.print_setter_h(proto_data_rvalue, function_name)

    def process_one_of_object(self, field, field_variable_name, function_name, signal_name):
        """
        Обрабатывает объект oneof
        Args:
            field: Поле protobuf
            field_variable_name: Имя переменной поля
            function_name: Имя функции
            signal_name: Имя сигнала
        Returns:
            Tuple[str, str]: Значения для установки lvalue и rvalue
        """
        one_of_name = case_converter.to_pascal(field.one_of.name)
        enum_name = case_converter.to_upper_snake(field.name)

        self.properties += printer.print_read_only_property(
            "bool",
            "has" + function_name,
            "Has" + function_name,
            signal_name,
            ""
        )

        lvalue_set_val = printer.print_set_one_of_object(
            one_of_name,
            signal_name,
            enum_name,
            printer.print_set_object_lvalue(field.name, field_variable_name)
        )
        rvalue_set_val = printer.print_set_one_of_object(
            one_of_name,
            signal_name,
            enum_name,
            printer.print_set_object_rvalue(field.name, field_variable_name)
        )
        return lvalue_set_val, rvalue_set_val

    def process_optional_object(self, field, field_variable_name, function_name, signal_name):
        """
        Обрабатывает опциональный объект
        Args:
            field: Поле protobuf
            field_variable_name: Имя переменной поля
            function_name: Имя функции
            signal_name: Имя сигнала
        Returns:
            Tuple[str, str]: Значения для установки lvalue и rvalue
        """
        self.sync_signals += printer.print_val_sync(signal_name, self.property_id)
        self.check_signals += printer.print_optional_check(field.name, self.property_id)
        self.initialize += printer.print_changed_connect(f"{signal_name}Changed", self.type_name, "this",
                                                         self.type_name)
        self.notifiers += printer.print_notifier(signal_name)
        self.property_id += 1

        self.properties += printer.print_property(
            "bool",
            f"has{function_name}",
            f"Has{function_name}",
            signal_name,
            ""
        )
        self.function_definitions += printer.print_setter_h("bool", f"Has{function_name}")
        self.function_implementations += printer.print_setter_cpp(
            "bool",
            self.type_name,
            f"Has{function_name}",
            printer.print_set_optional_object_case(field.name, function_name, signal_name,
                                                   field_variable_name)
        )
        lvalue_set_val = printer.print_set_optional_object(
            function_name,
            signal_name,
            printer.print_set_object_lvalue(field.name, field_variable_name)
        )
        rvalue_set_val = printer.print_set_optional_object(
            function_name,
            signal_name,
            printer.print_set_object_rvalue(field.name, field_variable_name)
        )
        return lvalue_set_val, rvalue_set_val

    def process_one_of_and_optionals_object(self, field, field_variable_name, field_type_name, field_type_name_ptr, function_name, signal_name, property_name):
        """
        Обрабатывает объект oneof или опциональный объект
        Args:
            field: Поле protobuf
            field_variable_name: Имя переменной поля
            field_type_name: Имя типа поля
            field_type_name_ptr: Имя указателя на тип поля
            function_name: Имя функции
            signal_name: Имя сигнала
            property_name: Имя свойства
        Returns:
            Tuple[str, str]: Значения для установки lvalue и rvalue
        """
        lvalue_set_val = ""
        rvalue_set_val = ""

        self.properties += printer.print_read_only_property(
            field_type_name_ptr,
            property_name,
            function_name,
            signal_name
        )

        self.function_implementations += printer.print_getter_cpp(
            field_type_name_ptr,
            self.type_name,
            function_name,
            printer.print_get_optional_object(field.name, field_variable_name, field_type_name, "changed",
                                              self.type_name, "nullptr")
            # pointer will be nullptr if one of is in different case
        )

        self.function_definitions += printer.print_getter_h("bool", f"Has{function_name}", "")
        self.function_implementations += printer.print_getter_cpp(
            "bool",
            self.type_name,
            f"Has{function_name}",
            printer.print_return(printer.print_get_val(f"has_{field.name}")),
            ""
        )

        # ONE_OF OBJECTS
        if field.one_of:
            lvalue_set_val, rvalue_set_val = self.process_one_of_object(field, field_variable_name, function_name,
                                                                        signal_name)
        # OPTIONAL OBJECTS
        else:
            lvalue_set_val, rvalue_set_val = self.process_optional_object(field, field_variable_name, function_name,
                                                                          signal_name)
        return lvalue_set_val, rvalue_set_val

    def process_object(self, field, function_name, property_name, class_type_name, signal_name):
        """
        Обрабатывает объект
        Args:
            field: Поле protobuf
            function_name: Имя функции
            property_name: Имя свойства
            class_type_name: Имя класса типа
            signal_name: Имя сигнала
        """
        field_type_name = make_field_type_name(field, class_type_name)
        field_type_name_ptr = make_field_type_name_ptr(field_type_name)
        field_variable_name = make_field_variable_name(field)
        lvalue_set_val = ""
        rvalue_set_val = ""

        proto_data_type = make_proto_data_type(field)
        proto_data_lvalue = f"const {proto_data_type}&"
        proto_data_rvalue = f"{proto_data_type}&&"

        self.prepare_process_object(field, function_name, field_type_name_ptr, field_variable_name, proto_data_lvalue,
                                    proto_data_rvalue)

        # ONE_OF AND OPTIONAL OBJECTS
        if field.one_of or field.is_optional():
            lvalue_set_val, rvalue_set_val = self.process_one_of_and_optionals_object(
                field, field_variable_name, field_type_name, field_type_name_ptr, function_name, signal_name,
                property_name)
        # SIMPLE OBJECTS
        else:
            self.properties += printer.print_const_property(field_type_name_ptr, property_name, function_name)

            self.function_implementations += printer.print_getter_cpp(
                field_type_name_ptr,
                self.type_name,
                function_name,
                printer.print_get_object(field.name, field_variable_name, field_type_name, "changed", self.type_name)
            )

            lvalue_set_val = printer.print_set_object_lvalue(field.name, field_variable_name)
            rvalue_set_val = printer.print_set_object_rvalue(field.name, field_variable_name)

        self.function_implementations += printer.print_setter_cpp(
            proto_data_lvalue,
            self.type_name,
            function_name,
            lvalue_set_val
        )
        self.function_implementations += printer.print_setter_cpp(
            proto_data_rvalue,
            self.type_name,
            function_name,
            rvalue_set_val
        )

    def process_one_of_and_optionals_value(self, field, field_type_name, function_name, signal_name, updates):
        """
        Обрабатывает значение oneof или опциональное значение
        Args:
            field: Поле protobuf
            field_type_name: Имя типа поля
            function_name: Имя функции
            signal_name: Имя сигнала
            updates: Строка с обновлениями
        """
        one_of_name = case_converter.to_pascal(field.one_of.name)
        enum_name = case_converter.to_upper_snake(field.name)

        self.properties += printer.print_read_only_property(
            "bool",
            f"has{function_name}",
            f"Has{function_name}",
            signal_name,
            ""
        )

        self.function_implementations += printer.print_setter_cpp(
            field_type_name,
            self.type_name,
            function_name,
            printer.print_set_one_of_val(function_name, signal_name, one_of_name, enum_name, updates)
        )

        if field_type_name == proto_parser.qt_type_names["string"]:
            self.function_implementations += printer.print_setter_cpp(
                proto_parser.cpp_type_names["string"],
                self.type_name,
                f"{function_name}Std",
                printer.print_set_one_of_val_string_std(
                    function_name,
                    signal_name,
                    one_of_name,
                    enum_name,
                    printer.print_set_val_std(field.name)
                )
            )

        if field.is_of_enum_type():
            self.function_implementations += printer.print_setter_cpp(
                field.get_type_enum().cpp_name(),
                self.type_name,
                f"{function_name}Std",
                printer.print_check_and_set_val(
                    function_name, signal_name, printer.print_set_val(field.name))
            )

    def process_optional_value(self, field, field_type_name, function_name, signal_name, cpp_default_value, updates):
        """
        Обрабатывает опциональное значение
        Args:
            field: Поле protobuf
            field_type_name: Имя типа поля
            function_name: Имя функции
            signal_name: Имя сигнала
            cpp_default_value: Значение по умолчанию в C++
            updates: Строка с обновлениями
        """
        self.sync_signals += printer.print_val_sync(signal_name, self.property_id)
        self.check_signals += printer.print_optional_check(field.name, self.property_id,
                                                           printer.print_optional_val_check(field.name))
        self.initialize += printer.print_changed_connect(f"{signal_name}Changed", self.type_name, "this",
                                                         self.type_name)
        self.notifiers += printer.print_notifier(signal_name)
        self.property_id += 1

        self.properties += printer.print_property(
            "bool",
            f"has{function_name}",
            f"Has{function_name}",
            signal_name,
            ""
        )

        self.function_definitions += printer.print_setter_h("bool", f"Has{function_name}")
        self.function_implementations += printer.print_setter_cpp(
            "bool",
            self.type_name,
            f"Has{function_name}",
            printer.print_set_optional_val_case(field.name, function_name, signal_name,
                                                cpp_default_value)
        )

        self.function_implementations += printer.print_setter_cpp(
            field_type_name,
            self.type_name,
            function_name,
            printer.print_set_optional_val(function_name, signal_name, updates)
        )

        if field_type_name == proto_parser.qt_type_names["string"]:
            self.function_implementations += printer.print_setter_cpp(
                proto_parser.cpp_type_names["string"],
                self.type_name,
                f"{function_name}Std",
                printer.print_set_optional_val_string_std(
                    function_name,
                    signal_name,
                    printer.print_set_val_std(field.name)
                )
            )

        if field.is_of_enum_type():
            self.function_implementations += printer.print_setter_cpp(
                field.get_type_enum().cpp_name(),
                self.type_name,
                f"{function_name}Std",
                printer.print_check_and_set_val(
                    function_name, signal_name, printer.print_set_val(field.name))
            )

    def prepare_process_value(self, field, class_type_name) -> Tuple[str, str, str]:
        """
        Подготавливает обработку значения
        Args:
            field: Поле protobuf
            class_type_name: Имя класса типа
        Returns:
            Tuple[str, str, str]: Имя типа поля, значение для получения, строка с обновлениями
        """
        if field.is_of_string_type():
            return field.qt_type_name(), printer.print_get_string_val(field.name), printer.print_set_string_val(field.name)
        elif field.is_of_primitive_type():
            return field.qt_type_name(), printer.print_get_val(field.name), printer.print_set_val(field.name)
        elif field.is_of_enum_type():
            field_type_name = (f"{field.type_namespace_name()}"
                               f"{case_converter.to_pascal(class_type_name.replace('.', '_'))}"
                               f"Enum::Enum")
            enum_type_name = field.qt_type_name()
            get_val = printer.print_get_cast_val(field_type_name, field.name)
            updates = printer.print_set_cast_val(enum_type_name, field.name)
            return field_type_name, get_val, updates

    def process_value(self, field, function_name, property_name, class_type_name, signal_name):
        """
        Обрабатывает значение
        Args:
            field: Поле protobuf
            function_name: Имя функции
            property_name: Имя свойства
            class_type_name: Имя класса типа
            signal_name: Имя сигнала
        """
        field_type_name, get_val, updates = self.prepare_process_value(field, class_type_name)
        self.properties += printer.print_property(
            field_type_name,
            property_name,
            function_name,
            signal_name
        )

        self.function_definitions += printer.print_getter_h(field_type_name, function_name)
        self.function_definitions += printer.print_setter_h(field_type_name, function_name)

        """
        Добавление setters для стандартных типов
        """
        if field_type_name == proto_parser.qt_type_names["string"]:
            self.function_definitions += printer.print_setter_h(
                proto_parser.cpp_type_names["string"], f"{function_name}Std")

        if field.is_of_enum_type():
            self.function_definitions += printer.print_setter_h(
                field.get_type_enum().cpp_name(), f"{function_name}Std"
            )

            # ONE_OF AND OPTIONAL VALUES
        if field.one_of or field.is_optional():
            cpp_default_value = (f"static_cast<{field.cpp_type_name()}>(0)"
                                 if field.is_of_enum_type()
                                 else proto_parser.default_values[field.type_name])

            default_value = (f"static_cast<{field.type_namespace_name()}"
                             f"{case_converter.to_pascal(class_type_name.replace('.', '_'))}Enum::Enum>(0)"
                             if field.is_of_enum_type()
                             else proto_parser.default_values[field.type_name])

            self.function_implementations += printer.print_getter_cpp(
                field_type_name,
                self.type_name,
                function_name,
                printer.print_return(printer.print_get_optional_val(field.name, get_val, default_value))
            )

            self.function_definitions += printer.print_getter_h("bool", f"Has{function_name}", "")
            self.function_implementations += printer.print_getter_cpp(
                "bool",
                self.type_name,
                f"Has{function_name}",
                printer.print_return(printer.print_get_val(f"has_{field.name}")),
                ""
            )

            # ONE_OF VALUES
            if field.one_of:
                self.process_one_of_and_optionals_value(field, field_type_name, function_name, signal_name, updates)
            # OPTIONAL VALUES
            else:
                self.process_optional_value(field, field_type_name, function_name, signal_name, cpp_default_value, updates)

        # SIMPLE VALUES
        else:
            self.sync_signals += printer.print_val_sync(property_name, self.property_id)
            self.check_signals += printer.print_val_check(field.name, self.property_id)
            self.initialize += printer.print_changed_connect(f"{property_name}Changed", self.type_name, "this",
                                                             self.type_name)
            self.notifiers += printer.print_notifier(property_name)
            self.property_id += 1

            self.function_implementations += printer.print_getter_cpp(field_type_name, self.type_name, function_name,
                                                                      printer.print_return(get_val))
            self.function_implementations += printer.print_setter_cpp(
                field_type_name,
                self.type_name,
                function_name,
                printer.print_check_and_set_val(function_name, property_name, updates)
            )
            if field_type_name == proto_parser.qt_type_names["string"]:
                self.function_implementations += printer.print_setter_cpp(
                    proto_parser.cpp_type_names["string"],
                    self.type_name,
                    f"{function_name}Std",
                    printer.print_check_and_set_val_string_std(
                        function_name,
                        property_name,
                        printer.print_set_string_val_std(field.name)
                    )
                )
            if field.is_of_enum_type():
                self.function_implementations += printer.print_setter_cpp(
                    field.get_type_enum().cpp_name(),
                    self.type_name,
                    f"{function_name}Std",
                    printer.print_check_and_set_val(
                        function_name, property_name, printer.print_set_val(field.name))
                )

    def process_fields(self):
        """
        Обрабатывает все поля сообщения
        """
        for enum_ in self.message.enums.values():
            self.using += printer.print_using_enums(case_converter.to_pascal(enum_.name), enum_.qt_full_name)

        for message_ in self.message.messages.values():
            self.using += printer.print_using_nested_message(message_.name, f"{message_.full_name().replace('.', '::')}")

        for field in self.fields:
            function_name = case_converter.to_pascal(field.name)
            property_name = case_converter.to_camel(field.name)
            class_type_name = field.class_type_name()
            signal_name = case_converter.to_camel(field.one_of.name) if field.one_of else property_name

            if not class_type_name:
                print(f"Can not find type: {field.type_name}")
            elif field.is_map():
                print(f"Can not parse map<{field.map_key_type}, {field.type_name}>")
            # OBJECTS
            elif field.is_repeated() or field.is_of_message_type():
                self.process_object(field, function_name, property_name, class_type_name, signal_name)
            # VALUES
            else:
                self.process_value(field, function_name, property_name, class_type_name, signal_name)
            self.function_implementations += "\n"
            self.function_definitions += "\n"

    def process_one_of_fields(self, one_of, enum_type_name):
        """
        Обрабатывает поля oneof
        Args:
            one_of: Oneof поле
            enum_type_name: Имя типа перечисления
        Returns:
            Tuple[str, str, str]: Значения перечисления, обновления, проверки
        """
        values = ""
        updates = ""
        one_of_checks = ""

        for field in one_of.fields:
            enum_name = case_converter.to_upper_snake(field.name)
            field_function_name = case_converter.to_pascal(field.name)

            values += printer.print_enum_value(enum_type_name, f"k{field_function_name}", enum_name)
            field_variable_name = f"{field.name}_"

            default_value = (f"static_cast<{field.cpp_type_name()}>(0)"
                             if field.is_of_enum_type()
                             else proto_parser.default_values[field.type_name] if field.is_of_scalar_type() else "nullptr")

            if field.is_of_message_type():
                updates += printer.print_one_of_object_update(field.name, enum_name, field_variable_name)
            else:
                updates += printer.print_one_of_val_update(
                    field.name,
                    enum_name,
                    default_value
                )
                one_of_checks += printer.print_optional_val_check(field.name)

        return values, updates, one_of_checks

    def process_one_ofs(self):
        """
        Обрабатывает все oneof поля сообщения
        """
        for one_of in self.message.one_ofs.values():
            function_name = f"{case_converter.to_pascal(one_of.name)}Case"
            property_name = f"{case_converter.to_camel(one_of.name)}Case"
            signal_name = case_converter.to_camel(one_of.name)
            not_set_enum = f"{one_of.name.upper()}_NOT_SET"
            not_set_qt_enum = f"{case_converter.to_upper_snake(one_of.name)}_NOT_SET"
            name = f"{one_of.name}_case"
            get_val = printer.print_get_cast_val(function_name, name)
            enum_type_name = f"{one_of.message.cpp_name()}::{function_name}"

            values, updates, one_of_checks = self.process_one_of_fields(one_of, enum_type_name)

            values += printer.print_enum_value(enum_type_name, not_set_enum, not_set_qt_enum)
            values = values[:-2]
            self.enums += printer.print_enum(function_name, values)

            self.sync_signals += printer.print_val_sync(signal_name, self.property_id)
            self.check_signals += printer.print_one_of_check(name, one_of_checks, self.property_id)
            self.initialize += printer.print_changed_connect(f"{signal_name}Changed", self.type_name, "this", self.type_name)
            self.notifiers += printer.print_notifier(signal_name)
            self.property_id += 1

            self.private_function_definitions += printer.print_function_h(
                f"Update{function_name}",
                "bool",
                f"{self.type_name}::{function_name} val"
            )
            self.function_implementations += printer.print_function_cpp(
                self.type_name,
                f"Update{function_name}",
                printer.print_update_one_of_case(
                    one_of.name,
                    function_name,
                    f"{enum_type_name}::{not_set_enum}",
                    updates
                ),
                "bool",
                f"{self.type_name}::{function_name} val"
            )

            self.function_definitions += printer.print_getter_h(function_name, function_name)
            self.function_implementations += printer.print_getter_cpp(
                f"{self.type_name}::{function_name}",
                self.type_name,
                function_name,
                printer.print_return(get_val)
            )

            self.function_definitions += printer.print_setter_h(function_name, function_name)
            self.function_implementations += printer.print_setter_cpp(
                f"{self.type_name}::{function_name}",
                self.type_name,
                function_name,
                printer.print_set_one_of_case(function_name, signal_name)
            )

            self.slot_function_definitions += printer.print_function_h(f"Clear{function_name}")
            self.function_implementations += printer.print_function_cpp(
                self.type_name,
                f"Clear{function_name}",
                f"Set{function_name}({not_set_qt_enum});"
            )

            self.properties += printer.print_property(
                function_name,
                property_name,
                function_name,
                signal_name,
                "Get",
                f"RESET Clear{function_name}"
            )

            self.function_implementations += "\n"
            self.function_definitions += "\n"


def generate_qt_message_model_type(message: proto_parser.Message) -> typing.Tuple[str, str, str]:
    """
    Генерирует тип модели для сообщения
    Args:
        message: Protobuf сообщение
    Returns:
        Tuple[str, str, str]: Имя типа, определения и реализации
    """
    name = case_converter.to_pascal(message.class_name())
    type_name = f"{name}Model"
    message_type_name = f"{message.namespace_name()}{name}Object"
    data_type = message.cpp_name()

    return (type_name,
            printer.print_object_model_class_h(type_name, data_type, message_type_name),
            printer.print_object_model_class_cpp(type_name, data_type, message_type_name))
