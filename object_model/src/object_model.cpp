#include "object_model.h"
#include "settings.h"
#include "signal_timer.h"
#include <QtQml>

using namespace om;

ObjectRefModel::ObjectRefModel(QObject* parent /*= nullptr*/) : AbstractObjectModel(parent)
{}

ObjectRefModel::ObjectRefModel(const QMetaObject& meta_object, QObject* parent /*= nullptr*/) : AbstractObjectModel(meta_object, parent)
{}

ObjectRefModel::ObjectRefModel(const QMetaObject* meta_object, QObject* parent /*= nullptr*/) : AbstractObjectModel(meta_object, parent)
{}

ObjectRefModel::~ObjectRefModel()
{
    UninstallAllItems();
}

QStringList ObjectRefModel::GetItemNames() const
{
    QStringList res;
    res.reserve(items_.size());
    for (auto item : items_) res.push_back(item->objectName());
    return res;
}

QObject* ObjectRefModel::GetItem(int row) const
{
    return items_[row];
}

bool om::ObjectRefModel::SetItem(int row, QObject* val)
{
    if (row < 0 || row >= items_.size() || !CanAddItem(val))
        return false;
    // disconnecting old item
    UninstallItem(row);
    // placing new item
    items_[row] = val;
    InstallItem(row);

    emit dataChanged(index(row), index(row));
    return true;
}

bool ObjectRefModel::CanAddItem(QObject* item) const
{
    return item != nullptr;
}

bool ObjectRefModel::CanAddItems(const QVector<QObject*>& items) const
{
    for (auto item : items)
        if (!CanAddItem(item))
            return false;
    return true;
}

void ObjectRefModel::InstallItem(int row)
{
    try
    {
        AbstractObjectModel::InstallItem(row);
    }
    catch (const std::exception& e)
    {
        items_.remove(row);
//        LOG(e.what());
        throw;
    }
}

bool ObjectRefModel::removeRows(int row, int count, const QModelIndex& /*= QModelIndex()*/)
{
    if (count < 1 || row < 0 || row + count > items_.size())
        return false;

    beginRemoveRows(QModelIndex(), row, row + count - 1);
    // disconnecting old items
    UninstallItems(row, count);
    items_.remove(row, count);
    endRemoveRows();

    return true;
}

bool ObjectRefModel::moveRows(const QModelIndex& source_parent, int source_row, int count, const QModelIndex& destination_parent, int destination_row)
{
    if (!(source_row >= 0 && source_row + count <= rowCount() && destination_row >= 0 && destination_row + count <= rowCount() && qAbs(source_row - destination_row) >= count))
        return false;

    if (beginMoveRows(source_parent, source_row, source_row + count - 1, destination_parent, destination_row + (source_row < destination_row)))
    {
        for (int i = count - 1; i >= 0; --i) items_.move(source_row + i, destination_row);
        endMoveRows();
    }
    return true;
}

QObject* ObjectRefModel::At(int row) const
{
    return row >= 0 && row < rowCount() ? items_[row] : nullptr;
}

QObject* ObjectRefModel::First() const
{
    return At(0);
}

QObject* ObjectRefModel::Last() const
{
    return At(rowCount() - 1);
}

// Search
int ObjectRefModel::IndexOf(QObject* item) const
{
    return item ? items_.indexOf(item) : -1;
}

int ObjectRefModel::IndexOf(const QString& object_name) const
{
    for (int i = 0; i < items_.size(); ++i)
        if (items_[i]->objectName() == object_name)
            return i;
    return -1;
}

int ObjectRefModel::IndexOf(const QByteArray& property_name, const QVariant& val) const
{
    auto role_id = GetRoleId(property_name);
    if (role_id == kInvalidId)
        return -1;

    for (int i = 0; i < items_.size(); ++i)
        if (GetItemsProperty(items_[i], role_id) == val)
            return i;
    return -1;
}

int ObjectRefModel::IndexOf(const QVariantMap& props) const
{
    if (props.isEmpty())
        return -1;
    for (int i = 0; i < items_.size(); ++i)
        if (HasPropertiesMatch(items_[i], props))
            return i;
    return -1;
}

bool ObjectRefModel::Contains(QObject* item) const
{
    return IndexOf(item) >= 0;
}

bool ObjectRefModel::Contains(const QString& object_name) const
{
    return IndexOf(object_name) >= 0;
}

bool ObjectRefModel::Contains(const QByteArray& property_name, const QVariant& val) const
{
    return IndexOf(property_name, val) >= 0;
}

bool ObjectRefModel::Contains(const QVariantMap& props) const
{
    return IndexOf(props) >= 0;
}

QVector<int> ObjectRefModel::IndexesOf(const QByteArray& property_name, const QVariant& val) const
{
    QVector<int> res;

    auto role_id = GetRoleId(property_name);
    if (role_id == kInvalidId)
        return res;

    for (int i = 0; i < items_.size(); ++i)
        if (GetItemsProperty(items_[i], role_id) == val)
            res.push_back(i);
    return res;
}

QVector<int> ObjectRefModel::IndexesOf(const QVariantMap& props) const
{
    QVector<int> res;
    if (props.isEmpty())
        return res;
    for (int i = 0; i < items_.size(); ++i)
        if (HasPropertiesMatch(items_[i], props))
            res.push_back(i);
    return res;
}

QObject* ObjectRefModel::FindFirst(const QByteArray& property_name, const QVariant& val) const
{
    for (auto item : items_)
        if (GetItemsProperty(item, property_name) == val)
            return item;
    return nullptr;
}

QObject* ObjectRefModel::FindFirst(const QVariantMap& props) const
{
    if (props.isEmpty())
        return nullptr;
    for (auto i : items_)
        if (HasPropertiesMatch(i, props))
            return i;
    return nullptr;
}

QObject* ObjectRefModel::Find(const QString& object_name) const
{
    for (auto i : items_)
        if (i->objectName() == object_name)
            return i;
    return nullptr;
}

QVector<QObject*> ObjectRefModel::FindObjects(const QStringList& object_names) const
{
    QVector<QObject*> res;
    if (object_names.isEmpty())
        return res;

    for (auto object_name : object_names)
        for (auto i : items_)
            if (i->objectName() == object_name)
            {
                res.push_back(i);
                break;
            }
    return res;
}

QVector<QObject*> ObjectRefModel::FindObjects(const QString& object_name) const
{
    QVector<QObject*> res;
    for (auto i : items_)
        if (i->objectName() == object_name)
            res.push_back(i);
    return res;
}

QVector<QObject*> ObjectRefModel::Find(const QByteArray& property_name, const QVariant& val) const
{
    QVector<QObject*> res;
    for (auto item : items_)
        if (GetItemsProperty(item, property_name) == val)
            res.push_back(item);
    return res;
}

QVector<QObject*> ObjectRefModel::Find(const QVariantMap& props) const
{
    QVector<QObject*> res;
    if (props.isEmpty())
        return res;
    for (auto i : items_)
        if (HasPropertiesMatch(i, props))
            res.push_back(i);
    return res;
}

// Take
QObject* ObjectRefModel::Take(int row)
{
    if (row < 0 || row >= items_.size())
        return nullptr;
    beginRemoveRows(QModelIndex(), row, row);
    UninstallItem(row, true);
    auto item = row == items_.size() - 1 ? items_.takeLast() : items_.takeAt(row);
    endRemoveRows();
    return item;
}

QObject* ObjectRefModel::TakeFirst()
{
    return Take(0);
}

QObject* ObjectRefModel::TakeLast()
{
    return Take(items_.size() - 1);
}

QObject* ObjectRefModel::Append(QObject* item)
{
    return Insert(items_.size(), item);
}

QObject* ObjectRefModel::Append(const QVariantMap& props)
{
    auto row  = items_.size();
    auto item = Insert(row);
    SetData(row, props);
    return item;
}

void ObjectRefModel::AppendVector(const QVector<QObject*>& items)
{
    if (items.isEmpty() || !CanAddItems(items))
        return;
    auto row = items_.size();
    beginInsertRows(QModelIndex(), row, row + items.size() - 1);
    items_.append(items);
    InstallItems(row, items.size());
    endInsertRows();
}

void ObjectRefModel::AppendModel(ObjectRefModel* m)
{
    if (!m->ItemMetaObject() || !ItemMetaObject() || m->ItemMetaObject()->className() != ItemMetaObject()->className())
        throw std::invalid_argument("Trying to add object model not of the item class");
    AppendVector(m->items_);
}

QObject* ObjectRefModel::Insert(int row, QObject* item)
{
    if (row < 0 || row > items_.size() || !CanAddItem(item))
        return nullptr;
    beginInsertRows(QModelIndex(), row, row);
    if (row == items_.size())
        items_.append(item);
    else
        items_.insert(row, item);
    InstallItem(row);
    endInsertRows();
    return items_[row];
}

QObject* ObjectRefModel::Set(int row, QObject* item)
{
    return SetItem(row, item) ? items_[row] : nullptr;
}

void ObjectRefModel::Remove(int row, int count)
{
    removeRows(row, count);
}

void ObjectRefModel::Remove(QObject* item)
{
    auto index = IndexOf(item);
    if (index < 0)
        return;
    Remove(index);
}

void ObjectRefModel::Remove(const QString& object_name)
{
    auto index = IndexOf(object_name);
    if (index < 0)
        return;
    Remove(index);
}

void ObjectRefModel::RemoveAll(const QByteArray& property_name, const QVariant& val)
{
    for (int i = 0; i < items_.size();)
    {
        if (GetItemsProperty(items_[i], property_name) == val)
            Remove(i);
        else
            ++i;
    }
}

void ObjectRefModel::RemoveAll(const QVariantMap& props)
{
    for (int i = 0; i < items_.size();)
    {
        if (HasPropertiesMatch(items_[i], props))
            Remove(i);
        else
            ++i;
    }
}

void ObjectRefModel::RemoveAllIf(const QByteArray& property_name, const QVariant& val, const std::function<bool(QObject*)>& func)
{
    if (!func)
        return;
    for (int i = 0; i < items_.size();)
    {
        if (GetItemsProperty(items_[i], property_name) == val && func(items_[i]))
            Remove(i);
        else
            ++i;
    }
}

void ObjectRefModel::RemoveAllIf(const QVariantMap& props, const std::function<bool(QObject*)>& func)
{
    if (!func)
        return;
    for (int i = 0; i < items_.size();)
    {
        if (HasPropertiesMatch(items_[i], props) && func(items_[i]))
            Remove(i);
        else
            ++i;
    }
}

void ObjectRefModel::Clear()
{
    if (items_.isEmpty())
        return;

    beginResetModel();
    // disconnecting old items
    UninstallAllItems();
    items_.clear();
    endResetModel();
}

void ObjectRefModel::SetItems(QVector<QObject*> items)
{
    if (!CanAddItems(items))
        return;

    beginResetModel();

    // disconnecting old items
    UninstallAllItems();
    std::swap(items_, items);
    // installing new items
    InstallItems(0, items_.size());

    endResetModel();
}

void ObjectRefModel::Move(int from, int to, int)
{
    if (from == to)
        return;

    moveRow(QModelIndex(), from, QModelIndex(), to);
}

void ObjectRefModel::ItemAboutToBeDeleted(QObject* item)
{
    Take(IndexOf(item));
}

QString ObjectRefModel::DebugString() const
{
    if (!rowCount() || !ItemMetaObject())
        return "Empty model";
    QString res;

    // TODO: use RoleInfo instead of QMetaProperty
    for (int i = 0; i < rowCount(); ++i)
    {
        res += QString("Item_%1 {\n").arg(i);
        auto item = items_[i];
        for (int j = 1; j < ItemMetaObject()->propertyCount(); ++j)
        {
            const auto property = ItemMetaObject()->property(j);
            if (property.isReadable())
            {
                res += QString("\t\"%1\" : %2\n").arg(property.name()).arg(item->property(property.name()).toString());
            }
        }
        res += "}\n\n";
    }
    return res;
}

int ObjectRefModel::rowCount(const QModelIndex&) const
{
    return items_.size();
}

// ObjectModel
ObjectModel::ObjectModel(QObject* parent /*= nullptr*/) : ObjectRefModel(parent)
{}

ObjectModel::ObjectModel(const QMetaObject* meta_object, QObject* parent /*= nullptr*/) : ObjectRefModel(meta_object, parent)
{}

ObjectModel::ObjectModel(const QMetaObject& meta_object, QObject* parent /*= nullptr*/) : ObjectRefModel(meta_object, parent)
{}

ObjectModel::~ObjectModel()
{
    UninstallAllItems();
}

bool ObjectModel::insertRows(int row, int count, const QModelIndex& parent /*= QModelIndex()*/)
{
    if (row < 0 || row > rowCount())
        return false;

    beginInsertRows(parent, row, row + count - 1);
    items_.insert(row, count, nullptr);
    InstallItems(row, count);
    endInsertRows();
    return true;
}

bool ObjectModel::CanAddItem(QObject*) const
{
    return true;
}

void ObjectModel::InstallItem(int row)
{
    auto& item = items_[row];
    if (!item)
    {
        if (!ItemMetaObject())
            throw std::logic_error("Models meta object is not set");
        item = CreateNewInstance(row);
        if (!item)
            throw std::logic_error("Can not create instance of object");
        if (item)
            items_[row] = item;
        else
            return;
    }
    ObjectRefModel::InstallItem(row);
    item->setParent(this);
}

void ObjectModel::UninstallItem(int row, bool taken)
{
    auto item = items_[row];
    if (!item)
        return;

    item->setParent(nullptr);
    ObjectRefModel::UninstallItem(row);
    if (!taken)
    {
        items_[row] = nullptr;
        delete item;
    }
}

QObject* ObjectModel::CreateNewInstance(int)
{
    return ItemMetaObject()->newInstance();
}

void ObjectModel::Resize(int size)
{
    if (size == items_.size())
        return;

    auto dif = qAbs(size - items_.size());

    if (size > items_.size())
        insertRows(items_.size(), dif);
    else
        removeRows(items_.size() - dif, dif);
}

// QQmlListProperty
QQmlListProperty<QObject> ObjectModel::GetItemsQmlListProperty()
{
    return QQmlListProperty<QObject>(this, this, &ObjectModel::ListAppend, &ObjectModel::ListCount, &ObjectModel::ListGet, &ObjectModel::ListClear);
}

void ObjectModel::ListAppend(QQmlListProperty<QObject>* list, QObject* val)
{
    reinterpret_cast<ObjectModel*>(list->data)->Append(val);
}

int ObjectModel::ListCount(QQmlListProperty<QObject>* list)
{
    return reinterpret_cast<ObjectModel*>(list->data)->rowCount();
}

QObject* ObjectModel::ListGet(QQmlListProperty<QObject>* list, int index)
{
    return reinterpret_cast<ObjectModel*>(list->data)->At(index);
}

void ObjectModel::ListClear(QQmlListProperty<QObject>* list)
{
    reinterpret_cast<ObjectModel*>(list->data)->Clear();
}
