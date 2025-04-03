#pragma once

#include "property_change_listener.h"

#include <QObject>
#include <QQmlListProperty>
#include <QQmlParserStatus>
#include <QVariant>

//#include <log_qt.h>

namespace om
{
class AbstractSettings;

class SettingsListener : public PropertyChangeListener
{
    Q_OBJECT
    Q_PROPERTY(QString category READ GetCategory WRITE SetCategory NOTIFY categoryChanged)
public:
    SettingsListener(QObject *parent = nullptr);

    QString GetCategory() const;
    void    SetCategory(const QString &val);

signals:
    void categoryChanged();

private:
    QString category_;
};

class AbstractSettings : public QObject, public QQmlParserStatus
{
    Q_OBJECT
    Q_INTERFACES(QQmlParserStatus)
    Q_PROPERTY(QString category READ GetCategory WRITE SetCategory FINAL)
    Q_PROPERTY(QString fileName READ GetFileName WRITE SetFileName FINAL)
    Q_PROPERTY(QQmlListProperty<om::SettingsListener> settingsListeners READ GetSettingsListenerQmlListProperty)
    Q_CLASSINFO("DefaultProperty", "settingsListeners")
public:
    AbstractSettings(QObject *parent = nullptr);
    ~AbstractSettings();

    QString GetCategory() const;
    void    SetCategory(const QString &val);

    QString GetFileName() const;
    void    SetFileName(const QString &val);

    const QVector<SettingsListener *> &GetListeners() const;
    void                               AddListener(SettingsListener *val);
    void                               RemoveListener(SettingsListener *val);
    void                               ClearListeners();

public slots:
    // public slots names starts with lower case to save compatibility with QSettings
    virtual QVariant value(const QString &key, const QVariant &default_value = QVariant()) = 0;
    virtual void     setValue(const QString &key, const QVariant &value)                   = 0;
    virtual void     sync()                                                                = 0;
    virtual void     beginGroup(const QString &prefix)                                     = 0;
    virtual void     endGroup()                                                            = 0;

protected:
    void classBegin() override{};
    void componentComplete() override;

    virtual void ResetStorage() = 0;

    void Load();
    void Save();

private:
    QQmlListProperty<SettingsListener> GetSettingsListenerQmlListProperty();
    static void                        ListAppend(QQmlListProperty<SettingsListener> *list, SettingsListener *val);
    static int                         ListCount(QQmlListProperty<SettingsListener> *list);
    static SettingsListener *          ListGet(QQmlListProperty<SettingsListener> *list, int index);
    static void                        ListClear(QQmlListProperty<SettingsListener> *list);

    class Impl;
    std::unique_ptr<Impl> d;
};
}  // namespace om
