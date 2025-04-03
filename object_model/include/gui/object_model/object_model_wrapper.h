#pragma once

#include "object_model.h"

#include <QGuiApplication>
#include <QPointer>

#include <stdexcept>
#include <type_traits>

namespace om
{
// Base model proxy class
template <typename T>
class ObjectModelWrapper
{
    static_assert(std::is_base_of<QObject, T>::value, "T must be derived from QObject");

public:
    using const_iterator = T *const *;

    virtual ~ObjectModelWrapper()
    {
        if (delete_model_)
            delete this->model_;
    }

    ObjectModel *GetModel() const { return this->model_; }

    T *          At(int index) const { return static_cast<T *>(this->model_->At(index)); }
    T *          FindFirst(char *property_name, const QVariant &val) const { return static_cast<T *>(this->model_->FindFirst(property_name, val)); }
    QVector<T *> Find(char *property_name, const QVariant &val) const
    {
        auto temp = this->model_->Find(property_name, val);
        return QVector<T *>(static_cast<T **>(temp.begin()), static_cast<T **>(temp.end()));
    }

    void RemoveAll(char *property_name, const QVariant &val) { this->model_->RemoveAll(property_name, val); }
    void RemoveAllIf(char *property_name, const QVariant &val, const std::function<bool(T *item)> &func);

    const_iterator begin() const { return reinterpret_cast<const_iterator>(model_->constBegin()); }
    const_iterator end() const { return reinterpret_cast<const_iterator>(model_->constEnd()); }

protected:
    // constructor that doesn't control models life time
    ObjectModelWrapper(ObjectModel *model)
    {
        if (model->ItemMetaObject() != &T::staticMetaObject)
            throw std::invalid_argument("Proxy type T not equal models item type");
        this->model_ = model;
        if (this->model_->thread() != QGuiApplication::instance()->thread())
            this->model_->moveToThread(QGuiApplication::instance()->thread());
        delete_model_ = false;
    }

    // constructor that controls model life time
    ObjectModelWrapper()
    {
        this->model_ = new ObjectModel(T::staticMetaObject);
        if (this->model_->thread() != QGuiApplication::instance()->thread())
            this->model_->moveToThread(QGuiApplication::instance()->thread());
        delete_model_ = true;
    }

    bool                  delete_model_;
    QPointer<ObjectModel> model_;
};

template <typename T>
void ObjectModelWrapper<T>::RemoveAllIf(char *property_name, const QVariant &val, const std::function<bool(T *item)> &func)
{
    if (!func)
        return;
    this->model_->RemoveAllIf(property_name, val, [func](QObject *item) { return func(static_cast<T *>(item)); });
}
}  // namespace om
