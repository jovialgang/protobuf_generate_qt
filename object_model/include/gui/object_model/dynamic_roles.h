#pragma once

#include "roles.h"
#include <QObject>

namespace om
{
class AbstractDynamicRolesProvider;

class AbstractDynamicRolesReceiver
{
public:
    virtual ~AbstractDynamicRolesReceiver();

    void AddDynamicRolesProvider(AbstractDynamicRolesProvider* provider);
    void RemoveDynamicRolesProvider(AbstractDynamicRolesProvider* provider);
    void ClearDynamicRolesProviders();
    void UpdateDynamicRoles();

protected:
    virtual void SetDynamicRoles(const Ids& val) = 0;

    QVector<AbstractDynamicRolesProvider*> dynamic_roles_providers_;
};

class AbstractDynamicRolesProvider
{
public:
    virtual ~AbstractDynamicRolesProvider();

    virtual Ids GetDynamicRoles() const = 0;

    void SetDynamicRolesReceiver(AbstractDynamicRolesReceiver* receiver);

protected:
    virtual void OnDynamicRolesChanged();

private:
    AbstractDynamicRolesReceiver* dynamic_roles_receiver_ = nullptr;
};
}  // namespace om
