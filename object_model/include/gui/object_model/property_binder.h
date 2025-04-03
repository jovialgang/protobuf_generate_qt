#pragma once

#include "signal_binder.h"

#include <QPointer>
#include <QVariant>

namespace om
{
class PropertyBinder
{
public:
    class AbstractReceiver : public SignalBinder::AbstractReceiver
    {
    public:
        void         OnSignalReceived(const SignalBinder::Binding& binding, void** args) override final;
        virtual void OnPropertiesChanged(Ids&& properties) = 0;

    private:
        friend PropertyBinder;

        PropertyBinder* parent_ = nullptr;
    };

    template <typename Functor>
    class Receiver : public AbstractReceiver
    {
    public:
        Receiver(Functor functor) : AbstractReceiver(), functor_(std::move(functor)) {}

        void OnPropertiesChanged(Ids&& properties) override { functor_(std::move(properties)); }

    private:
        Functor functor_;
    };

    PropertyBinder(std::shared_ptr<AbstractReceiver> receiver = nullptr);

    std::shared_ptr<AbstractReceiver> GetReceiver() const;
    void                              SetReceiver(std::shared_ptr<AbstractReceiver> val);

    bool IsBound() const;

    void Bind(QObject* sender, const Ids& properties);
    void Bind(QObject* sender, const QVariant& properties);

    void Reset();

    QObject* GetSender() const;
    Ids  GetProperties() const;

    // disables/enables all bindings
    bool IsEnabled() const;
    void SetEnabled(bool val);

    // works when sender is set, otherwise will throw an exception
    int      GetPropertyId(const QString& name) const;
    QString  GetPropertyName(int role_id) const;
    QVariant Read(int role_id) const;
    bool     Write(int role_id, const QVariant& val);

private:
    void SetProperties(const Ids& role_ids);
    void SetSender(QObject* sender);

    void UpdateBinding();

    Ids OnSignalReceived(const SignalBinder::Binding& binding, void** args);

    QObject*                        sender_ = nullptr;
    std::shared_ptr<ObjectMetaData> senders_meta_data_;
    Ids                             role_ids_;
    Ids                             notifier_ids_;
    bool                            enabled_ = true;
    SignalBinder                    binder_;
};
}  // namespace om
