#pragma once

#include "roles.h"
#include <QByteArray>
#include <QHash>
#include <QMetaObject>
#include <QMetaProperty>
#include <vector>
#include <memory>

struct AbstractObjectModelMetaData
{
public:
    static const int                    kItemRole;
    static const QByteArray             kItemRoleName;
    static const QHash<int, QByteArray> kDefaultRoleNames;
    static const QHash<QByteArray, int> kDefaultRoleIds;

    static std::shared_ptr<AbstractObjectModelMetaData> GetMetaData(const QMetaObject* meta_object);

    struct Notifier;

    struct RoleInfo
    {
        RoleInfo(int id, const QByteArray& name, const QMetaProperty& property, const QMetaObject* meta_object = nullptr, const RoleInfo* parent = nullptr,
            const Notifier* notifier = nullptr)
            : id(id), name(name), property(property), meta_object(meta_object), parent(parent), notifier(notifier)
        {}

        int                          id;
        QByteArray                   name;
        QMetaProperty                property;
        const QMetaObject*           meta_object = nullptr;
        const RoleInfo*              parent      = nullptr;
        const Notifier*              notifier    = nullptr;
        std::vector<const RoleInfo*> children;
        RoleIds                      dependent_role_ids;  // own role id and all ids of the children tree

        QVariant Read(QObject* root) const;
        QVariant ReadFromItem(QObject* item) const;
    };

    struct Notifier
    {
        Notifier(int id, int signal_index, const RoleInfo* parent = nullptr) : id(id), signal_index(signal_index), parent(parent) {}

        int                          id;
        int                          signal_index;
        const RoleInfo*              parent = nullptr;
        std::vector<const RoleInfo*> properties;
    };

    bool is_qml_type = false;

    // role ID, role name
    QHash<int, QByteArray> role_names;
    // role name, role ID
    QHash<QByteArray, int> role_ids;
    // notifiers
    std::vector<std::unique_ptr<Notifier>> notifiers;
    // index = role ID - kItemRole
    std::vector<std::unique_ptr<RoleInfo>> role_infos;

    const RoleInfo* GetFirstRoleInfo() const { return role_infos.front().get(); }
    const RoleInfo* GetLastRoleInfo() const { return role_infos.back().get(); }
    const RoleInfo* GetRoleInfo(int id) const;
    const RoleInfo* GetRoleInfo(const QByteArray& role) const;
    const Notifier* GetFirstNotifier() const { return notifiers.front().get(); }
    const Notifier* GetLastNotifier() const { return notifiers.back().get(); }
    const Notifier* GetNotifier(int id) const { return notifiers[id].get(); }
};
