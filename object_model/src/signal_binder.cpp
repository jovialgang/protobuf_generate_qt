#include "signal_binder.h"

#include <stdexcept>

using namespace om;

const int kMethodIdOffset = 1000;

SignalBinder::SignalBinder(std::shared_ptr<AbstractReceiver> receiver) : QObject(), receiver_(std::move(receiver))
{}

std::shared_ptr<SignalBinder::AbstractReceiver> SignalBinder::GetReceiver() const
{
    return receiver_;
}

void SignalBinder::SetReceiver(std::shared_ptr<AbstractReceiver> val)
{
    receiver_ = std::move(val);
}

bool SignalBinder::Bind(unsigned int id, QObject* sender, int signal_index)
{
    if (!sender || signal_index < 0)
        return false;

    auto new_binding = std::make_unique<Binding>(this, id, sender, signal_index);

    if (id < bindings_.size())
    {
        auto& old_binding = bindings_[id];
        if (old_binding && old_binding->Equals(*new_binding))
            return true;
        old_binding = std::move(new_binding);
    }
    else if (id == bindings_.size())
    {
        bindings_.emplace_back(std::move(new_binding));
    }
    else
    {
        bindings_.resize(id + 1);
        bindings_[id] = std::move(new_binding);
    }

    bindings_[id]->connection = QMetaObject::connect(sender, signal_index, this, id + kMethodIdOffset);
    return bindings_[id]->IsConnected();
}

bool SignalBinder::Bind(QObject* sender, const ObjectMetaData::RoleInfo* role, const Ids& notifiers)
{
    if (!sender || !role || notifiers.empty())
        return false;

    bool res = false;
    for (auto child : role->children)
    {
        bool child_res = false;
        if (child->meta_object)
            child_res = Bind(child->ReadFromItem(sender).value<QObject*>(), child, notifiers);

        if (child->notifier)
        {
            if (child_res || notifiers.contains(child->notifier->id))
            {
                if (Bind(child->notifier->id, sender, child->notifier->signal_index))
                {
                    child_res = true;
                }
            }
            else
            {
                Unbind(child->notifier->id);
            }
        }

        res = res || child_res;
    }

    return res;
}

void SignalBinder::Unbind(QObject* sender, std::optional<int> signal_index /*= {}*/)
{
    if (!sender && !signal_index)
        return;

    for (auto& binding : bindings_)
    {
        if (binding && (!sender || binding->sender == sender) && (!signal_index || binding->signal_index == *signal_index))
            binding.reset();
    }
}

void SignalBinder::Unbind(unsigned int id)
{
    if (id < 0 || id >= bindings_.size())
        return;

    bindings_[id].reset();
}

void SignalBinder::UnbindAll()
{
    bindings_.clear();
}

bool SignalBinder::HasBindings() const
{
    return std::any_of(bindings_.crbegin(), bindings_.crend(), [](const auto& binding) { return binding != nullptr; });
}

int SignalBinder::qt_metacall(QMetaObject::Call call, int method_id, void** args)
{
    auto binding_id = method_id - kMethodIdOffset;
    if (receiver_ && binding_id >= 0 && binding_id < bindings_.size() && bindings_[binding_id])
    {
        receiver_->OnSignalReceived(*bindings_[binding_id], args);
        return -1;
    }
    else
    {
        return QObject::qt_metacall(call, method_id, args);
    }
}
