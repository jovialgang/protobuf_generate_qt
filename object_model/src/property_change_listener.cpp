#include "property_change_listener.h"

#include "property_binder.h"
#include "signal_timer.h"

//#include <log_qt.h>

#include <QBasicTimer>
#include <QTimerEvent>

using namespace om;

struct PropertyChangeListener::Impl
{
//    COMPONENT_LOGGER("ui.pchl");

    PropertyChangeListener* listener = nullptr;
    PropertyBinder          binder;
    QObject*                target = nullptr;
    QVariant                roles;
    bool                    listening = false;
    Ids                 changed_properties;
    SignalTimer             properties_changed_timer;

    Impl(PropertyChangeListener* listener) : listener(listener)
    {
        auto callback = [=](Ids&& properties) { OnPropertiesChanged(std::move(properties)); };
        binder.SetReceiver(std::make_shared<PropertyBinder::Receiver<decltype(callback)>>(callback));
        properties_changed_timer.Initialize(listener, [=] { EmitPropertiesChanged(); });
    }

    // Emit last changed properties
    ~Impl()
    {
        if (!changed_properties.empty())
            EmitPropertiesChanged();
    }

    void SetTarget(QObject* val)
    {
        if (target == val)
            return;

        if (target)
            target->disconnect(listener);

        target = val;

        if (target)
            connect(target, &QObject::destroyed, listener, [=] { SetTarget(nullptr); });

        UpdateBinding();
        listener->targetChanged();
    }

    void SetRoles(const QVariant& val)
    {
        if (roles == val)
            return;
        roles = val;
        UpdateBinding();
        listener->rolesChanged();
    }

    void UpdateBinding()
    {
        binder.Bind(target, roles);
        SetListening(binder.IsBound());
    }

    void SetEnabled(bool val)
    {
        if (val == binder.IsEnabled())
            return;
        binder.SetEnabled(val);
        listener->enabledChanged();

        SetListening(binder.IsBound());
    }

    void SetListening(bool val)
    {
        if (val == listening)
            return;

        listening = val;
        listener->listeningChanged();
    }

    void OnTimerEvent(QTimerEvent* event)
    {
        if (event->timerId() == properties_changed_timer.GetTimerId())
            OnPropertiesChangedTimer();
    }

    void OnPropertiesChanged(Ids&& properties)
    {
        changed_properties.merge(properties);
        EnqueuePropertiesChanged();
    }

    void EmitPropertiesChanged()
    {
        listener->propertiesChanged(changed_properties);
        changed_properties.clear();
    }

    // Starts signal timer
    void EnqueuePropertiesChanged() { properties_changed_timer.Start(); }

    // Timer event
    void OnPropertiesChangedTimer()
    {
        if (changed_properties.empty())
            properties_changed_timer.Stop();
        else
            properties_changed_timer.OnTimerEvent();
    }
};

PropertyChangeListener::PropertyChangeListener(QObject* parent /*= nullptr*/) : QObject(parent), d(std::make_unique<Impl>(this))
{}

PropertyChangeListener::PropertyChangeListener(QObject* target, QObject* parent) : PropertyChangeListener(parent)
{
    SetTarget(target);
}

PropertyChangeListener::~PropertyChangeListener()
{}

QObject* PropertyChangeListener::GetTarget() const
{
    return d->target;
}

void PropertyChangeListener::SetTarget(QObject* val)
{
    d->SetTarget(val);
}

QVariant PropertyChangeListener::GetRoles() const
{
    return d->roles;
}

void PropertyChangeListener::SetRoles(const QVariant& val)
{
    d->SetRoles(val);
}

bool PropertyChangeListener::IsEnabled() const
{
    return d->binder.IsEnabled();
}

void PropertyChangeListener::SetEnabled(bool val)
{
    d->SetEnabled(val);
}

Ids PropertyChangeListener::GetRoleIds() const
{
    return d->binder.GetProperties();
}

bool PropertyChangeListener::IsListening() const
{
    return d->listening;
}

unsigned int PropertyChangeListener::GetPropertiesChangedDelay() const
{
    return d->properties_changed_timer.GetDelay();
}

void PropertyChangeListener::SetPropertiesChangedDelay(unsigned int val)
{
    d->properties_changed_timer.SetDelay(val);
}

unsigned int PropertyChangeListener::GetPropertiesChangedInterval() const
{
    return d->properties_changed_timer.GetInterval();
}

void PropertyChangeListener::SetPropertiesChangedInterval(unsigned int val)
{
    d->properties_changed_timer.SetInterval(val);
}

int PropertyChangeListener::GetPropertyId(const QString& name) const
{
    return d->binder.GetPropertyId(name);
}

QString PropertyChangeListener::GetPropertyName(int role_id) const
{
    return d->binder.GetPropertyName(role_id);
}

QVariant PropertyChangeListener::Read(int role_id) const
{
    return d->binder.Read(role_id);
}

QVariant PropertyChangeListener::Read(const QString& name) const
{
    return Read(GetPropertyId(name));
}

bool PropertyChangeListener::Write(int role_id, const QVariant& val)
{
    return d->binder.Write(role_id, val);
}

bool PropertyChangeListener::Write(const QString& name, const QVariant& val)
{
    return Write(GetPropertyId(name), val);
}

void PropertyChangeListener::timerEvent(QTimerEvent* event)
{
    d->OnTimerEvent(event);
}
