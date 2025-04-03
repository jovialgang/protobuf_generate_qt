"""
Модуль для парсинга .proto файлов и создания структур данных для генерации C++ кода с Qt поддержкой.
Этот модуль отвечает за:
1. Чтение и разбор .proto файлов
2. Создание иерархической структуры данных, отражающей структуру protobuf сообщений
3. Обработку всех типов полей (простые, повторяющиеся, опциональные, map, oneof)
4. Поддержку вложенных сообщений и перечислений
"""

from __future__ import annotations

import os
import re
import collections

from enum import Enum
from typing import List, Dict, Tuple, Optional

# Регулярные выражения для разбора .proto файлов
field_number_re_str = r'0x[0-9a-fA-F]+|[0-9]+'

file_name_re = re.compile(r'[^\\/]+$')
comment_re = re.compile(r'(//.*\n)|(/\*[^*]*\*/)')
message_start_re = re.compile(r'message\s+(\w+)\s*{')
field_re = re.compile(fr'([\w\.]+)\s+(\w+)\s*=\s*({field_number_re_str})\s*;')
repeated_field_re = re.compile(fr'repeated\s+([\w\.]+)\s+(\w+)\s*=\s*({field_number_re_str})\s*;')
optional_field_re = re.compile(r'optional\s+([\w.]+)\s+(\w+)\s*=\s*(\d+)\s*;')
map_field_re = re.compile(fr'map<\s*(\w+)\s*,\s*([\w\.]+)\s*>\s+(\w+)\s*=\s*({field_number_re_str})\s*;')
enum_re = re.compile(r'enum\s+(\w+)\s*{([^}]*)}')
enum_values_re = re.compile(fr'(\w+)\s*=\s*({field_number_re_str})\s*;')
one_of_re = re.compile(r'oneof\s+(\w+)\s*{([^}]*)}')
syntax_re = re.compile(r'syntax\s*=\s*"(\w+)"\s*;')
package_re = re.compile(r'package\s+([\w.]+)\s*;')
import_re = re.compile(r'import\s+"([\w/.]+)"\s*;')

# Определение типов данных protobuf
primitive_types = {
    "double",
    "float",
    "int32",
    "int64",
    "uint32",
    "uint64",
    "bool",
}
string_types = {
    "string",
    "bytes",
}
data_types = set.union(primitive_types, string_types)

# Маппинг типов protobuf на типы Qt
qt_type_names = {
    "double": "double",
    "float": "float",
    "int32": "int",
    "int64": "double",
    "uint32": "unsigned int",
    "uint64": "double",
    "bool": "bool",
    "string": "QString",
    "bytes": "QString"
}

# Маппинг типов protobuf на типы C++
cpp_type_names = {
    "double": "double",
    "float": "float",
    "int32": "int",
    "int64": "int64_t",
    "uint32": "unsigned int",
    "uint64": "uint64_t",
    "bool": "bool",
    "string": "std::string",
    "bytes": "std::string"
}

# Значения по умолчанию для типов
default_values = {
    "double": "0",
    "float": "0",
    "int32": "0",
    "int64": "0",
    "uint32": "0",
    "uint64": "0",
    "bool": "false",
    "string": '""',
    "bytes": '""'
}


def get_string_common_start(s1: str, s2: str) -> str:
    """
    Находит общее начало двух строк
    Args:
        s1: Первая строка
        s2: Вторая строка
    Returns:
        str: Общее начало строк
    """
    for i in range(min(len(s1), len(s2))):
        if s1[i] != s2[i]:
            return s1[:i]
    return s1 if len(s1) < len(s2) else s2


def get_well_known_types_path() -> str:
    """
    Возвращает путь к файлу с известными типами protobuf
    Returns:
        str: Путь к файлу well_known_types.proto
    """
    return os.path.join(os.path.dirname(__file__), "well_known_types.proto")


class File:
    """
    Класс для хранения распарсенного *.proto файла
    Содержит информацию о пакете, импортах, перечислениях и сообщениях
    """
    def __init__(self, path: str, parser: Parser):
        self.parser: Parser = parser
        self.path: str = path
        self.name: str = file_name_re.search(path).group()
        self.syntax: str = ""
        self.package: str = ""
        self.imports: List[str] = []
        self.enums: Dict[str, Enum] = {}
        self.messages: Dict[str, Message] = {}
        self.is_well_known_types: bool = self.name == "well_known_types.proto"

        self.parsing_file(path)

    def parsing_file(self, path: str) -> None:
        """
        Разбирает .proto файл
        Args:
            path: Путь к файлу
        """
        text = self.read_file(path)
        text = comment_re.sub("", text)

        self.parse_syntax(text)
        self.parse_package(text)
        self.parse_imports(text)

        # parsing messages
        text = self.parse_messages(text)
        # parsing enums
        text = self.parse_enums(text)

    def read_file(self, path, encoding: str = "UTF8", mode: str = "r") -> str:
        """
        Читает содержимое файла
        Args:
            path: Путь к файлу
            encoding: Кодировка файла
            mode: Режим открытия файла
        Returns:
            str: Содержимое файла
        """
        with open(path, mode, encoding=encoding) as file:
            return file.read()

    def parse_syntax(self, text: str) -> None:
        """
        Разбирает объявление синтаксиса protobuf
        Args:
            text: Текст файла
        """
        syntax_match = syntax_re.search(text)
        if syntax_match:
            self.syntax = syntax_match.group(1)

    def parse_package(self, text: str) -> None:
        """
        Разбирает объявление пакета
        Args:
            text: Текст файла
        """
        package_match = package_re.search(text)
        if package_match:
            self.package = package_match.group(1)

    def parse_imports(self, text) -> None:
        """
        Разбирает импорты из других .proto файлов
        Args:
            text: Текст файла
        """
        for match in import_re.finditer(text):
            self.imports.append(match.group(1))
            
    def parse_messages(self, text: str, parent_message: Optional[Message] = None):
        """
        Разбирает сообщения из файла
        Args:
            text: Текст файла
            parent_message: Родительское сообщение (для вложенных сообщений)
        Returns:
            str: Оставшийся текст после разбора сообщений
        """
        MessageParsed = collections.namedtuple(
            "MessageParsed", ["name", "message_start", "content_start", "content_end"]
        )

        def parse_message() -> Optional[MessageParsed]:
            """
            Разбирает одно сообщение
            Returns:
                Optional[MessageParsed]: Информация о разобранном сообщении или None
            """
            message_start = message_start_re.search(text)
            if message_start is None:
                return None
            nest_level = 1
            string_pos = message_start.end() - 1
            while nest_level != 0:
                string_pos += 1
                if text[string_pos] == "{":
                    nest_level += 1
                if text[string_pos] == "}":
                    nest_level -= 1
            return MessageParsed(message_start.group(1), message_start.start(), message_start.end(), string_pos)

        message_match = parse_message()
        while message_match is not None:
            message = Message(
                message_match.name,
                text[message_match.content_start:message_match.content_end],
                self,
                parent_message
            )
            if parent_message:
                parent_message.messages[message.name] = message
            self.messages[message.full_name()] = message
            text = text[:message_match.message_start] + text[message_match.content_end + 1:]  # cut message from text
            message_match = parse_message()
        return text

    def parse_enums(self, text: str, parent_message=None):
        """
        Разбирает перечисления из файла
        Args:
            text: Текст файла
            parent_message: Родительское сообщение (для вложенных перечислений)
        Returns:
            str: Оставшийся текст после разбора перечислений
        """
        for match in enum_re.finditer(text):
            enum = Enum(match.group(1), match.group(2), self, parent_message)
            if parent_message:
                parent_message.enums[enum.name] = enum
            self.enums[enum.full_name()] = enum
        return enum_re.sub("", text)

    def find_message(self, name: str, prefix: str):
        """
        Ищет сообщение по имени
        Args:
            name: Имя сообщения
            prefix: Префикс для поиска
        Returns:
            Optional[Message]: Найденное сообщение или None
        """
        if name.startswith(self.package):
            if name in self.messages:
                return self.messages[name]

        while prefix:
            key = prefix + "." + name if prefix else name
            if key in self.messages:
                return self.messages[key]
            if len(prefix):
                prefix = prefix[:max(0, prefix.rfind("."))]
        return None

    def find_enum(self, name, prefix):
        """
        Ищет перечисление по имени
        Args:
            name: Имя перечисления
            prefix: Префикс для поиска
        Returns:
            Optional[Enum]: Найденное перечисление или None
        """
        if name.startswith(self.package):
            if name in self.enums:
                return self.enums[name]

        while prefix:
            key = prefix + "." + name if prefix else name
            if key in self.enums:
                return self.enums[key]
            if len(prefix):
                prefix = prefix[:max(0, prefix.rfind("."))]

        return None

    def namespace_name(self):
        """
        Возвращает имя пространства имен для C++
        Returns:
            str: Имя пространства имен
        """
        return self.package.replace(".", "::") if self.package else ""


class Message:
    """
    Класс, представляющий сообщение в .proto файле.
    Хранит информацию о:
    - Имени сообщения
    - Полях сообщения
    - Вложенных перечислениях
    - Вложенных сообщениях
    - Oneof полях
    """
    def __init__(self, name: str, text: str, file: File, message=None):
        """
        Инициализирует объект сообщения.
        
        Args:
            name: Имя сообщения
            text: Текст сообщения
            file: Файл, содержащий сообщение
            message: Родительское сообщение (для вложенных сообщений)
        """
        self.file: File = file
        self.message: Message = message
        self.name: str = name
        self.one_ofs: Dict[str, OneOf] = {}
        self.fields: Dict[str, Field] = {}
        self.enums: Dict[str, Enum] = {}
        self.messages: Dict[str, Message] = {}
        self.has_repeated_instances: bool = file.is_well_known_types

        # parsing messages
        text = file.parse_messages(text, self)

        # parsing enums
        text = file.parse_enums(text, self)

        # parsing one ofs
        text = self.parse_one_ofs(text)

        # parsing fields
        text = self.parse_fields(text)

    def parse_one_ofs(self, text):
        """
        Парсит oneof поля из содержимого.
        
        Args:
            text: Содержимое блока message
        Returns:
            str: Оставшийся текст после разбора oneof
        """
        for match in one_of_re.finditer(text):
            one_of = OneOf(match.group(1), match.group(2), self)
            self.one_ofs[one_of.name] = one_of
            # parsing one of fields
            self.parse_fields(match.group(2), one_of)
        return one_of_re.sub("", text)

    def parse_fields(self, text, one_of=None):
        """
        Разбирает все поля сообщения
        Args:
            text: Текст сообщения
            one_of: Oneof поле, если разбираются поля oneof
        Returns:
            str: Оставшийся текст после разбора полей
        """
        return self.parse_simple_fields(
            self.parse_repeated_fields(self.parse_map_fields(self.parse_optional_fields(text, one_of), one_of), one_of),
            one_of)

    def parse_simple_fields(self, text, one_of=None):
        """
        Разбирает простые поля
        Args:
            text: Текст сообщения
            one_of: Oneof поле, если разбираются поля oneof
        Returns:
            str: Оставшийся текст после разбора простых полей
        """
        for match in field_re.finditer(text):
            field = Field(len(self.fields), match.group(2), match.group(1), match.group(3), FieldType.SIMPLE, one_of, self)
            if one_of:
                one_of.fields.append(field)
            self.fields[field.full_name()] = field
        return field_re.sub("", text)

    def parse_repeated_fields(self, text, one_of=None):
        """
        Разбирает повторяющиеся поля
        Args:
            text: Текст сообщения
            one_of: Oneof поле, если разбираются поля oneof
        Returns:
            str: Оставшийся текст после разбора повторяющихся полей
        """
        for match in repeated_field_re.finditer(text):
            field = Field(len(self.fields), match.group(2), match.group(1), match.group(3), FieldType.REPEATED, one_of, self)
            if one_of:
                one_of.fields.append(field)
            self.fields[field.full_name()] = field
        return repeated_field_re.sub("", text)

    def parse_map_fields(self, text, one_of=None):
        """
        Разбирает поля типа map
        Args:
            text: Текст сообщения
            one_of: Oneof поле, если разбираются поля oneof
        Returns:
            str: Оставшийся текст после разбора map полей
        """
        for match in map_field_re.finditer(text):
            field = Field(len(self.fields), match.group(3), match.group(2), match.group(3), FieldType.MAP, one_of, self)
            field.map_key_type = match.group(1)
            if one_of:
                one_of.fields.append(field)
            self.fields[field.full_name()] = field
        return map_field_re.sub("", text)

    def parse_optional_fields(self, text, one_of=None):
        """
        Разбирает опциональные поля
        Args:
            text: Текст сообщения
            one_of: Oneof поле, если разбираются поля oneof
        Returns:
            str: Оставшийся текст после разбора опциональных полей
        """
        for match in optional_field_re.finditer(text):
            field = Field(len(self.fields), match.group(2), match.group(1), match.group(3), FieldType.OPTIONAL, one_of, self)
            if one_of:
                one_of.fields.append(field)
            self.fields[field.full_name()] = field
        return optional_field_re.sub("", text)

    def full_name(self):
        return self.message.full_name() + "." + self.name if self.message else self.file.package + "." + self.name if self.file.package else self.name

    def cpp_name(self):
        """
        Возвращает имя сообщения в формате C++.
        
        Returns:
            str: Имя сообщения в формате C++
        """
        return self.message.cpp_name() + "_" + self.name if self.message else self.namespace_name() + self.name

    def class_name(self):
        return self.message.class_name() + "_" + self.name if self.message else self.name

    def namespace_name(self):
        """
        Возвращает имя пространства имен для C++
        Returns:
            str: Имя пространства имен
        """
        return self.file.package.replace(".", "::") + "::" if self.file.package else ""


class FieldType(Enum):
    """
    Типы полей protobuf
    """
    SIMPLE = 1      # Обычное поле
    REPEATED = 2    # Повторяющееся поле
    MAP = 3         # Поле типа map
    OPTIONAL = 4    # Опциональное поле


class Field:
    """
    Класс, представляющий поле в сообщении protobuf.
    Хранит информацию о:
    - Типе поля
    - Имени поля
    - Номере поля
    - Признаках (повторяющееся, опциональное, map)
    """
    def __init__(self, field_id: int, name: str, type_name: str, field_number: str, field_type: FieldType, one_of, message):
        """
        Инициализирует объект поля.
        
        Args:
            field_id: Идентификатор поля
            name: Имя поля
            type_name: Тип поля
            field_number: Номер поля
            field_type: Тип поля
            one_of: Oneof поле, если разбираются поля oneof
            message: Сообщение, содержащее поле
        """
        self.id: int = field_id
        self.message: Message = message
        self.name: str = name
        self.type_name: str = type_name
        self.field_number: str = field_number
        self.field_type: FieldType = field_type
        self.one_of: OneOf = one_of
        self.type_message = None
        self.type_enum = None
        self.map_key_type = None

        if self.is_repeated():
            self.message.file.parser.repeated_fields.append(self)

    def __eq__(self, other):
        return self == other

    def __lt__(self, other):
        #if self.is_repeated() != other.is_repeated():
        #    return self.is_repeated() < other.is_repeated()
        #if self.is_map() != other.is_map():
        #    return self.is_map() < other.is_map()
        #if self.is_of_message_type() != other.is_of_message_type():
        #    return self.is_of_message_type() < other.is_of_message_type()
        return self.id < other.id

    def is_container(self):
        return self.field_type != FieldType.SIMPLE

    def is_repeated(self):
        """
        Проверяет, является ли поле повторяющимся
        Returns:
            bool: True если поле является повторяющимся
        """
        return self.field_type == FieldType.REPEATED

    def is_map(self):
        """
        Проверяет, является ли поле типом map
        Returns:
            bool: True если поле является map
        """
        return self.field_type == FieldType.MAP

    def is_optional(self):
        """
        Проверяет, является ли поле опциональным
        Returns:
            bool: True если поле является опциональным
        """
        return self.field_type == FieldType.OPTIONAL

    def full_name(self):
        return self.message.full_name() + "." + self.name

    def class_type_name(self):
        if self.is_of_scalar_type():
            return self.type_name
        if self.is_of_message_type():
            return self.get_type_message().class_name()
        if self.is_of_enum_type():
            return self.get_type_enum().class_name()
        return None

    def type_namespace_name(self):
        if self.is_of_scalar_type() and self.is_container():
            return "models::"
        if self.is_of_message_type():
            return self.get_type_message().namespace_name()
        if self.is_of_enum_type():
            return self.get_type_enum().namespace_name()
        return None

    def cpp_type_name(self):
        if self.is_of_scalar_type():
            return cpp_type_names[self.type_name]
        if self.is_of_message_type():
            return self.get_type_message().cpp_name()
        if self.is_of_enum_type():
            return self.get_type_enum().cpp_name()
        return None

    def qt_type_name(self):
        if self.is_of_scalar_type():
            return qt_type_names[self.type_name]
        if self.is_of_message_type():
            return self.get_type_message().cpp_name()
        if self.is_of_enum_type():
            return self.get_type_enum().cpp_name()
        return None

    def is_of_primitive_type(self):
        """
        Проверяет, является ли тип поля примитивным
        Returns:
            bool: True если тип поля примитивный
        """
        return self.type_name in primitive_types

    def is_of_scalar_type(self):
        """
        Проверяет, является ли тип поля скалярным
        Returns:
            bool: True если тип поля скалярный
        """
        return self.is_of_primitive_type() or self.is_of_string_type()

    def is_of_string_type(self):
        """
        Проверяет, является ли тип поля строковым
        Returns:
            bool: True если тип поля строковый
        """
        return self.type_name in string_types

    def is_of_enum_type(self):
        return self.get_type_enum()

    def is_of_message_type(self):
        return self.get_type_message() is not None

    def get_type_message(self):
        if self.type_message:
            return self.type_message

        if self.is_of_scalar_type():
            return None

        if "." in self.type_name:
            self.type_message = self.message.file.parser.find_message(self.type_name, self.message.file.package)
        else:
            self.type_message = self.message.file.find_message(self.type_name, self.message.full_name())
            if not self.type_message:
                self.type_message = self.message.file.parser.find_message(self.type_name, self.message.full_name())

        return self.type_message

    def get_type_enum(self):
        if self.type_enum:
            return self.type_enum

        if self.is_of_scalar_type():
            return None

        if "." in self.type_name:
            self.type_enum = self.message.file.parser.find_enum(self.type_name, self.message.file.package)
        else:
            self.type_enum = self.message.file.find_enum(self.type_name, self.message.full_name())
            if not self.type_enum:
                self.type_enum = self.message.file.parser.find_enum(self.type_name, self.message.full_name())

        return self.type_enum


class OneOf:
    """
    Класс, представляющий oneof поле в сообщении protobuf.
    Хранит информацию о:
    - Имени oneof
    - Полях, входящих в oneof
    """
    def __init__(self, name: str, text: str, message: Message):
        """
        Инициализирует объект oneof.
        
        Args:
            name: Имя oneof
            text: Содержимое oneof
            message: Сообщение, содержащее oneof
        """
        self.name: str = name
        self.fields: Dict[str, Field] = {}
        self.message: Message = message
        self.name: str = name
        self.fields: List[Field] = []

    def full_name(self):
        """
        Возвращает полное имя oneof поля
        Returns:
            str: Полное имя поля
        """
        return self.message.full_name() + "." + self.name


class Enum:
    """
    Класс, представляющий перечисление в .proto файле.
    Хранит информацию о:
    - Имени перечисления
    - Значениях перечисления
    """
    def __init__(self, name: str, text: str, file: File, message: Message = None):
        """
        Инициализирует объект перечисления.
        
        Args:
            name: Имя перечисления
            text: Содержимое перечисления
            file: Файл, содержащий перечисление
            message: Сообщение, содержащее перечисление
        """
        self.file: File = file
        self.message: Message = message
        self.name: str = name
        self.values: list = []
        self.qt_full_name = ""
        self.has_repeated_instances: bool = False

        # parsing values
        for match in enum_values_re.finditer(text):
            self.values.append(match.group(1))

    def full_name(self):
        return self.message.full_name() + "." + self.name if self.message else self.file.package + "." + self.name if self.file.package else self.name

    def cpp_name(self):
        return self.message.cpp_name() + "_" + self.name if self.message else self.namespace_name() + self.name

    def class_name(self):
        return self.message.class_name() + "_" + self.name if self.message else self.name

    def namespace_name(self):
        return self.file.package.replace(".", "::") + "::" if self.file.package else ""


class Parser:
    """
    Основной класс парсера protobuf файлов
    Управляет разбором всех .proto файлов и их зависимостей
    """
    def __init__(self, file_paths: list[str]):
        self.files: Dict[str, File] = {}
        self.repeated_fields: list = []
        self.repeated_data_types: set = set()

        self.init_files(file_paths)

        for field in self.repeated_fields:
            if field.is_of_scalar_type():
                self.repeated_data_types.add(field.type_name)
            elif field.is_of_message_type():
                field.type_message.has_repeated_instances = True
            elif field.is_of_enum_type():
                field.type_enum.has_repeated_instances = True

    def init_files(self, file_paths: list[str]):
        """
        Инициализирует разбор файлов
        Args:
            file_paths: Список путей к .proto файлам
        """
        for file_path in file_paths:
            if file_path not in self.files:
                self.files[file_path] = File(file_path, self)

    def find_message(self, name, prefix):
        result = None
        for file in self.files.values():
            result = file.find_message(name, prefix)
            if result:
                break
        return result

    def find_enum(self, name, prefix):
        result = None
        for file in self.files.values():
            result = file.find_enum(name, prefix)
            if result:
                break
        return result
