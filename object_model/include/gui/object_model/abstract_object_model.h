#pragma once

#include "dynamic_roles.h"
#include "model_access.h"
#include <QAbstractListModel>
#include <QHash>
#include <QSet>
//#include <log_qt.h>
#include <memory>

namespace om
{
class AbstractObjectModel : public QAbstractListModel, public ListModelAccess, public AbstractDynamicRolesReceiver
{
    Q_OBJECT
    Q_PROPERTY(int count READ rowCount NOTIFY rowCountChanged)
    Q_PROPERTY(QVariant dataRoles READ GetDataRoles WRITE SetDataRoles NOTIFY dataRolesChanged)
    Q_PROPERTY(QVariant itemDataChangedRoles READ GetItemDataChangedRoles WRITE SetItemDataChangedRoles NOTIFY itemDataChangedRolesChanged)
    Q_PROPERTY(unsigned int itemDataChangedDelay READ GetItemDataChangedDelay WRITE SetItemDataChangedDelay FINAL)
    Q_PROPERTY(unsigned int itemDataChangedInterval READ GetItemDataChangedInterval WRITE SetItemDataChangedInterval FINAL)
public:
    static const int        kItemRole;
    static const QByteArray kItemRoleName;

    AbstractObjectModel(QObject* parent = nullptr);
    AbstractObjectModel(const QMetaObject* static_meta_object, QObject* parent = nullptr);
    AbstractObjectModel(const QMetaObject& static_meta_object, QObject* parent = nullptr);
    virtual ~AbstractObjectModel();

    // Meta data
    const QMetaObject* ItemMetaObject() const;
    QString            ItemClassName() const;
    bool               IsInitialized() const;

    // Data
    // if int role is not a role of a model, it will cause a crash
    // if QByteArray role_name is not a role of a model, invalid QVariant will be returned
    QVariant data(const QModelIndex& index, int role = Qt::DisplayRole) const override;
    QVariant data(const QModelIndex& index, const QByteArray& role_name) const;
    bool     setData(const QModelIndex& index, const QVariant& value, int role /* = Qt::EditRole */) override;
    bool     setData(const QModelIndex& index, const QVariant& value, const QByteArray& role_name);

    QVariant GetData(const QModelIndex& index, int role = Qt::DisplayRole) const;
    QVariant GetData(const QModelIndex& index, const QByteArray& role_name) const;
    QVariant GetData(int row, int role) const override;

    bool SetData(const QModelIndex& index, const QVariant& value, int role = Qt::DisplayRole);
    bool SetData(const QModelIndex& index, const QVariant& value, const QByteArray& role_name);
    bool SetData(int row, const QVariant& value, int role) override;

    bool IsNullRow(int row) const;

    // Overridden
    Id GetRoleId(const QByteArray& role_name) const override;

    QHash<int, QByteArray> roleNames() const override;
    QHash<QByteArray, int> roleIds() const override;

    // Roles that will be exposed to QML from roleNames()
    QVariant GetDataRoles() const;
    void     SetDataRoles(const QVariant& val);

    // Roles whose notifiers will be connected to itemDataChanged signal. If DynamicRolesProviders is set, this roles will be ignored
    // If set to DataRolesPolicy::ITEM_ROLE no roles will be connected.
    QVariant GetItemDataChangedRoles() const;
    void     SetItemDataChangedRoles(const QVariant& val);

    unsigned int GetItemDataChangedDelay() const;
    void         SetItemDataChangedDelay(unsigned int val);  // DelayType or any int
    unsigned int GetItemDataChangedInterval() const;
    void         SetItemDataChangedInterval(unsigned int val);

public slots:
    int      GetCount() const override;
    QVariant GetData(int row, const QByteArray& role_name = AbstractObjectModel::kItemRoleName) const override;
    bool     SetData(int row, const QVariant& value, const QByteArray& role_name = AbstractObjectModel::kItemRoleName) override;
    bool     SetData(int row, const QVariantMap& values);

signals:
    // Notifiers
    void initialized();
    void rowCountChanged();
    void dataRolesChanged();
    void itemDataChanged(const Ids& changed_roles);
    void itemDataChangedRolesChanged();
    void itemInstalled(QObject* item);
    void itemUninstalled(QObject* item);

protected:
    void timerEvent(QTimerEvent* event) override;

    virtual QVariant GetItemsProperty(QObject* item, int role) const;
    virtual QVariant GetItemsProperty(QObject* item, const QByteArray& role_name) const;
    //TODO: virtual bool     HasPropertiesMatch(QObject* item, const QVector<QPair<Id, QVariant>>& properties) const;
    virtual bool     HasPropertiesMatch(QObject* item, const QVariantMap& properties) const;
    virtual bool     SetItemsProperty(QObject* item, const QVariant& value, int role) const;
    virtual bool     SetItemsProperty(QObject* item, const QVariant& value, const QByteArray& role_name) const;

    virtual void ItemAboutToBeDeleted(QObject*) {};

    virtual QObject* GetItem(int row) const         = 0;
    virtual bool     SetItem(int row, QObject* val) = 0;

    void InitializeMetaObject(const QMetaObject* meta_object);
    void CheckItemsMetaObject(QObject* item);

    virtual void InstallItem(int row);
    void         InstallItems(int row, int count);
    virtual void UninstallItem(int row, bool taken = false);
    void         UninstallItems(int row, int count, bool taken = false);
    void         UninstallAllItems(bool taken = false);

    void UpdateDataRoles();
    void UpdateDataChangeRoles();
    void UpdateItemsConnections();

    void SetDynamicRoles(const Ids& val) override;

private:
//    COMPONENT_LOGGER("ui.aom");

    struct Impl;
    std::unique_ptr<Impl> d;
};
}  // namespace om
