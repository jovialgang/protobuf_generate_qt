#include "abstract_object_model_meta_data.h"

namespace
{
void ParseMetaObjectProperties(const std::shared_ptr<AbstractObjectModelMetaData> &meta_data, const QMetaObject *meta_object, int &role, RoleNames &visited,
    AbstractObjectModelMetaData::RoleInfo *role_info, const QByteArray &role_prefix = {})
{
    if (!meta_object || visited.contains(meta_object->className()))
        return;
    visited.insert(meta_object->className());

    std::vector<AbstractObjectModelMetaData::Notifier *> notifiers;

    for (int i = 0; i < meta_object->propertyCount(); ++i)
    {
        const auto &property             = meta_object->property(i);
        auto        property_meta_object = QMetaType::metaObjectForType(property.userType());

        QByteArray new_role_name;
        if(role_prefix.isEmpty())
            new_role_name = property.name();
        else
            new_role_name = role_prefix + '.' + property.name();

        const QByteArray &role_name = new_role_name;

        meta_data->role_names[role]    = role_name;
        meta_data->role_ids[role_name] = role;

        auto property_role_info = std::make_unique<AbstractObjectModelMetaData::RoleInfo>(role, role_name, property, property_meta_object, role_info);
        property_role_info->dependent_role_ids.insert(role);
        auto property_role_info_ptr = property_role_info.get();
        role_info->children.push_back(property_role_info_ptr);

        auto signal_index = property.notifySignalIndex();
        if (signal_index != -1)
        {
            AbstractObjectModelMetaData::Notifier *notifier = nullptr;
            auto notifier_it = std::find_if(notifiers.begin(), notifiers.end(), [=](AbstractObjectModelMetaData::Notifier *n) { return n->signal_index == signal_index; });
            if (notifier_it != notifiers.end())
            {
                notifier = *notifier_it;
            }
            else
            {
                meta_data->notifiers.emplace_back(std::make_unique<AbstractObjectModelMetaData::Notifier>(static_cast<int>(meta_data->notifiers.size()), signal_index, role_info));
                notifier = meta_data->notifiers.back().get();
                notifiers.push_back(notifier);
            }

            property_role_info->notifier = notifier;
            notifier->properties.push_back(property_role_info_ptr);
        }

        meta_data->role_infos.emplace_back(std::move(property_role_info));

        ++role;

        if (property_meta_object)
        {
            ParseMetaObjectProperties(meta_data, property_meta_object, role, visited, property_role_info_ptr, role_name);
            role_info->dependent_role_ids.insert(property_role_info_ptr->dependent_role_ids.begin(), property_role_info_ptr->dependent_role_ids.end());
        }
        else
        {
            role_info->dependent_role_ids.insert(role);
        }
    }
    visited.erase(meta_object->className());
}

QObject *GetRoleObject(QObject *root, const AbstractObjectModelMetaData::RoleInfo *info)
{
    return info->parent->id == AbstractObjectModelMetaData::kItemRole ? root : info->parent->ReadFromItem(GetRoleObject(root, info->parent)).value<QObject *>();
}
}  // namespace

const int                    AbstractObjectModelMetaData::kItemRole         = Qt::UserRole + 1;
const QByteArray             AbstractObjectModelMetaData::kItemRoleName     = "item";
const QHash<int, QByteArray> AbstractObjectModelMetaData::kDefaultRoleNames = { { kItemRole, kItemRoleName } };
const QHash<QByteArray, int> AbstractObjectModelMetaData::kDefaultRoleIds   = { { kItemRoleName, kItemRole } };

std::shared_ptr<AbstractObjectModelMetaData> AbstractObjectModelMetaData::GetMetaData(const QMetaObject *meta_object)
{
    if (!meta_object)
        return nullptr;
    static QHash<QByteArray, std::weak_ptr<AbstractObjectModelMetaData>> meta_data_cache;
    auto                                                                 meta_data_it = meta_data_cache.find(meta_object->className());
    if (meta_data_it != meta_data_cache.end())
        if (auto meta_data = meta_data_it.value().lock())
            return meta_data;

    auto meta_data = std::make_shared<AbstractObjectModelMetaData>();
    meta_data_cache.insert(meta_object->className(), meta_data);

    meta_data->role_names = kDefaultRoleNames;
    meta_data->role_ids   = kDefaultRoleIds;

    meta_data->is_qml_type = QString(meta_object->className()).contains("QMLTYPE");
    if (meta_data->is_qml_type)
        return meta_data;

    meta_data->role_infos.push_back(std::make_unique<AbstractObjectModelMetaData::RoleInfo>(kItemRole, kItemRoleName, QMetaProperty(), meta_object));
    auto role_id = kItemRole + 1;

    RoleNames visited;
    ParseMetaObjectProperties(meta_data, meta_object, role_id, visited, meta_data->role_infos[0].get());

    return meta_data;
}

QVariant AbstractObjectModelMetaData::RoleInfo::Read(QObject *item) const
{
    return ReadFromItem(GetRoleObject(item, this));
}

QVariant AbstractObjectModelMetaData::RoleInfo::ReadFromItem(QObject *item) const
{
    if (!item)
        return QVariant();
    QVariant res(property.isEnumType() ? QVariant::Int : property.userType(), nullptr);  // читаем енамы как инты, так как чтение енамов очень медленное
    auto     data = res.data();
    QMetaObject::metacall(item, QMetaObject::ReadProperty, property.propertyIndex(), &data);
    return res;
    // item->qt_metacall(QMetaObject::ReadProperty, property.propertyIndex(), &data); - не работает для QObject сгенерированных QML
    // return property.read(item);// - очень медленный метод с кучей ненужных проверок
}

const AbstractObjectModelMetaData::RoleInfo *AbstractObjectModelMetaData::GetRoleInfo(int id) const
{
    // TODO: maybe we need to check id here?
    return role_infos[id - kItemRole].get();
}

const AbstractObjectModelMetaData::RoleInfo *AbstractObjectModelMetaData::GetRoleInfo(const QByteArray &role) const
{
    auto id = role_ids.find(role);
    return id != role_ids.end() ? role_infos[id.value() - kItemRole].get() : nullptr;
}
