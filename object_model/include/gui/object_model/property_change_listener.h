#pragma once

#include "roles.h"

#include <QObject>
#include <memory>

namespace om
{
class PropertyChangeListener : public QObject
{
    Q_OBJECT
    Q_PROPERTY(QObject* target READ GetTarget WRITE SetTarget NOTIFY targetChanged)
    Q_PROPERTY(QVariant roles READ GetRoles WRITE SetRoles NOTIFY rolesChanged)
    Q_PROPERTY(bool enabled READ IsEnabled WRITE SetEnabled NOTIFY enabledChanged)
    Q_PROPERTY(bool listening READ IsListening NOTIFY listeningChanged)
    Q_PROPERTY(unsigned int propertiesChangedDelay READ GetPropertiesChangedDelay WRITE SetPropertiesChangedDelay FINAL)
    Q_PROPERTY(unsigned int propertiesChangedInterval READ GetPropertiesChangedInterval WRITE SetPropertiesChangedInterval FINAL)
public:
    PropertyChangeListener(QObject* parent = nullptr);
    PropertyChangeListener(QObject* target, QObject* parent);
    ~PropertyChangeListener();

    QObject* GetTarget() const;
    void     SetTarget(QObject* val);

    QVariant GetRoles() const;
    Ids      GetRoleIds() const;
    void     SetRoles(const QVariant& val);

    bool IsEnabled() const;
    void SetEnabled(bool val);

    bool IsListening() const;

    unsigned int GetPropertiesChangedDelay() const;
    void         SetPropertiesChangedDelay(unsigned int val);  // DelayType or any int
    unsigned int GetPropertiesChangedInterval() const;
    void         SetPropertiesChangedInterval(unsigned int val);

    int      GetPropertyId(const QString& name) const;
    QString  GetPropertyName(int role_id) const;
    QVariant Read(int role_id) const;
    QVariant Read(const QString& name) const;
    bool     Write(int role_id, const QVariant& val);
    bool     Write(const QString& name, const QVariant& val);

signals:
    void targetChanged();
    void rolesChanged();
    void enabledChanged();
    void listeningChanged();
    void propertiesChanged(const Ids&);

private:
    void timerEvent(QTimerEvent* event) override;

    struct Impl;
    std::unique_ptr<Impl> d;
};
}  // namespace om
