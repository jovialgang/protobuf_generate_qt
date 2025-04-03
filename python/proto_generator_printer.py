import os.path
from string import Template


def read_template(filename: str) -> Template:
    # read file template from subfolder
    filepath = os.path.join(os.path.dirname(__file__), 'templates', filename)
    with open(filepath, 'r', encoding='utf-8') as file:
        return Template(file.read())


H_FILE_TEMPLATE = Template("""#pragma once
#include <QObject>
#include <QString>
#include <QVector>
#include <QVariant>
#include <QAbstractListModel>
#include <array>
#include <gui/object_model/object_model.h>
#include <google/protobuf/util/message_differencer.h>

${includes}

namespace ${namespace_name}
{
${classes}
${definitions}
}

namespace ${name}_qt_pb
{
    void Initialize();
}
""")


def print_h_file(name, namespace_name, includes, classes, definitions):
    return H_FILE_TEMPLATE.substitute({
        "name": name,
        "namespace_name": namespace_name,
        "includes": includes,
        "classes": classes,
        "definitions": definitions
    })


CPP_FILE_TEMPLATE = Template("""#include "${name}_qt_pb.h"
#include <QQmlEngine>
#include <algorithm>
#include <iterator>
#include <stdexcept>
#include <google/protobuf/util/message_differencer.h>

using namespace ${namespace_name};

${implementations}

namespace ${name}_qt_pb
{
    void Initialize()
    {
        static bool initialized = false;
        if (initialized)
            return;
        initialized = true;
${initialize}
    }
}
""")


def print_cpp_file(name, namespace_name, implementations, initialize):
    return CPP_FILE_TEMPLATE.substitute({
        "name": name,
        "namespace_name": namespace_name,
        "implementations": implementations,
        "initialize": initialize
    })


QML_REGISTER_TYPE_TEMPLATE = Template("        qmlRegisterType<${type_name}>(\"${name}\", 1, 0, \"${type_name}\");\n")


def print_qml_register_type(type_name, name):
    return QML_REGISTER_TYPE_TEMPLATE.substitute({
        "type_name": type_name,
        "name": name
    })


QML_REGISTER_UNCREATABLE_TYPE_TEMPLATE = Template(
    "        qmlRegisterUncreatableType<${type_name}>(\"${name}\", 1, 0, \"${type_name}\", \"Error: uncreatable type\");\n")


def print_qml_register_uncreatable_type(type_name, name):
    return QML_REGISTER_UNCREATABLE_TYPE_TEMPLATE.substitute({
        "type_name": type_name,
        "name": name
    })


QML_REGISTER_METATYPE_TEMPLATE = Template(
    "        qmlRegisterUncreatableMetaObject(${type_name}::staticMetaObject, \"${name}\", 1, 0, \"${type_name}\", \"Error: only enums\");\n")


def print_qml_register_metatype(type_name, name):
    return QML_REGISTER_METATYPE_TEMPLATE.substitute({
        "type_name": type_name,
        "name": name
    })


QML_REGISTER_FILE_TEMPLATE = Template("        ${file_name}::Initialize();\n")


def print_qml_register_file(file_name):
    return QML_REGISTER_FILE_TEMPLATE.substitute({
        "file_name": file_name
    })


FORWARD_CLASS_DEFINITION_TEMPLATE = Template("class ${type_name};\n")


def print_forward_class_definition(type_name):
    return FORWARD_CLASS_DEFINITION_TEMPLATE.substitute({
        "type_name": type_name
    })


ENUM_TEMPLATE = Template("""
    enum ${name} 
    {
${values}
    };
    Q_ENUM(${name})
""")


def print_enum(name, values):
    return ENUM_TEMPLATE.substitute({
        "name": name,
        "values": values
    })


ENUM_NAMESPACE_TEMPLATE = Template("""
namespace ${name}
{
    Q_NAMESPACE
    enum Enum 
    {
${values}
    };
    Q_ENUM_NS(Enum)
}
""")


def print_enum_namespace(name, values):
    return ENUM_NAMESPACE_TEMPLATE.substitute({
        "name": name,
        "values": values
    })


ENUM_VALUE_TEMPLATE = Template("        ${new_name} = ${type_name}::${name},\n")


def print_enum_value(type_name, name, new_name):
    return ENUM_VALUE_TEMPLATE.substitute({
        "name": name,
        "type_name": type_name,
        "new_name": new_name
    })


OBJECT_CLASS_H_TEMPLATE = read_template('object_class_template.h')


def print_object_class_h(type_name, message_type, properties, enums, function_definitions, private_function_definitions,
                         slot_function_definitions, notifiers, members, fields_count, using):
    return OBJECT_CLASS_H_TEMPLATE.substitute({
        'type_name': type_name,
        'message_type': message_type,
        'properties': properties,
        'enums': enums,
        'function_definitions': function_definitions,
        'slot_function_definitions': slot_function_definitions,
        'private_function_definitions': private_function_definitions,
        'notifiers': notifiers,
        'members': members,
        'fields_count': fields_count,
        'using': using
    })


OBJECT_CLASS_CPP_TEMPLATE = read_template('object_class_template.cpp')


def print_object_class_cpp(type_name, message_type, sync_members, sync_signals, check_members,
                           setup, function_implementations):
    return OBJECT_CLASS_CPP_TEMPLATE.substitute({
        "type_name": type_name,
        "message_type": message_type,
        "sync_members": sync_members,
        "sync_signals": sync_signals,
        "check_members": check_members,
        "setup": setup,
        "function_implementations": function_implementations
    })


INCLUDE_TEMPLATE = Template("#include \"${file_name}\"\n")


def print_include(file_name):
    return INCLUDE_TEMPLATE.substitute({
        "file_name": file_name
    })


EXTERNAL_INCLUDE_TEMPLATE = Template("#include <${file_name}>\n")


def print_external_include(file_name):
    return EXTERNAL_INCLUDE_TEMPLATE.substitute({
        "file_name": file_name
    })


WELL_KNOWN_TYPES_INCLUDES = """#include <google/protobuf/timestamp.pb.h>
#include <google/protobuf/duration.pb.h>
"""


# PROPERTIES

def print_property(type_name, property_name, function_name, signal_name, read_prefix="Get"):
    return PROPERTY_TEMPLATE.substitute({
        "type_name": type_name,
        "property_name": property_name,
        "function_name": function_name,
        "signal_name": signal_name,
        "read_prefix": read_prefix
    })


PROPERTY_TEMPLATE = Template(
    "    Q_PROPERTY(${type_name} ${property_name} READ ${read_prefix}${function_name} WRITE Set${function_name} NOTIFY ${signal_name}Changed${features})\n")


def print_property(type_name, property_name, function_name, signal_name, read_prefix="Get", features=""):
    if features:
        features = " " + features
    return PROPERTY_TEMPLATE.substitute({
        "type_name": type_name,
        "property_name": property_name,
        "function_name": function_name,
        "signal_name": signal_name,
        "read_prefix": read_prefix,
        "features": features
    })


READ_ONLY_PROPERTY_TEMPLATE = Template(
    "    Q_PROPERTY(${type_name} ${property_name} READ ${read_prefix}${function_name} NOTIFY ${signal_name}Changed)\n")


def print_read_only_property(type_name: str, property_name: str, function_name: str, signal_name: str, read_prefix:str = "Get") -> str:
    return READ_ONLY_PROPERTY_TEMPLATE.substitute({
        "type_name": type_name,
        "property_name": property_name,
        "function_name": function_name,
        "signal_name": signal_name,
        "read_prefix": read_prefix
    })


CONST_PROPERTY_TEMPLATE = Template(
    "    Q_PROPERTY(${type_name} ${property_name} READ ${read_prefix}${function_name} CONSTANT)\n")


def print_const_property(type_name, property_name, function_name, read_prefix="Get"):
    return CONST_PROPERTY_TEMPLATE.substitute({
        "type_name": type_name,
        "property_name": property_name,
        "function_name": function_name,
        "read_prefix": read_prefix
    })


USING_ENUMS_TEMPLATE = Template(
    "   using ${using_type_name} = ${enum_type_name};\n"
)


def print_using_enums(using_type_name, enum_type_name) -> str:
    return USING_ENUMS_TEMPLATE.substitute(
        using_type_name=using_type_name,
        enum_type_name=enum_type_name
    )


USING_NESTED_MESSAGE_TEMPLATE = Template(
    "   using ${using_type_name} = ${message_type_name};\n"
)


def print_using_nested_message(using_type_name, message_type_name) -> str:
    return USING_NESTED_MESSAGE_TEMPLATE.substitute(
        using_type_name=using_type_name,
        message_type_name=message_type_name
    )


"""
HAS_OPTIONAL_PROPERTY_TEMPLATE = Template(
    "    Q_PROPERTY(bool has${function_name} READ Has${function_name} NOTIFY ${signal_name}Changed)\n")


def print_has_optional_property(function_name, signal_name):
    return HAS_OPTIONAL_PROPERTY_TEMPLATE.substitute({
        "function_name": function_name,
        "signal_name": signal_name
    })


ONE_OF_CASE_PROPERTY_TEMPLATE = Template(
    "    Q_PROPERTY(${type_name} ${property_name}Case READ Get${function_name} WRITE Set${function_name} NOTIFY ${property_name}Changed)\n")


def print_one_of_case_property(type_name, property_name, function_name):
    return ONE_OF_CASE_PROPERTY_TEMPLATE.substitute({
        "type_name": type_name,
        "property_name": property_name,
        "function_name": function_name
    })
"""


# FUNCTIONS

def print_function_h(function_name, type_name="void", args="", specifiers=""):
    if specifiers:
        specifiers = " " + specifiers
    return Template("    ${type_name} ${function_name}(${args})${specifiers};\n").substitute({
        "type_name": type_name,
        "function_name": function_name,
        "args": args,
        "specifiers": specifiers
    })


def print_getter_h(type_name, function_name, prefix="Get"):
    return Template("    ${type_name} ${prefix}${function_name}() const;\n").substitute({
        "type_name": type_name,
        "function_name": function_name,
        "prefix": prefix
    })


def print_mutable_getter_h(type_name, function_name, prefix="Get"):
    return Template("    ${type_name} ${prefix}${function_name}();\n").substitute({
        "type_name": type_name,
        "function_name": function_name,
        "prefix": prefix
    })


def print_setter_h(type_name, function_name):
    return Template("    void Set${function_name}(${type_name} val);\n").substitute({
        "type_name": type_name,
        "function_name": function_name
    })


FUNCTION_CPP_TEMPLATE = Template("""${type_name} ${class_name}::${function_name}(${args}) ${specifiers}
{
${code}
}

""")


def print_function_cpp(class_name, function_name, code, type_name="void", args="", specifiers=""):
    return FUNCTION_CPP_TEMPLATE.substitute({
        "type_name": type_name,
        "class_name": class_name,
        "function_name": function_name,
        "code": code,
        "args": args,
        "specifiers": specifiers
    })


RETURN_CPP_TEMPLATE = Template("    return ${get_val};")


def print_return(get_val):
    return RETURN_CPP_TEMPLATE.substitute({
        "get_val": get_val
    })


GETTER_CPP_TEMPLATE = Template("""${type_name} ${class_name}::${prefix}${function_name}() const
{
${get_val}
}

""")


def print_getter_cpp(type_name, class_name, function_name, get_val, prefix="Get"):
    return GETTER_CPP_TEMPLATE.substitute({
        "type_name": type_name,
        "class_name": class_name,
        "function_name": function_name,
        "get_val": get_val,
        "prefix": prefix
    })


MUTABLE_GETTER_CPP_TEMPLATE = Template("""${type_name} ${class_name}::${prefix}${function_name}()
{
${get_val}
}

""")


def print_mutable_getter_cpp(type_name, class_name, function_name, get_val, prefix="Get"):
    return MUTABLE_GETTER_CPP_TEMPLATE.substitute({
        "type_name": type_name,
        "class_name": class_name,
        "function_name": function_name,
        "get_val": get_val,
        "prefix": prefix
    })


SETTER_CPP_TEMPLATE = Template("""void ${class_name}::Set${function_name}(${type_name} val)
{
${set_val}
}

""")


def print_setter_cpp(type_name, class_name, function_name, set_val):
    return SETTER_CPP_TEMPLATE.substitute({
        "type_name": type_name,
        "class_name": class_name,
        "function_name": function_name,
        "set_val": set_val
    })


VAL_CHECK_TEMPLATE = Template("""
    if (message_->${name}() != new_message.${name}())
        changed_properties_[${field_index}] = true;
""")


def print_val_check(name, field_index):
    return VAL_CHECK_TEMPLATE.substitute({
        "name": name,
        "field_index": field_index
    })


OBJECT_CHECK_TEMPLATE = Template("""
    if (${variable_name})
        ${variable_name}->CheckForChangedProperties(new_message.${name}());
""")


def print_object_check(name, variable_name):
    return OBJECT_CHECK_TEMPLATE.substitute({
        "name": name,
        "variable_name": variable_name
    })


OPTIONAL_VAL_CHECK_TEMPLATE = Template(""" ||
        (message_->has_${name}() && message_->${name}() != new_message.${name}())""")


def print_optional_val_check(name):
    return OPTIONAL_VAL_CHECK_TEMPLATE.substitute({
        "name": name
    })


ONE_OF_CHECK_TEMPLATE = Template("""
    if (message_->${one_of_name}() != new_message.${one_of_name}()${one_of_values_check})
        changed_properties_[${field_index}] = true;
""")


def print_one_of_check(one_of_name, one_of_values_check, field_index):
    return ONE_OF_CHECK_TEMPLATE.substitute({
        "one_of_name": one_of_name,
        "one_of_values_check": one_of_values_check,
        "field_index": field_index
    })


OPTIONAL_CHECK_TEMPLATE = Template("""
    if (message_->has_${name}() != new_message.has_${name}()${val_check})
        changed_properties_[${field_index}] = true;
""")


def print_optional_check(name, field_index, val_check=""):
    return OPTIONAL_CHECK_TEMPLATE.substitute({
        "name": name,
        "val_check": val_check,
        "field_index": field_index
    })


NOTIFIER_TEMPLATE = Template("    void ${signal_name}Changed();\n")


def print_notifier(signal_name):
    return NOTIFIER_TEMPLATE.substitute({
        "signal_name": signal_name
    })


MEMBER_TEMPLATE = Template("    ${specifiers}${type_name} ${variable_name} = ${default_value};\n")


def print_member(type_name, variable_name, default_value="nullptr", specifiers=""):
    if specifiers:
        specifiers = specifiers + " "
    return MEMBER_TEMPLATE.substitute({
        "type_name": type_name,
        "variable_name": variable_name,
        "default_value": default_value,
        "specifiers": specifiers
    })


MEMBER_FIELD_INITIALIZE_TEMPLATE = Template(
    "    ${variable_name} = new ${type_name}(message_->mutable_${name}(), this);\n")


def print_member_field_initialize(name, type_name, variable_name):
    return MEMBER_FIELD_INITIALIZE_TEMPLATE.substitute({
        "type_name": type_name,
        "variable_name": variable_name,
        "name": name
    })


MEMBER_ONE_OF_FIELD_INITIALIZE_TEMPLATE = Template(
    "    ${variable_name} = new ${type_name}(message_->has_${name}() ? message_->mutable_${name}() : nullptr, this);\n")


def print_member_one_of_field_initialize(name, type_name, variable_name):
    return MEMBER_ONE_OF_FIELD_INITIALIZE_TEMPLATE.substitute({
        "type_name": type_name,
        "variable_name": variable_name,
        "name": name
    })


CHANGED_CONNECT_TEMPLATE = Template(
    "    connect(${variable_name}, &${variable_type}::${signal_name}, this, &${type_name}::EmitChanged);\n")


def print_changed_connect(signal_name, type_name, variable_name, variable_type):
    return CHANGED_CONNECT_TEMPLATE.substitute({
        "type_name": type_name,
        "signal_name": signal_name,
        "variable_name": variable_name,
        "variable_type": variable_type
    })


OBJECT_SYNC_TEMPLATE = Template("""
    if (${variable_name})
        ${variable_name}->SetProtoMessage(message_->mutable_${name}(), emit_all_signals);
""")


def print_object_sync(name, variable_name):
    return OBJECT_SYNC_TEMPLATE.substitute({
        "variable_name": variable_name,
        "name": name
    })


REPEATED_SYNC_TEMPLATE = Template("""
    if (${variable_name})
        ${variable_name}->SetProtoMessage(message_->mutable_${name}());
""")


def print_repeated_sync(name, variable_name):
    return REPEATED_SYNC_TEMPLATE.substitute({
        "variable_name": variable_name,
        "name": name
    })


VAL_SYNC_TEMPLATE = Template("""
        if (emit_all_signals_ || changed_properties_[${property_index}])
            emit ${signal_name}Changed();
""")


def print_val_sync(signal_name, property_index):
    return VAL_SYNC_TEMPLATE.substitute({
        "signal_name": signal_name,
        "property_index": property_index
    })


# getters/setters


GET_VAL_TEMPLATE = Template("message_->${name}()")


def print_get_val(name):
    return GET_VAL_TEMPLATE.substitute({
        "name": name
    })


SET_VAL_TEMPLATE = Template("    message_->set_${name}(val);")


def print_set_val(name):
    return SET_VAL_TEMPLATE.substitute({
        "name": name
    })


SET_VAL_STD_TEMPLATE = Template("    message_->set_${name}(val);")


def print_set_val_std(name):
    return SET_VAL_STD_TEMPLATE.substitute({
        "name": name
    })


GET_STRING_VAL_TEMPLATE = Template("QString::fromStdString(message_->${name}())")


def print_get_string_val(name):
    return GET_STRING_VAL_TEMPLATE.substitute({
        "name": name
    })


SET_STRING_VAL_TEMPLATE = Template("    message_->set_${name}(val.toStdString());")


def print_set_string_val(name):
    return SET_STRING_VAL_TEMPLATE.substitute({
        "name": name
    })


SET_STRING_VAL_STD_TEMPLATE = Template("    message_->set_${name}(val);")


def print_set_string_val_std(name):
    return SET_STRING_VAL_STD_TEMPLATE.substitute({
        "name": name
    })


GET_CAST_VAL_TEMPLATE = Template("static_cast<${type_name}>(message_->${name}())")


def print_get_cast_val(type_name, name):
    return GET_CAST_VAL_TEMPLATE.substitute({
        "type_name": type_name,
        "name": name
    })


SET_CAST_VAL_TEMPLATE = Template("    message_->set_${name}(static_cast<${type_name}>(val));")


def print_set_cast_val(type_name, name):
    return SET_CAST_VAL_TEMPLATE.substitute({
        "type_name": type_name,
        "name": name
    })


GET_OPTIONAL_VAL_TEMPLATE = Template("message_->has_${name}() ? ${get_val} : ${default_value}")


def print_get_optional_val(name, get_val, default_value):
    return GET_OPTIONAL_VAL_TEMPLATE.substitute({
        "name": name,
        "get_val": get_val,
        "default_value": default_value
    })


GET_OBJECT_TEMPLATE = Template("""
    if (!${variable_name})
    {
        ${variable_name} = new ${variable_type}(message_->mutable_${name}(), const_cast<${type_name}*>(this));
        connect(${variable_name}, &${variable_type}::${signal_name}, this, &${type_name}::EmitChanged);
    }
    return ${variable_name};
""")


def print_get_object(name, variable_name, variable_type, signal_name, type_name):
    return GET_OBJECT_TEMPLATE.substitute({
        "name": name,
        "variable_name": variable_name,
        "variable_type": variable_type,
        "signal_name": signal_name,
        "type_name": type_name
    })


GET_OPTIONAL_OBJECT_TEMPLATE = Template("""
    if (!message_->has_${name}())
        return ${default_value};
    ${get_val}
""")


def print_get_optional_object(name, variable_name, variable_type, signal_name, type_name, default_value):
    return GET_OPTIONAL_OBJECT_TEMPLATE.substitute({
        "name": name,
        "get_val": print_get_object(name, variable_name, variable_type, signal_name, type_name),
        "default_value": default_value
    })


SET_OBJECT_TEMPLATE = Template("""
    if (${variable_name})
        ${variable_name}->Set(val);
    else
        *message_->mutable_${name}() = val;
""")


def print_set_object_lvalue(name, variable_name):
    return SET_OBJECT_TEMPLATE.substitute({
        "name": name,
        "variable_name": variable_name
    })


SET_MOVE_OBJECT_TEMPLATE = Template("""
    if (${variable_name})
        ${variable_name}->Set(std::move(val));
    else
        *message_->mutable_${name}() = std::move(val);
""")


def print_set_object_rvalue(name, variable_name):
    return SET_MOVE_OBJECT_TEMPLATE.substitute({
        "name": name,
        "variable_name": variable_name
    })


CHECK_AND_SET_VAL_TEMPLATE = Template("""    if (val == Get${function_name}())
        return;
${set_val}
    emit ${property_name}Changed();
""")


def print_check_and_set_val(function_name, property_name, set_val):
    return CHECK_AND_SET_VAL_TEMPLATE.substitute({
        "function_name": function_name,
        "property_name": property_name,
        "set_val": set_val
    })


CHECK_AND_SET_VAL_STRING_STD_TEMPLATE = Template("""    if (val == Get${function_name}().toStdString())
        return;
${set_val}
    emit ${property_name}Changed();
""")


def print_check_and_set_val_string_std(function_name, property_name, set_val):
    return CHECK_AND_SET_VAL_STRING_STD_TEMPLATE.substitute({
        "function_name": function_name,
        "property_name": property_name,
        "set_val": set_val
    })


SET_ONE_OF_VAL_TEMPLATE = Template("""
    if (!Update${one_of_name}Case(${enum_name}) && val == Get${function_name}())
        return;
${set_val}
    emit ${signal_name}Changed();
""")


def print_set_one_of_val(function_name, signal_name, one_of_name, enum_name,
                         set_val):
    return SET_ONE_OF_VAL_TEMPLATE.substitute({
        "function_name": function_name,
        "signal_name": signal_name,
        "one_of_name": one_of_name,
        "enum_name": enum_name,
        "set_val": set_val
    })


SET_ONE_OF_VAL_STRING_STD_TEMPLATE = Template("""
    if (!Update${one_of_name}Case(${enum_name}) && val == Get${function_name}().toStdString())
        return;
${set_val}
    emit ${signal_name}Changed();
""")


def print_set_one_of_val_string_std(function_name, signal_name, one_of_name, enum_name, set_val):
    return SET_ONE_OF_VAL_STRING_STD_TEMPLATE.substitute({
        "function_name": function_name,
        "signal_name": signal_name,
        "one_of_name": one_of_name,
        "enum_name": enum_name,
        "set_val": set_val
    })


SET_ONE_OF_OBJECT_TEMPLATE = Template("""
    auto case_changed = Update${one_of_name}Case(${enum_name});
${set_val}
    if (case_changed)
        emit ${signal_name}Changed();
""")


def print_set_one_of_object(one_of_name, signal_name, enum_name,
                            set_val):
    return SET_ONE_OF_OBJECT_TEMPLATE.substitute({
        "signal_name": signal_name,
        "one_of_name": one_of_name,
        "enum_name": enum_name,
        "set_val": set_val
    })


SET_OPTIONAL_VAL_TEMPLATE = Template("""
    if (Has${function_name}() && val == Get${function_name}())
        return;
${set_val}
    emit ${signal_name}Changed();
""")


def print_set_optional_val(function_name, signal_name, set_val):
    return SET_OPTIONAL_VAL_TEMPLATE.substitute({
        "function_name": function_name,
        "signal_name": signal_name,
        "set_val": set_val
    })


SET_OPTIONAL_VAL_STRING_STD_TEMPLATE = Template("""
    if (Has${function_name}() && val == Get${function_name}().toStdString())
        return;
${set_val}
    emit ${signal_name}Changed();
""")


def print_set_optional_val_string_std(function_name, signal_name, set_val):
    return SET_OPTIONAL_VAL_STRING_STD_TEMPLATE.substitute({
        "function_name": function_name,
        "signal_name": signal_name,
        "set_val": set_val
    })


SET_OPTIONAL_OBJECT_TEMPLATE = Template("""
    auto case_changed = !Has${function_name}();
${set_val}
    if (case_changed)
        emit ${signal_name}Changed();
""")


def print_set_optional_object(function_name, signal_name, set_val):
    return SET_OPTIONAL_OBJECT_TEMPLATE.substitute({
        "signal_name": signal_name,
        "function_name": function_name,
        "set_val": set_val
    })


# ONE_OFS
SET_ONE_OF_CASE_TEMPLATE = Template("""    if (Update${function_name}(val))
        emit ${signal_name}Changed();""")


def print_set_one_of_case(function_name, signal_name):
    return SET_ONE_OF_CASE_TEMPLATE.substitute({
        "function_name": function_name,
        "signal_name": signal_name
    })


UPDATE_ONE_OF_CASE = Template("""
    if (val == Get${function_name}())
        return false;
    if (val == ${not_set_case})
        message_->clear_${name}();
${updates}
    return true;
""")


def print_update_one_of_case(name, function_name, not_set_case, updates):
    return UPDATE_ONE_OF_CASE.substitute({
        "name": name,
        "function_name": function_name,
        "not_set_case": not_set_case,
        "updates": updates
    })


ONE_OF_OBJECT_UPDATE_TEMPLATE = Template("""
    if (val == ${enum_name})
    {
        message_->mutable_${name}();
    }
    else if (${variable_name})
    {
        delete ${variable_name};
        ${variable_name} = nullptr;
    }
""")


def print_one_of_object_update(name, enum_name, variable_name):
    return ONE_OF_OBJECT_UPDATE_TEMPLATE.substitute({
        "name": name,
        "enum_name": enum_name,
        "variable_name": variable_name
    })


ONE_OF_VAL_UPDATE_TEMPLATE = Template("""
    if (val == ${enum_name})
        message_->set_${name}(${default_value});
""")


def print_one_of_val_update(name, enum_name, default_value):
    return ONE_OF_VAL_UPDATE_TEMPLATE.substitute({
        "name": name,
        "enum_name": enum_name,
        "default_value": default_value
    })


# OPTIONALS

SET_OPTIONAL_OBJECT_CASE_TEMPLATE = Template("""
    if (val == Has${function_name}())
        return;
    if (val)
    {
        message_->mutable_${name}();
    }
    else
    {
        if (${variable_name})
        {
            delete ${variable_name};
            ${variable_name} = nullptr;
        }
        message_->clear_${name}();
    }
    emit ${signal_name}Changed();
""")


def print_set_optional_object_case(name, function_name, signal_name, variable_name):
    return SET_OPTIONAL_OBJECT_CASE_TEMPLATE.substitute({
        "name": name,
        "function_name": function_name,
        "signal_name": signal_name,
        "variable_name": variable_name
    })


SET_OPTIONAL_VAL_CASE_TEMPLATE = Template("""
    if (val == Has${function_name}())
        return;
    if (val)
        message_->set_${name}(${default_value});
    else
        message_->clear_${name}();
    emit ${signal_name}Changed();
""")


def print_set_optional_val_case(name, function_name, signal_name, default_value):
    return SET_OPTIONAL_VAL_CASE_TEMPLATE.substitute({
        "name": name,
        "function_name": function_name,
        "signal_name": signal_name,
        "default_value": default_value
    })


# MODELS


MODEL_CLASS_H_TEMPLATE = read_template('model_class_template.h')


def print_model_class_h(type_name: str, cpp_data_type: str, repeated_field_type: str, properties: str = "") -> str:
    return MODEL_CLASS_H_TEMPLATE.substitute({
        "properties": properties,
        "type_name": type_name,
        "cpp_data_type": cpp_data_type,
        "repeated_field_type": repeated_field_type
    })


MODEL_CLASS_CPP_TEMPLATE = read_template('model_class_template.cpp')


def print_model_class_cpp(type_name: str, cpp_data_type: str, repeated_field_type: str, get_val: str, set_val: str) -> str:
    return MODEL_CLASS_CPP_TEMPLATE.substitute({
        "type_name": type_name,
        "cpp_data_type": cpp_data_type,
        "repeated_field_type": repeated_field_type,
        "get_val": get_val,
        "set_val": set_val
    })


OBJECT_MODEL_CLASS_H_TEMPLATE = read_template('object_model_class_template.h')


def print_object_model_class_h(type_name, data_type, object_type):
    return OBJECT_MODEL_CLASS_H_TEMPLATE.substitute({
        "type_name": type_name,
        "data_type": data_type,
        "object_type": object_type
    })


OBJECT_MODEL_CLASS_CPP_TEMPLATE = read_template('object_model_class_template.cpp')


def print_object_model_class_cpp(type_name, data_type, object_type):
    return OBJECT_MODEL_CLASS_CPP_TEMPLATE.substitute({
        "type_name": type_name,
        "data_type": data_type,
        "object_type": object_type
    })


MODEL_GET_VAL_TEMPLATE = "data_->Get(index.row())"


def print_model_get_val():
    return MODEL_GET_VAL_TEMPLATE


MODEL_SET_VAL_TEMPLATE = Template("*data_->Mutable(index.row()) = val.value<${type_name}>()")


def print_model_set_val(type_name):
    return MODEL_SET_VAL_TEMPLATE.substitute({
        "type_name": type_name
    })


MODEL_GET_STRING_VAL_TEMPLATE = "QString::fromStdString(data_->Get(index.row()))"


def print_model_get_string_val():
    return MODEL_GET_STRING_VAL_TEMPLATE


MODEL_SET_STRING_VAL_TEMPLATE = "*data_->Mutable(index.row()) = val.toString().toStdString()"


def print_model_set_string_val():
    return MODEL_SET_STRING_VAL_TEMPLATE


MODEL_GET_CAST_VAL_TEMPLATE = Template("static_cast<${type_name}>(data_->Get(index.row()))")


def print_model_get_cast_val(type_name):
    return MODEL_GET_CAST_VAL_TEMPLATE.substitute({
        "type_name": type_name
    })


MODEL_SET_CAST_VAL_TEMPLATE = Template(
    "*data_->Mutable(index.row()) = static_cast<${cast_type_name}>(val.value<${type_name}>())")


def print_model_set_cast_val(type_name, cast_type_name):
    return MODEL_SET_CAST_VAL_TEMPLATE.substitute({
        "type_name": type_name,
        "cast_type_name": cast_type_name
    })
