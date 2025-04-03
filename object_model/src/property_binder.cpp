#include "property_binder.h"
#include <iostream>

using namespace om;

PropertyBinder::PropertyBinder(std::shared_ptr<AbstractReceiver> receiver /*= nullptr*/) : binder_(std::move(receiver))
{}

std::shared_ptr<PropertyBinder::AbstractReceiver> PropertyBinder::GetReceiver() const
{
    return std::static_pointer_cast<PropertyBinder::AbstractReceiver>(binder_.GetReceiver());
}

void PropertyBinder::SetReceiver(std::shared_ptr<AbstractReceiver> val)
{
    if (val)
        val->parent_ = this;
    binder_.SetReceiver(std::move(val));
}

bool PropertyBinder::IsBound() const
{
    return binder_.HasBindings();
}

QObject* PropertyBinder::GetSender() const
{
    return sender_;
}

void PropertyBinder::SetSender(QObject* sender)
{
    // Sender update
    if (sender_ == sender)
        return;

    sender_ = sender;

    // Meta data update
    if (!sender_)
    {
        Reset();
        senders_meta_data_.reset();
    }
    else if (!senders_meta_data_ || sender_->metaObject()->className() != senders_meta_data_->GetClassName())
    {
        Reset();
        senders_meta_data_ = ObjectMetaData::GetMetaData(sender_->metaObject());
    }
}

Ids PropertyBinder::GetProperties() const
{
    return role_ids_;
}

void PropertyBinder::SetProperties(const Ids& role_ids)
{
    if (role_ids_ == role_ids || !senders_meta_data_)
        return;

    role_ids_ = role_ids;
    // Notifiers update
    for (auto role : role_ids_)
    {
        if (auto notifier = senders_meta_data_->GetRoleInfo(role)->notifier)
            notifier_ids_.insert(notifier->id);
    }
}

bool PropertyBinder::IsEnabled() const
{
    return enabled_;
}

void PropertyBinder::SetEnabled(bool val)
{
    if (enabled_ == val)
        return;
    enabled_ = val;
    UpdateBinding();
}

void PropertyBinder::Bind(QObject* sender, const Ids& role_ids)
{
    SetSender(sender);
    SetProperties(role_ids);
    UpdateBinding();
}

void PropertyBinder::Bind(QObject* sender, const QVariant& properties)
{
    // can not use Bind(QObject* sender, const RoleIds& role_ids) because senders_meta_data_ is defined only after SetSender call
    SetSender(sender);
    if (!senders_meta_data_)
        return;
    SetProperties(senders_meta_data_->ConvertToRoleIds(properties));
    UpdateBinding();
}

void PropertyBinder::Reset()
{
    role_ids_.clear();
    notifier_ids_.clear();
    senders_meta_data_.reset();
    binder_.UnbindAll();
}

int PropertyBinder::GetPropertyId(const QString& name) const
{
    if (!senders_meta_data_)
        std::cout << (std::logic_error("Sender is not set"), "Sender is not set");
    return senders_meta_data_->GetRoleId(name.toUtf8());
}

QString PropertyBinder::GetPropertyName(int role_id) const
{
    if (!senders_meta_data_)
        std::cout << (std::logic_error("Sender is not set"), "Sender is not set");
    return senders_meta_data_->GetRoleInfo(role_id)->name;
}

QVariant PropertyBinder::Read(int role_id) const
{
    if (!senders_meta_data_ || !sender_)
        std::cout << (std::logic_error("Sender is not set"), "Sender is not set");
    return senders_meta_data_->GetRoleInfo(role_id)->ReadFromRoot(sender_);
}

bool PropertyBinder::Write(int role_id, const QVariant& val)
{
    if (!senders_meta_data_ || !sender_)
        std::cout << (std::logic_error("Sender is not set"), "Sender is not set");
    return senders_meta_data_->GetRoleInfo(role_id)->WriteToRoot(sender_, val);
}

void PropertyBinder::UpdateBinding()
{
    if (enabled_)
        binder_.Bind(sender_, senders_meta_data_->GetItemRoleInfo(), notifier_ids_);
    else
        binder_.UnbindAll();
}

Ids PropertyBinder::OnSignalReceived(const SignalBinder::Binding& binding, void**)
{
    Ids  changed_roles;
    auto notifier = senders_meta_data_->GetNotifier(binding.id);
    for (auto role : notifier->roles)
    {
        if (SetIntersection(role_ids_, role->dependent_role_ids, changed_roles))
        {
            if (role->meta_object)
                binder_.Bind(role->ReadFromItem(binding.sender).value<QObject*>(), role, notifier_ids_);
        }
    }
    return changed_roles;
}

void PropertyBinder::AbstractReceiver::OnSignalReceived(const SignalBinder::Binding& binding, void** args)
{
    OnPropertiesChanged(parent_->OnSignalReceived(binding, args));
}
