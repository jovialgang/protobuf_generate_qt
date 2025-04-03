#pragma once

#include "roles.h"
#include <QByteArray>
#include <QHash>
#include <QMetaObject>
#include <QMetaProperty>
//#include <log_qt.h>
#include <vector>

namespace om
{
class ObjectMetaData
{
public:
    static const int                    kItemRole;
    static const QByteArray             kItemRoleName;
    static const QHash<int, QByteArray> kDefaultRoleNames;
    static const QHash<QByteArray, int> kDefaultRoleIds;

    static std::shared_ptr<ObjectMetaData> GetMetaData(const QMetaObject* meta_object);

    struct Notifier;

    struct RoleInfo
    {
        RoleInfo(int id, const QByteArray& name, const QMetaProperty& property, const QMetaObject* meta_object = nullptr, const RoleInfo* parent = nullptr,
            const Notifier* notifier = nullptr)
            : id(id), name(name), property(property), meta_object(meta_object), parent(parent), notifier(notifier)
        {
            if (parent)
                inherited = parent->parent ? parent->inherited : property.propertyIndex() < parent->meta_object->propertyOffset();
        }

        Id                           id = kInvalidId;
        QByteArray                   name;
        QMetaProperty                property;
        const QMetaObject*           meta_object = nullptr;
        const RoleInfo*              parent      = nullptr;
        const Notifier*              notifier    = nullptr;
        std::vector<const RoleInfo*> children;
        Ids                          dependent_role_ids;  // own role id and all ids of the children tree
        bool                         inherited = false;

        inline bool IsSignal() const;
        inline bool IsObjectName() const;

        // Can not move this methods to ObjectMetaData, because we don't always have a pointer to its instance.
        QVariant ReadFromRoot(QObject* root) const;
        QVariant ReadFromItem(QObject* item) const;
        bool     WriteToRoot(QObject* root, const QVariant& val) const;
        bool     WriteToItem(QObject* item, const QVariant& val) const;
    };

    struct Notifier
    {
        Notifier(int id, int signal_index, const RoleInfo* parent = nullptr) : id(id), signal_index(signal_index), parent(parent) {}

        int                          id           = -1;
        int                          signal_index = -1;
        const RoleInfo*              parent       = nullptr;
        std::vector<const RoleInfo*> roles;
    };

    ~ObjectMetaData();

    const QMetaObject*                            GetMetaObject() const { return meta_object_; }
    const char*                                   GetClassName() const { return meta_object_->className(); }
    const QHash<int, QByteArray>&                 GetRoleNames() const { return role_names_; }
    const QHash<QByteArray, int>&                 GetRoleIds() const { return role_ids_; }
    om::Id                                        GetRoleId(const QByteArray& role_name) const;  // throws exception when doesn't contain role_name
    om::Id                                        GetRoleId(const QString& role_name) const;     // throws exception when doesn't contain role_name
    const std::vector<std::unique_ptr<Notifier>>& GetNotifiers() const { return notifiers_; }
    const std::vector<std::unique_ptr<RoleInfo>>& GetRoleInfos() const { return role_infos_; }

    // Use '*' in the end of the role path, to return all roles that start with the passed role_name
    // You can combine role prefix with Role::Roles flag by adding text parameter after '/' in the end of the role_name
    // ITEM_ROLE - can not be combined with prefix
    // (default)ALL_ROLES - /a or when no flag is provided
    // OWN_ROLES - /o
    // INHERITED_ROLES - /i
    // OBJECT_NAME_ROLES -/on
    // ALL_ROLES | OBJECT_NAME_ROLES - /aon
    Ids ParseRoleName(QString role_name) const;
    Ids ParseRoles(Role::Roles roles) const;
    Ids ConvertToRoleIds(const QVariant& roles) const;

    const RoleInfo* GetItemRoleInfo() const { return GetRoleInfo(kItemRole); }
    const RoleInfo* GetFirstRoleInfo() const { return role_infos_.front().get(); }
    const RoleInfo* GetLastRoleInfo() const { return role_infos_.back().get(); }
    const RoleInfo* GetRoleInfo(int id) const;
    const RoleInfo* GetRoleInfo(const QByteArray& role_name) const;  // throws exception when doesn't contain role_name
    const RoleInfo* GetRoleInfo(const QString& role_name) const;     // throws exception when doesn't contain role_name
    const Notifier* GetFirstNotifier() const { return notifiers_.front().get(); }
    const Notifier* GetLastNotifier() const { return notifiers_.back().get(); }
    const Notifier* GetNotifier(int id) const { return notifiers_[id].get(); }

    void PrintToLog() const;

private:
    ObjectMetaData(const QMetaObject* meta_object);
    void ParseMetaObjectProperties(const QMetaObject* meta_object, int& role, Names& visited, ObjectMetaData::RoleInfo* role_info, const QByteArray& role_prefix = {});

    const QMetaObject* meta_object_ = nullptr;
    QString            class_name_;
    // role ID, role name
    QHash<int, QByteArray> role_names_ = kDefaultRoleNames;
    // role name, role ID
    QHash<QByteArray, int> role_ids_ = kDefaultRoleIds;
    // notifiers
    std::vector<std::unique_ptr<Notifier>> notifiers_;
    // index = role ID - kItemRole
    std::vector<std::unique_ptr<RoleInfo>> role_infos_;

//    COMPONENT_LOGGER("ui.omd");
};
}  // namespace om
