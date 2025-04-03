#include "dynamic_roles.h"

#include <QMetaProperty>
#include <stdexcept>

using namespace om;

// Receiver

AbstractDynamicRolesReceiver::~AbstractDynamicRolesReceiver()
{
    ClearDynamicRolesProviders();
}

void AbstractDynamicRolesReceiver::UpdateDynamicRoles()
{
    Ids new_roles;
    for (auto provider : dynamic_roles_providers_)
    {
        const auto& roles = provider->GetDynamicRoles();
        new_roles.insert(roles.begin(), roles.end());
    }
    SetDynamicRoles(new_roles);
}

void AbstractDynamicRolesReceiver::AddDynamicRolesProvider(AbstractDynamicRolesProvider* provider)
{
    if (!provider || dynamic_roles_providers_.contains(provider))
        return;
    dynamic_roles_providers_.push_back(provider);
    UpdateDynamicRoles();
}

void AbstractDynamicRolesReceiver::RemoveDynamicRolesProvider(AbstractDynamicRolesProvider* provider)
{
    auto i = std::find(dynamic_roles_providers_.begin(), dynamic_roles_providers_.end(), provider);
    if (i == dynamic_roles_providers_.end())
        return;
    dynamic_roles_providers_.erase(i);
    provider->SetDynamicRolesReceiver(nullptr);  // рекурсивно зайдет в эту функцию, но вылетит, потому что уже удален из dynamic_roles_providers_
    UpdateDynamicRoles();
}

void AbstractDynamicRolesReceiver::ClearDynamicRolesProviders()
{
    QVector<AbstractDynamicRolesProvider*> providers;
    std::swap(providers, dynamic_roles_providers_);
    for (auto provider : providers) provider->SetDynamicRolesReceiver(nullptr);
}

// Provider

AbstractDynamicRolesProvider::~AbstractDynamicRolesProvider()
{
    if (dynamic_roles_receiver_)
        dynamic_roles_receiver_->RemoveDynamicRolesProvider(this);
}

void AbstractDynamicRolesProvider::SetDynamicRolesReceiver(AbstractDynamicRolesReceiver* receiver)
{
    if (receiver == dynamic_roles_receiver_)
        return;
    if (dynamic_roles_receiver_)
        dynamic_roles_receiver_->RemoveDynamicRolesProvider(this);
    dynamic_roles_receiver_ = receiver;
    if (dynamic_roles_receiver_)
        dynamic_roles_receiver_->AddDynamicRolesProvider(this);
}

void AbstractDynamicRolesProvider::OnDynamicRolesChanged()
{
    if (dynamic_roles_receiver_)
        dynamic_roles_receiver_->UpdateDynamicRoles();
}
