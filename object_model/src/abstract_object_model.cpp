#include "abstract_object_model.h"
#include "signal_binder.h"
#include "signal_timer.h"
#include <QBasicTimer>
#include <QMetaProperty>
#include <QPointer>
#include <QTimerEvent>
#include <algorithm>
#include <stdexcept>
#include <iostream>

using namespace om;

namespace
{
QString RoleNameToCamelCase(const QString& s)
{
    auto parts = s.split('.', Qt::SkipEmptyParts);
    for (int i = 1; i < parts.size(); ++i) parts[i].replace(0, 1, parts[i][0].toUpper());
    return parts.join("");
}
}  // namespace

// Impl
struct AbstractObjectModel::Impl
{
    AbstractObjectModel* model = nullptr;

    QString                         class_name;
    const QMetaObject*              static_meta_object = nullptr;
    std::shared_ptr<ObjectMetaData> meta_data;

    Ids                    changed_roles;
    Ids                    dynamic_roles;
    Ids                    dynamic_roles_notifiers;
    bool                   update_item_connections_queued = false;
    QVariant               data_roles                     = Role::ITEM_ROLE;
    QVariant               data_change_roles              = Role::ITEM_ROLE;
    QHash<int, QByteArray> role_names                     = { { kItemRole, kItemRoleName } };

    QHash<QObject*, SignalBinder*>                  item_connections;
    std::shared_ptr<SignalBinder::AbstractReceiver> connections_receiver;

    SignalTimer data_changed_timer;

    Impl(AbstractObjectModel* model) : model(model)
    {
        auto connections_callback = [=](const SignalBinder::Binding& binding, void**) { OnPropertyChanged(binding); };
        connections_receiver      = std::make_shared<SignalBinder::Receiver<decltype(connections_callback)>>(connections_callback);
        data_changed_timer.Initialize(model, [=] { EmitItemDataChanged(); });
    }

    bool ConnectItem(QObject* item, SignalBinder* connections = nullptr, const ObjectMetaData::RoleInfo* role = nullptr)
    {
        if (!(item || connections) || dynamic_roles_notifiers.empty())
            return false;

        if (!role)
        {
            role = meta_data->GetItemRoleInfo();
        }

        if (!connections)
        {
            auto connections_it = item_connections.find(item);
            if (connections_it == item_connections.end())
                throw std::logic_error("Can not find item connections");
            connections = connections_it.value();
        }

        return connections->Bind(item, role, dynamic_roles_notifiers);
    }

    void OnPropertyChanged(const SignalBinder::Binding& binding)
    {
        bool enqueue_signal = false;
        auto notifier       = meta_data->GetNotifier(binding.id);
        for (auto role : notifier->roles)
        {
            if (SetIntersection(dynamic_roles, role->dependent_role_ids, changed_roles))
            {
                enqueue_signal = true;
                if (role->meta_object)
                    ConnectItem(role->ReadFromItem(binding.sender).value<QObject*>(), binding.binder, role);
            }
        }
        if (enqueue_signal)
            EnqueueItemDataChanged();
    }

    void EmitItemDataChanged()
    {
        model->itemDataChanged(changed_roles);
        changed_roles.clear();
    }

    void EnqueueItemDataChanged() { data_changed_timer.Start(); }

    void OnTimerEvent(QTimerEvent* event)
    {
        if (event->timerId() == data_changed_timer.GetTimerId())
            OnDataChangeTimer();
    }

    void OnDataChangeTimer()
    {
        if (changed_roles.empty())
            data_changed_timer.Stop();
        else
            data_changed_timer.OnTimerEvent();
    }
};

// AbstractObjectModel
const int        AbstractObjectModel::kItemRole     = Qt::UserRole + 1;
const QByteArray AbstractObjectModel::kItemRoleName = "item";

AbstractObjectModel::AbstractObjectModel(QObject* parent /*= nullptr*/) : QAbstractListModel(parent), d(std::make_unique<Impl>(this))
{
    connect(this, &AbstractObjectModel::rowsInserted, this, &AbstractObjectModel::rowCountChanged);
    connect(this, &AbstractObjectModel::rowsRemoved, this, &AbstractObjectModel::rowCountChanged);
    connect(this, &AbstractObjectModel::modelReset, this, &AbstractObjectModel::rowCountChanged);
    connect(this, &AbstractObjectModel::rowCountChanged, [this] {
        if (!rowCount())
        {
            d->class_name = QString();
            d->meta_data  = nullptr;
        }
    });
}

AbstractObjectModel::AbstractObjectModel(const QMetaObject* static_meta_object, QObject* parent /*= nullptr*/) : AbstractObjectModel(parent)
{
    d->static_meta_object = static_meta_object;
    InitializeMetaObject(static_meta_object);
}

AbstractObjectModel::AbstractObjectModel(const QMetaObject& static_meta_object, QObject* parent /*= nullptr*/) : AbstractObjectModel(parent)
{
    d->static_meta_object = &static_meta_object;
    InitializeMetaObject(&static_meta_object);
}

AbstractObjectModel::~AbstractObjectModel() = default;

const QMetaObject* AbstractObjectModel::ItemMetaObject() const
{
    return d->static_meta_object ? d->static_meta_object : (rowCount() ? GetItem(0)->metaObject() : nullptr);
}

QString AbstractObjectModel::ItemClassName() const
{
    return d->class_name;
}

bool AbstractObjectModel::IsInitialized() const
{
    return d->meta_data != nullptr;
}

// GetData
QVariant AbstractObjectModel::GetData(int row, int role) const
{
    if (!d->meta_data || row < 0 || row >= rowCount())
        return QVariant();

    if (role == kItemRole)
        return QVariant::fromValue(GetItem(row));

    return GetItemsProperty(GetItem(row), role);
}

QVariant AbstractObjectModel::data(const QModelIndex& index, int role /*= Qt::DisplayRole*/) const
{
    return GetData(index, role);
}

QVariant AbstractObjectModel::data(const QModelIndex& index, const QByteArray& role_name) const
{
    return GetData(index, role_name);
}

QVariant AbstractObjectModel::GetData(const QModelIndex& index, int role /*= Qt::DisplayRole*/) const
{
    return GetData(index.row(), role);
}

QVariant AbstractObjectModel::GetData(const QModelIndex& index, const QByteArray& role_name) const
{
    return GetData(index.row(), role_name);
}

QVariant AbstractObjectModel::GetData(int row, const QByteArray& role_name) const
{
    return ListModelAccess::GetData(row, role_name);
}

// SetData
bool AbstractObjectModel::SetData(int row, const QVariant& value, int role)
{
    if (!d->meta_data || row < 0 || row >= rowCount())
        return false;

    if (role == AbstractObjectModel::kItemRole)
        return SetItem(row, value.value<QObject*>());

    return SetItemsProperty(GetItem(row), value, role);
}

bool AbstractObjectModel::setData(const QModelIndex& index, const QVariant& value, int role /* = Qt::EditRole */)
{
    return SetData(index.row(), value, role);
}

bool AbstractObjectModel::setData(const QModelIndex& index, const QVariant& value, const QByteArray& role_name)
{
    return SetData(index.row(), value, role_name);
}

bool AbstractObjectModel::SetData(const QModelIndex& index, const QVariant& value, int role /*= Qt::DisplayRole*/)
{
    return SetData(index.row(), value, role);
}

bool AbstractObjectModel::SetData(const QModelIndex& index, const QVariant& value, const QByteArray& role_name)
{
    return SetData(index.row(), value, role_name);
}

bool AbstractObjectModel::SetData(int row, const QVariant& value, const QByteArray& role_name /*= AbstractObjectModel::kItemRoleName*/)
{
    return ListModelAccess::SetData(row, value, role_name);
}

bool AbstractObjectModel::SetData(int row, const QVariantMap& values)
{
    auto res = true;
    for (auto role = values.begin(); role != values.end(); ++role) res &= SetData(row, role.value(), role.key().toUtf8());
    return res;
}

QVariant AbstractObjectModel::GetItemsProperty(QObject* item, const QByteArray& role_name) const
{
    return d->meta_data->GetRoleInfo(role_name)->ReadFromRoot(item);
}

QVariant AbstractObjectModel::GetItemsProperty(QObject* item, int role) const
{
    return d->meta_data->GetRoleInfo(role)->ReadFromRoot(item);
}

bool AbstractObjectModel::HasPropertiesMatch(QObject* item, const QVariantMap& properties) const
{
    for (auto p = properties.begin(); p != properties.end(); ++p)
    {
        if (GetItemsProperty(item, p.key().toUtf8()) != p.value())
            return false;
    }
    return true;
}

bool AbstractObjectModel::SetItemsProperty(QObject* item, const QVariant& value, int role) const
{
    return d->meta_data->GetRoleInfo(role)->WriteToRoot(item, value);
}

bool AbstractObjectModel::SetItemsProperty(QObject* item, const QVariant& value, const QByteArray& role_name) const
{
    return d->meta_data->GetRoleInfo(role_name)->WriteToRoot(item, value);
}

bool AbstractObjectModel::IsNullRow(int row) const
{
    return GetItem(row) == nullptr;
}

// Meta object
void AbstractObjectModel::InitializeMetaObject(const QMetaObject* meta_object)
{
    d->class_name = meta_object->className();
    d->meta_data  = ObjectMetaData::GetMetaData(meta_object);
    UpdateDataRoles();
    UpdateDataChangeRoles();
    emit initialized();
}

void AbstractObjectModel::CheckItemsMetaObject(QObject* item)
{
    if (!item)
        return;

    if (d->class_name.isEmpty())
    {
        InitializeMetaObject(item->metaObject());
    }
    auto meta_object = item->metaObject();
    while (meta_object)
        if (meta_object->className() == d->class_name)
            break;
        else
            meta_object = meta_object->superClass();

    if (!meta_object)
    {
        auto error_message = QString("Trying to add object with type %1 to model with type %2").arg(item->metaObject()->className()).arg(d->class_name).toStdString();
        std::cout << (std::invalid_argument(error_message), error_message);
    }
}

// Item management
void AbstractObjectModel::InstallItem(int row)
{
    auto item = GetItem(row);
    if (!item)
        return;

    CheckItemsMetaObject(item);
    auto& connections = d->item_connections[item];
    if (connections)
    {
        auto error_message = QString("Item at row = %1 already installed").arg(row).toStdString();
        std::cout << (std::logic_error(error_message), error_message);
    }
    connections = new SignalBinder(d->connections_receiver);

    connect(item, &QObject::destroyed, this, &AbstractObjectModel::ItemAboutToBeDeleted);
    d->ConnectItem(item, connections);

    emit itemInstalled(item);
}

void AbstractObjectModel::InstallItems(int row, int count)
{
    for (int i = row; i < row + count; ++i) InstallItem(i);
}

void AbstractObjectModel::UninstallItem(int row, bool)
{
    auto item = GetItem(row);
    if (!item)
        return;

    disconnect(item, 0, this, 0);

    auto connections = d->item_connections.find(item);
    if (connections == d->item_connections.end())
    {
        auto error_message = QString("Item at row = %1 already uninstalled").arg(row).toStdString();
        std::cout << (std::logic_error(error_message), error_message);
    }
    delete connections.value();
    d->item_connections.erase(connections);

    emit itemUninstalled(item);
}

void AbstractObjectModel::UninstallItems(int row, int count, bool taken)
{
    for (int i = row; i < row + count; ++i) UninstallItem(i, taken);
}

void AbstractObjectModel::UninstallAllItems(bool taken /*= false*/)
{
    UninstallItems(0, rowCount(), taken);
}

// Roles
QHash<int, QByteArray> AbstractObjectModel::roleNames() const
{
    return d->role_names;
}

QHash<QByteArray, int> AbstractObjectModel::roleIds() const
{
    return d->meta_data ? d->meta_data->GetRoleIds() : QHash<QByteArray, int>{};
}

om::Id AbstractObjectModel::GetRoleId(const QByteArray& role_name) const
{
    return d->meta_data ? d->meta_data->GetRoleId(role_name) : kInvalidId;
}

// data roles policy
QVariant AbstractObjectModel::GetDataRoles() const
{
    return d->data_roles;
}

void AbstractObjectModel::SetDataRoles(const QVariant& val)
{
    if (d->data_roles == val)
        return;
    d->data_roles = val;
    UpdateDataRoles();
    emit dataRolesChanged();
}

void AbstractObjectModel::UpdateDataRoles()
{
    if (!IsInitialized())
        return;
    d->role_names = ObjectMetaData::kDefaultRoleNames;
    auto role_ids = d->meta_data->ConvertToRoleIds(d->data_roles);
    for (auto role_id : role_ids)
    {
        d->role_names[role_id] = RoleNameToCamelCase(d->meta_data->GetRoleInfo(role_id)->name).toUtf8();
    }
}

// item data change roles
QVariant AbstractObjectModel::GetItemDataChangedRoles() const
{
    return d->data_change_roles;
}

void AbstractObjectModel::SetItemDataChangedRoles(const QVariant& val)
{
    if (d->data_change_roles == val)
        return;
    d->data_change_roles = val;
    UpdateDataChangeRoles();
    emit itemDataChangedRolesChanged();
}

void AbstractObjectModel::UpdateDataChangeRoles()
{
    if (!IsInitialized() || dynamic_roles_providers_.size())
        return;
    SetDynamicRoles(d->meta_data->ConvertToRoleIds(d->data_change_roles));
}

unsigned int AbstractObjectModel::GetItemDataChangedDelay() const
{
    return d->data_changed_timer.GetDelay();
}

void AbstractObjectModel::SetItemDataChangedDelay(unsigned int val)
{
    d->data_changed_timer.SetDelay(val);
}

unsigned int AbstractObjectModel::GetItemDataChangedInterval() const
{
    return d->data_changed_timer.GetInterval();
}

void AbstractObjectModel::SetItemDataChangedInterval(unsigned int val)
{
    d->data_changed_timer.SetInterval(val);
}

// dynamic roles
void AbstractObjectModel::SetDynamicRoles(const Ids& val)
{
    d->dynamic_roles = val;
    d->dynamic_roles_notifiers.clear();
    for (auto role : d->dynamic_roles)
    {
        if (auto notifier = d->meta_data->GetRoleInfo(role)->notifier)
            d->dynamic_roles_notifiers.insert(notifier->id);
    }
    UpdateItemsConnections();
}

void AbstractObjectModel::UpdateItemsConnections()
{
    if (d->update_item_connections_queued)
        return;
    d->update_item_connections_queued = true;
    QMetaObject::invokeMethod(
        this,
        [this] {
            for (int i = 0; i < rowCount(); ++i) d->ConnectItem(GetItem(i));
            d->update_item_connections_queued = false;
        },
        Qt::QueuedConnection);
}

void AbstractObjectModel::timerEvent(QTimerEvent* event)
{
    d->OnTimerEvent(event);
}

// row count
int AbstractObjectModel::GetCount() const
{
    return rowCount();
}
