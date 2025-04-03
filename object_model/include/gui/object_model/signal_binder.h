#pragma once

#include "object_meta_data.h"

#include <QObject>
#include <QPointer>
#include <memory>
#include <numeric>
#include <optional>
#include <vector>

namespace om
{
class SignalBinder : public QObject
{
    // No Q_OBJECT macro, because we need custom qt_metacall
public:
    class AbstractReceiver;

    struct Binding
    {
        Binding(SignalBinder* const binder, unsigned int id, QObject* sender, int signal_index) : binder(binder), id(id), sender(sender), signal_index(signal_index) {}

        ~Binding()
        {
            if (connection)
                QObject::disconnect(connection);
        }

        SignalBinder* const     binder;
        unsigned int            id           = std::numeric_limits<unsigned int>::max();
        QPointer<QObject>       sender       = nullptr;
        int                     signal_index = -1;
        QMetaObject::Connection connection;

        bool IsConnected() const { return connection; }
        bool Equals(const Binding& val) const { return signal_index == val.signal_index && sender == val.sender; }

        QString DebugString() const
        {
            return QString("Binding { id = %1, sender = %2, signal_index = %3, connected = %4 }")
                .arg(id)
                .arg(QVariant::fromValue(sender).toString())
                .arg(signal_index)
                .arg(IsConnected());
        }
    };

    class AbstractReceiver
    {
    public:
        virtual ~AbstractReceiver() = default;

        virtual void OnSignalReceived(const Binding& binding, void** args) = 0;
    };

    template <typename Functor>
    class Receiver : public AbstractReceiver
    {
    public:
        Receiver(Functor functor) : AbstractReceiver(), functor_(std::move(functor)) {}

        void OnSignalReceived(const Binding& binding, void** args) override { functor_(binding, args); }

    private:
        Functor functor_;
    };

    SignalBinder(std::shared_ptr<AbstractReceiver> receiver = nullptr);

    std::shared_ptr<AbstractReceiver> GetReceiver() const;
    void                              SetReceiver(std::shared_ptr<AbstractReceiver> val);

    // It's better to increment id or use fixed range of ids from 0, because bindings stored in vector that resizes to max_id + 1
    bool Bind(unsigned int id, QObject* sender, int signal_index);
    // Binds senders notifiers. Bindings id is equal to corresponding notifier id
    bool Bind(QObject* sender, const ObjectMetaData::RoleInfo* root_role, const Ids& notifiers);
    void Unbind(unsigned int id);
    void Unbind(QObject* sender, std::optional<int> signal_index = {});
    void UnbindAll();

    bool HasBindings() const;

    virtual int qt_metacall(QMetaObject::Call call, int method_id, void** args) override;

private:
    std::vector<std::unique_ptr<Binding>> bindings_;
    std::shared_ptr<AbstractReceiver>     receiver_;
};
}  // namespace om
