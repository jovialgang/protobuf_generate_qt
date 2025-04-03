#pragma once

#include "object_model_wrapper.h"

#include <stdexcept>

namespace om
{
// ObjectModel proxy with map interface
template <typename Key, typename T>
class ObjectModelMapWrapper : public ObjectModelWrapper<T>
{
public:
    // Creates proxy and new ObjectModel. Proxy controls life time of the ObjectModel
    ObjectModelMapWrapper();
    // Creates proxy for existing model. if key role is empty string, clears model.
    ObjectModelMapWrapper(ObjectModel *model, const char *key_property_name = "");

    virtual ~ObjectModelMapWrapper();

    bool Contains(const Key &key) const { return map_.contains(key); }

    int Count() const;

    T *Take(const Key &key);
    T *Get(const Key &key) const;
    T *operator[](const Key &key) const;

    void Insert(const Key &key, T *item);

    void Remove(const Key &key);
    void Remove(T *item);

    void Clear();

    const QHash<Key, T *> &GetMap() const;

protected:
    void ConnectItemAboutToBeRemoved();

    QHash<Key, T *> map_;

    QMetaObject::Connection item_about_to_be_removed_connection_;
};

// constructors
template <typename Key, typename T>
ObjectModelMapWrapper<Key, T>::ObjectModelMapWrapper() : ObjectModelWrapper<T>()
{
    ConnectItemAboutToBeRemoved();
}

template <typename Key, typename T>
ObjectModelMapWrapper<Key, T>::ObjectModelMapWrapper(ObjectModel *model, const char *key_property_name) : ObjectModelWrapper<T>(model)
{
    if (model->rowCount() > 0)
    {
        if (key_property_name == nullptr || key_property_name[0] == 0)
        {
            this->model_->Clear();
        }
        else
        {
            if (this->model_->ItemMetaObject()->indexOfProperty(key_property_name) < 0)
                throw std::invalid_argument("key_property_name is not a property of T");

            if (!this->model_->At(0)->property(key_property_name).template canConvert<Key>())
                throw std::invalid_argument("key property is not of Key type");

            for (auto *object : *this->model_)
            {
                map_[object->property(key_property_name).template value<Key>()] = qobject_cast<T *>(object);
            }
        }
    }
    ConnectItemAboutToBeRemoved();
}

// destructor
template <typename Key, typename T>
ObjectModelMapWrapper<Key, T>::~ObjectModelMapWrapper()
{
    QObject::disconnect(item_about_to_be_removed_connection_);
}

template <typename Key, typename T>
void ObjectModelMapWrapper<Key, T>::ConnectItemAboutToBeRemoved()
{
    item_about_to_be_removed_connection_ = QObject::connect(this->model_, &ObjectModel::rowsAboutToBeRemoved, [=](const QModelIndex &parent, int first, int last) {
        for (int i = first; i <= last; ++i)
        {
            map_.remove(map_.key(static_cast<T *>(this->model_->At(i))));
        }
    });
}

template <typename Key, typename T>
int ObjectModelMapWrapper<Key, T>::Count() const
{
    return this->model_->rowCount();
}

template <typename Key, typename T>
T *ObjectModelMapWrapper<Key, T>::Get(const Key &key) const
{
    return map_.value(key, nullptr);
}

template <typename Key, typename T>
T *ObjectModelMapWrapper<Key, T>::operator[](const Key &key) const
{
    auto res = map_.value(key, nullptr);
    if (!res)
        res = this->model_->ItemMetaObject().newInstance();
    Insert(key, res);
    return res;
}

template <typename Key, typename T>
T *ObjectModelMapWrapper<Key, T>::Take(const Key &key)
{
    auto res = map_.take(key);
    if (res)
        QMetaObject::invokeMethod(this->model_, "Take", Qt::AutoConnection, Q_ARG(int, this->model_->IndexOf(res)));
    return res;
}

template <typename Key, typename T>
void ObjectModelMapWrapper<Key, T>::Insert(const Key &key, T *item)
{
    if (!item)
        return;

    if (item->thread() != QGuiApplication::instance()->thread())
        item->moveToThread(QGuiApplication::instance()->thread());

    const auto prev_item = map_.value(key, nullptr);
    if (prev_item)
        QMetaObject::invokeMethod(this->model_, "Set", Qt::AutoConnection, Q_ARG(int, this->model_->IndexOf(prev_item)), Q_ARG(QObject *, item));
    else
        QMetaObject::invokeMethod(this->model_, "Append", Qt::AutoConnection, Q_ARG(QObject *, item));
    map_[key] = item;
}

template <typename Key, typename T>
void ObjectModelMapWrapper<Key, T>::Remove(const Key &key)
{
    if (map_.contains(key))
        QMetaObject::invokeMethod(this->model_, "Remove", Qt::AutoConnection, Q_ARG(QObject *, map_[key]));
}

template <typename Key, typename T>
void ObjectModelMapWrapper<Key, T>::Remove(T *item)
{
    QMetaObject::invokeMethod(this->model_, "Remove", Qt::AutoConnection, Q_ARG(QObject *, item));
}

template <typename Key, typename T>
void ObjectModelMapWrapper<Key, T>::Clear()
{
    map_.clear();
    QMetaObject::invokeMethod(this->model_, "Clear", Qt::AutoConnection);
}

template <typename Key, typename T>
const QHash<Key, T *> &ObjectModelMapWrapper<Key, T>::GetMap() const
{
    return map_;
}
}  // namespace om
