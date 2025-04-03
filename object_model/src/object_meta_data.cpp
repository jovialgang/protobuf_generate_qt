#include "object_meta_data.h"
#include <QDebug>
#include <iostream>
#include <array>
#include <iomanip>

using namespace om;

namespace
{
QObject* GetRoleObject(QObject* root, const ObjectMetaData::RoleInfo* info)
{
    return info->parent->id == ObjectMetaData::kItemRole ? root : info->parent->ReadFromItem(GetRoleObject(root, info->parent)).value<QObject*>();
}
}  // namespace

const int                    ObjectMetaData::kItemRole         = Qt::UserRole + 1;
const QByteArray             ObjectMetaData::kItemRoleName     = "item";
const QHash<int, QByteArray> ObjectMetaData::kDefaultRoleNames = { { kItemRole, kItemRoleName } };
const QHash<QByteArray, int> ObjectMetaData::kDefaultRoleIds   = { { kItemRoleName, kItemRole } };

std::shared_ptr<ObjectMetaData> ObjectMetaData::GetMetaData(const QMetaObject* meta_object)
{
    if (!meta_object)
        return nullptr;

    static QHash<const QMetaObject*, std::weak_ptr<ObjectMetaData>> meta_data_cache;
    auto                                                            meta_data_it = meta_data_cache.find(meta_object);
    if (meta_data_it != meta_data_cache.end())
        if (auto meta_data = meta_data_it.value().lock())
            return meta_data;

    auto meta_data = std::shared_ptr<ObjectMetaData>(new ObjectMetaData(meta_object));  // not make_shared because of private constructor
    meta_data_cache.insert(meta_object, meta_data);
    return meta_data;
}

ObjectMetaData::ObjectMetaData(const QMetaObject* meta_object) : meta_object_(meta_object), class_name_(meta_object->className())
{
    role_infos_.push_back(std::make_unique<ObjectMetaData::RoleInfo>(kItemRole, kItemRoleName, QMetaProperty(), meta_object_));
    auto  role_id = kItemRole + 1;
    Names visited;
    ParseMetaObjectProperties(meta_object, role_id, visited, role_infos_[0].get());
    qDebug() << (QString("%1 meta data parsed").arg(class_name_));
    // PrintToLog();
}

ObjectMetaData::~ObjectMetaData()
{
    qDebug() << (QString("%1 meta data removed").arg(class_name_));
}

void ObjectMetaData::ParseMetaObjectProperties(const QMetaObject* meta_object, int& role, Names& visited, RoleInfo* role_info, const QByteArray& role_prefix /*= {}*/)
{
    if (!meta_object || visited.contains(meta_object->className()))
        return;

    visited.insert(meta_object->className());

    std::vector<Notifier*> role_notifiers;

    const auto create_role_info = [&](const char* name, const QMetaProperty& property = QMetaProperty(), const QMetaObject* property_meta_object = nullptr) -> RoleInfo* {
        const QByteArray& role_name = role_prefix.isEmpty() ? QByteArray(name) : role_prefix + QByteArray(".") + name;

        role_names_[role]    = role_name;
        role_ids_[role_name] = role;

        auto property_role_info = std::make_unique<RoleInfo>(role, role_name, property, property_meta_object, role_info);
        property_role_info->dependent_role_ids.insert(role);
        auto property_role_info_ptr = property_role_info.get();
        role_info->children.push_back(property_role_info_ptr);
        role_infos_.emplace_back(std::move(property_role_info));
        ++role;
        return property_role_info_ptr;
    };

    const auto find_notifier = [&](int signal_index) -> Notifier* {
        auto notifier_it = std::find_if(role_notifiers.begin(), role_notifiers.end(), [=](Notifier* n) { return n->signal_index == signal_index; });
        return notifier_it != role_notifiers.end() ? *notifier_it : nullptr;
    };

    const auto create_notifier = [&](int signal_index, RoleInfo* property_role_info) -> Notifier* {
        notifiers_.emplace_back(std::make_unique<Notifier>(static_cast<int>(notifiers_.size()), signal_index, property_role_info));
        auto notifier = notifiers_.back().get();
        role_notifiers.push_back(notifier);
        property_role_info->notifier = notifier;
        notifier->roles.push_back(property_role_info);
        return notifier;
    };

    for (int i = 0; i < meta_object->propertyCount(); ++i)
    {
        const auto& property             = meta_object->property(i);
        auto        property_meta_object = QMetaType::metaObjectForType(property.userType());

        auto property_role_info = create_role_info(property.name(), property, property_meta_object);

        if (auto signal_index = property.notifySignalIndex(); signal_index != -1)
        {
            Notifier* notifier = find_notifier(signal_index);
            if (!notifier)
                notifier = create_notifier(signal_index, property_role_info);
        }

        if (property_meta_object)
        {
            ParseMetaObjectProperties(property_meta_object, role, visited, property_role_info, property_role_info->name);
            role_info->dependent_role_ids.insert(property_role_info->dependent_role_ids.begin(), property_role_info->dependent_role_ids.end());
        }
        else
        {
            role_info->dependent_role_ids.insert(property_role_info->id);
        }
    }

    for (int i = 0; i < meta_object->methodCount(); ++i)
    {
        const auto& method = meta_object->method(i);
        if (method.methodType() != QMetaMethod::Signal || method.parameterCount() != 0 || method.name() == "destroyed" || find_notifier(i))
            continue;

        auto property_role_info = create_role_info(method.name());
        create_notifier(i, property_role_info);
        role_info->dependent_role_ids.insert(property_role_info->id);
    }

    visited.erase(meta_object->className());
}

bool ObjectMetaData::RoleInfo::IsSignal() const
{
    return !property.isValid();
}

bool ObjectMetaData::RoleInfo::IsObjectName() const
{
    return name.endsWith("objectName");
}

QVariant ObjectMetaData::RoleInfo::ReadFromRoot(QObject* root) const
{
    return ReadFromItem(GetRoleObject(root, this));
}

QVariant ObjectMetaData::RoleInfo::ReadFromItem(QObject* item) const
{
    if (!item)
        return QVariant();
    if (IsSignal())
        return QVariant::fromValue(item);  // если роль сигнал, то возвращаем объект, который его вызвал
    QVariant res(property.isEnumType() ? QVariant::Int : property.userType(), nullptr);  // читаем енамы как инты, так как чтение енамов очень медленное
    auto data = res.data();
    QMetaObject::metacall(item, QMetaObject::ReadProperty, property.propertyIndex(), &data);

    return res;
}

bool ObjectMetaData::RoleInfo::WriteToRoot(QObject* root, const QVariant& val) const
{
    return WriteToItem(GetRoleObject(root, this), val);
}

bool ObjectMetaData::RoleInfo::WriteToItem(QObject* item, const QVariant& val) const
{
    if (!item)
        return false;
    return property.write(item, val);
}

int ObjectMetaData::GetRoleId(const QByteArray& role_name) const
{
    auto id = role_ids_.find(role_name);
    if (id == role_ids_.end())
    {
        qDebug() << (QString("Unknown data role %1").arg(QString::fromUtf8(role_name)));
        return kInvalidId;
    }
    return *id;
}

int ObjectMetaData::GetRoleId(const QString& role_name) const
{
    return GetRoleId(role_name.toUtf8());
}

Ids ObjectMetaData::ParseRoleName(QString role_name) const
{
    Ids res;
    if (role_name.contains('*'))
    {
        auto role_parts  = role_name.remove(' ').split('/');
        auto role_prefix = role_parts[0].left(role_parts[0].length() - 1).toUtf8();

        Role::Roles flag;

        if (role_parts.size() < 2 || role_parts[1] == "a")
            flag = Role::ALL_ROLES;
        else if (role_parts[1] == "aon")
            flag = Role::ALL_ROLES | Role::OBJECT_NAME_ROLES;
        else if (role_parts[1] == "i")
            flag = Role::INHERITED_ROLES;
        else if (role_parts[1] == "o")
            flag = Role::OWN_ROLES;
        else if (role_parts[1] == "on")
            flag = Role::OBJECT_NAME_ROLES;
        else
            qDebug() << (std::invalid_argument("Unknown role name flag"), QString("Unknown role name flag %1").arg(role_name));

        for (auto id : ParseRoles(flag))
        {
            auto info = GetRoleInfo(id);
            if (info->name.startsWith(role_prefix))
                res.insert(info->id);
        }
    }
    else
    {
        auto role = role_ids_.find(role_name.toUtf8());
        if (role != role_ids_.end())
            res.insert(role.value());
    }

    if (res.empty())
        qDebug() << (std::invalid_argument("Unknown data role"), QString("Unknown data role %1").arg(role_name));
    return res;
}

Ids ObjectMetaData::ParseRoles(Role::Roles roles) const
{
    Ids res;
    if (roles == Role::ITEM_ROLE)
        return res;

    for (const auto& info : role_infos_)
    {
        if (info->id == kItemRole)
            continue;
        if (info->IsSignal())
        {
            if (roles & Role::SIGNAL_ROLES)
                res.insert(info->id);
        }
        else if (info->IsObjectName())
        {
            if (roles & Role::OBJECT_NAME_ROLES)
                res.insert(info->id);
        }
        else
        {
            if ((roles & Role::OWN_ROLES && !info->inherited) || (roles & Role::INHERITED_ROLES && info->inherited))
                res.insert(info->id);
        }
    }
    return res;
}

Ids ObjectMetaData::ConvertToRoleIds(const QVariant& roles) const
{
    Ids res;
    if (roles.userType() == QVariant::Int || roles.userType() == qMetaTypeId<Role::Roles>())
    {
        res = ParseRoles(Role::Roles(roles.value<Role::Roles::Int>()));
    }
    else if (roles.userType() == QVariant::String || roles.userType() == QVariant::ByteArray)
    {
        res = ParseRoleName(roles.toByteArray());
    }
    else if (roles.userType() == QVariant::StringList)
    {
        for (const auto& role : roles.toStringList()) res.merge(ParseRoleName(role.toUtf8()));
    }
    else
    {
        qDebug() << (std::invalid_argument("Unsupported roles type"), QString("Unsupported roles type %1").arg(roles.toString()));
    }
    return res;
}

const ObjectMetaData::RoleInfo* ObjectMetaData::GetRoleInfo(int id) const
{
    return role_infos_[id - kItemRole].get();
}

const ObjectMetaData::RoleInfo* ObjectMetaData::GetRoleInfo(const QByteArray& role_name) const
{
    auto id = role_ids_.find(role_name);
    if (id == role_ids_.end())
    {
        qDebug() << (std::invalid_argument("Unknown data role"), QString("Unknown data role %1").arg(QString::fromUtf8(role_name)));
    }
    return role_infos_[id.value() - kItemRole].get();
}

const ObjectMetaData::RoleInfo* ObjectMetaData::GetRoleInfo(const QString& role_name) const
{
    return GetRoleInfo(role_name.toUtf8());
}

void ObjectMetaData::PrintToLog() const
{
    if (role_infos_.empty())
        return;

    constexpr std::array<int, 6> column_sizes = { 70, 50, 10, 10, 10, 100 };

    std::cout << std::setw(150) << std::setfill('*') << std::left << (QString("********** %1 meta data ").arg(role_infos_[0]->meta_object->className())).toStdString();
    std::cout << std::setw(column_sizes[0]) << std::setfill('_') << std::left << "PROPERTY" <<      //
               std::setw(column_sizes[1]) << std::setfill('_') << std::left << "TYPE" <<          //
               std::setw(column_sizes[2]) << std::setfill('_') << std::left << "ID" <<            //
               std::setw(column_sizes[3]) << std::setfill('_') << std::left << "NOTIFIER" <<      //
               std::setw(column_sizes[4]) << std::setfill('_') << std::left << "INHERITED" <<     //
               std::setw(column_sizes[5]) << std::setfill('_') << std::left << "DEPENDENT ROLES";

    for (int i = 1; i < role_infos_.size(); ++i)
    {
        std::string dependent_ids;
        for (auto d_id : role_infos_[i]->dependent_role_ids) dependent_ids += std::to_string(d_id) + " ";

        const auto& role = role_infos_[i];
        std::cout << std::setw(column_sizes[0]) << std::setfill(' ') << std::left << role->name.toStdString() <<                                           //
                   std::setw(column_sizes[1]) << std::setfill(' ') << std::left << (role->property.isValid() ? role->property.typeName() : "SIGNAL") <<  //
                   std::setw(column_sizes[2]) << std::setfill(' ') << std::left << role->id <<                                                           //
                   std::setw(column_sizes[3]) << std::setfill(' ') << std::left << QString(role->notifier ? QString::number(role->notifier->id) : "-").toStdString() <<       //
                   std::setw(column_sizes[4]) << std::setfill(' ') << std::left << std::boolalpha << role->inherited <<                                  //
                   std::setw(column_sizes[5]) << std::setfill(' ') << std::left << dependent_ids;
    }
}
