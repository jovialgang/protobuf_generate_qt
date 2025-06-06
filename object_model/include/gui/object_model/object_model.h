#pragma once

#include "abstract_object_model.h"

#include <QQmlListProperty>

namespace om
{
// Class for basic QML/c++ model that that returns item class object pointer to QML, providing easy access to it, both from QML and c++.
class ObjectRefModel : public AbstractObjectModel
{
    Q_OBJECT
    Q_PROPERTY(QVector<QObject*> itemList READ GetItems WRITE SetItems)
    Q_PROPERTY(QStringList itemNames READ GetItemNames)

public:
    using const_iterator = QVector<QObject*>::const_iterator;

    ObjectRefModel(QObject* parent = nullptr);
    ObjectRefModel(const QMetaObject* meta_object, QObject* parent = nullptr);
    ObjectRefModel(const QMetaObject& meta_object, QObject* parent = nullptr);
    ~ObjectRefModel() override;

    // Overridden
    int rowCount(const QModelIndex& parent = QModelIndex()) const override;

    // Item storage
    QVector<QObject*> GetItems() const { return items_; }
    QObject*          operator[](unsigned int i) const { return items_[i]; }

    QStringList GetItemNames() const;

    const_iterator begin() const { return items_.begin(); }
    const_iterator end() const { return items_.end(); }

    const_iterator constBegin() const { return items_.constBegin(); }
    const_iterator constEnd() const { return items_.constEnd(); }

    QString DebugString() const;

public slots:
    QObject* At(int row) const;
    QObject* First() const;
    QObject* Last() const;

    // Search
    bool              Contains(QObject* item) const;
    bool              Contains(const QString& object_name) const;
    bool              Contains(const QByteArray& property_name, const QVariant& val) const;
    bool              Contains(const QVariantMap& props) const;
    int               IndexOf(QObject* item) const;
    int               IndexOf(const QString& object_name) const;
    int               IndexOf(const QByteArray& property_name, const QVariant& val) const;
    int               IndexOf(const QVariantMap& props) const;
    QVector<int>      IndexesOf(const QByteArray& property_name, const QVariant& val) const;
    QVector<int>      IndexesOf(const QVariantMap& props) const;
    QObject*          FindFirst(const QByteArray& property_name, const QVariant& val) const;
    QObject*          FindFirst(const QVariantMap& props) const;
    QObject*          Find(const QString& object_name) const;
    QVector<QObject*> FindObjects(const QString& object_name) const;
    QVector<QObject*> FindObjects(const QStringList& object_names) const;
    QVector<QObject*> Find(const QByteArray& property_name, const QVariant& val) const;
    QVector<QObject*> Find(const QVariantMap& props) const;

    QObject* Take(int row);
    QObject* TakeFirst();
    QObject* TakeLast();

    // After set, append or insert model won't become its parent.
    // If item or one of the items in container equals nullptr, functions will do nothing
    // If item at row already exists, deletes it.
    QObject* Append(QObject* item = nullptr);
    QObject* Append(const QVariantMap& props);
    void     AppendVector(const QVector<QObject*>& items);
    void     AppendModel(ObjectRefModel* items);
    QObject* Insert(int row, QObject* item = nullptr);
    QObject* Set(int row, QObject* item = nullptr);
    void     Move(int from, int to, int count = 1);
    void     Remove(int row, int count = 1);
    void     Remove(QObject* item);
    void     Remove(const QString& object_name);
    void     RemoveAll(const QByteArray& property_name, const QVariant& val);
    void     RemoveAll(const QVariantMap& props);
    void     RemoveAllIf(const QByteArray& property_name, const QVariant& val, const std::function<bool(QObject*)>& func);
    void     RemoveAllIf(const QVariantMap& props, const std::function<bool(QObject*)>& func);
    void     SetItems(QVector<QObject*> items);
    void     Clear();

protected:
//    COMPONENT_LOGGER("ui.orm");

    void ItemAboutToBeDeleted(QObject* item) override;

    QObject* GetItem(int row) const override;
    bool     SetItem(int row, QObject* val) override;

    bool removeRows(int row, int count, const QModelIndex& parent = QModelIndex());
    bool moveRows(const QModelIndex& sourceParent, int sourceRow, int count, const QModelIndex& destinationParent, int destinationChild);

    virtual bool CanAddItem(QObject* item) const;
    bool         CanAddItems(const QVector<QObject*>& items) const;

    virtual void InstallItem(int row) override;

    QVector<QObject*> items_;
};

// Class for basic QML/c++ model that that returns item class object pointer to QML, providing easy access to it, both from QML and c++.
// Item class of the model must be registered in QML and have Q_INVOKABLE default constructor if you want to create items with insertRows
// Model becomes item's parent after insertion
class ObjectModel : public ObjectRefModel
{
    Q_OBJECT
    Q_PROPERTY(QQmlListProperty<QObject> items READ GetItemsQmlListProperty)
    Q_CLASSINFO("DefaultProperty", "items")
public:
    ObjectModel(QObject* parent = nullptr);
    ObjectModel(const QMetaObject* meta_object, QObject* parent = nullptr);
    ObjectModel(const QMetaObject& meta_object, QObject* parent = nullptr);
    ~ObjectModel() override;

public slots:
    void Resize(int size);

protected:
    bool insertRows(int row, int count, const QModelIndex& parent = QModelIndex());

    virtual bool CanAddItem(QObject* item) const override;

    virtual void     InstallItem(int row) override;
    virtual void     UninstallItem(int row, bool taken = false) override;
    virtual QObject* CreateNewInstance(int row);

    QQmlListProperty<QObject> GetItemsQmlListProperty();
    static void               ListAppend(QQmlListProperty<QObject>* list, QObject* val);
    static int                ListCount(QQmlListProperty<QObject>* list);
    static QObject*           ListGet(QQmlListProperty<QObject>* list, int index);
    static void               ListClear(QQmlListProperty<QObject>* list);
};
}  // namespace om
