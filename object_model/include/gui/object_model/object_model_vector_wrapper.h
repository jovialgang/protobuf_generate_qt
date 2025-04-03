#pragma once

#include "object_model_wrapper.h"

namespace om
{
// ObjectModel proxy with vector interface
template <typename T>
class ObjectModelVectorWrapper : public ObjectModelWrapper<T>
{
public:
    ObjectModelVectorWrapper() : ObjectModelWrapper<T>() {}
    ObjectModelVectorWrapper(ObjectModel *model) : ObjectModelWrapper<T>(model) {}
    virtual ~ObjectModelVectorWrapper() {}

    int Count() const;

    int          IndexOf(T *item);
    int          IndexOf(char *property_name, const QVariant &val);
    QVector<int> IndexesOf(char *property_name, const QVariant &val);

    T *operator[](int row) const;
    T *First() const;
    T *Last() const;
    T *Take(int row);
    T *TakeFirst();
    T *TakeLast();

    void Set(int row, T *item = nullptr);

    void Append(T *item = nullptr);
    void Append(QVector<T *> items);
    void Insert(int row, T *item = nullptr);

    void Move(int from, int to);
    void Remove(int row);
    void Remove(T *item);
    void Resize(int size);
    void Reset(QVector<T *> items);
    void Clear();
};

template <typename T>
int ObjectModelVectorWrapper<T>::Count() const
{
    return this->model_->rowCount();
}

template <typename T>
int ObjectModelVectorWrapper<T>::IndexOf(T *item)
{
    return this->model_->IndexOf(item);
}

template <typename T>
int ObjectModelVectorWrapper<T>::IndexOf(char *property_name, const QVariant &val)
{
    return this->model_->IndexOf(property_name, val);
}

template <typename T>
QVector<int> ObjectModelVectorWrapper<T>::IndexesOf(char *property_name, const QVariant &val)
{
    return this->model_->IndexesOf(property_name, val);
}

template <typename T>
T *ObjectModelVectorWrapper<T>::operator[](int row) const
{
    return static_cast<T *>(this->model_[row]);
}

template <typename T>
T *ObjectModelVectorWrapper<T>::First() const
{
    return static_cast<T *>(this->model_->First());
}

template <typename T>
T *ObjectModelVectorWrapper<T>::Last() const
{
    return static_cast<T *>(this->model_->Last());
}

template <typename T>
T *ObjectModelVectorWrapper<T>::Take(int row)
{
    auto res = this->At(row);
    if (res)
        QMetaObject::invokeMethod(this->model_, "Take", Qt::AutoConnection, Q_ARG(int, row));
    return res;
}

template <typename T>
T *ObjectModelVectorWrapper<T>::TakeFirst()
{
    auto res = this->First();
    if (res)
        QMetaObject::invokeMethod(this->model_, "TakeFirst", Qt::AutoConnection);
    return res;
}

template <typename T>
T *ObjectModelVectorWrapper<T>::TakeLast()
{
    auto res = this->Last();
    if (res)
        QMetaObject::invokeMethod(this->model_, "TakeLast", Qt::AutoConnection);
    return res;
}

template <typename T>
void ObjectModelVectorWrapper<T>::Set(int row, T *item /*= nullptr*/)
{
    if (item && item->thread() != QGuiApplication::instance()->thread())
        item->moveToThread(QGuiApplication::instance()->thread());
    QMetaObject::invokeMethod(this->model_, "Set", Qt::AutoConnection, Q_ARG(int, row), Q_ARG(QObject *, item));
}

template <typename T>
void ObjectModelVectorWrapper<T>::Append(T *item /*= nullptr*/)
{
    if (item && item->thread() != QGuiApplication::instance()->thread())
        item->moveToThread(QGuiApplication::instance()->thread());
    QMetaObject::invokeMethod(this->model_, "Append", Qt::AutoConnection, Q_ARG(QObject *, item));
}

template <typename T>
void ObjectModelVectorWrapper<T>::Append(QVector<T *> items)
{
    QVector<QObject *> temp;
    temp.reserve(items.size());
    for (auto item : items)
    {
        if (item && item->thread() != QGuiApplication::instance()->thread())
            item->moveToThread(QGuiApplication::instance()->thread());
        temp.push_back(item);
    }
    QMetaObject::invokeMethod(this->model_, "Append", Qt::AutoConnection, Q_ARG(QVector<QObject *>, temp));
}

template <typename T>
void ObjectModelVectorWrapper<T>::Insert(int row, T *item /*= nullptr*/)
{
    if (item->thread() != QGuiApplication::instance()->thread())
        item->moveToThread(QGuiApplication::instance()->thread());
    QMetaObject::invokeMethod(this->model_, "Insert", Qt::AutoConnection, Q_ARG(int, row), Q_ARG(QObject *, item));
}

template <typename T>
void ObjectModelVectorWrapper<T>::Move(int from, int to)
{
    QMetaObject::invokeMethod(this->model_, "Move", Qt::AutoConnection, Q_ARG(int, from), Q_ARG(int, to));
}

template <typename T>
void ObjectModelVectorWrapper<T>::Remove(int row)
{
    QMetaObject::invokeMethod(this->model_, "Remove", Qt::AutoConnection, Q_ARG(int, row));
}

template <typename T>
void ObjectModelVectorWrapper<T>::Remove(T *item)
{
    QMetaObject::invokeMethod(this->model_, "Remove", Qt::AutoConnection, Q_ARG(QObject *, item));
}

template <typename T>
void ObjectModelVectorWrapper<T>::Clear()
{
    QMetaObject::invokeMethod(this->model_, "Clear", Qt::AutoConnection);
}

template <typename T>
void ObjectModelVectorWrapper<T>::Reset(QVector<T *> items)
{
    QVector<QObject *> temp;
    temp.reserve(items.size());
    for (auto item : items)
    {
        if (item && item->thread() != QGuiApplication::instance()->thread())
            item->moveToThread(QGuiApplication::instance()->thread());
        temp.push_back(item);
    }
    QMetaObject::invokeMethod(this->model_, "Reset", Qt::AutoConnection, Q_ARG(QVector<QObject *>, temp));
}

template <typename T>
void ObjectModelVectorWrapper<T>::Resize(int size)
{
    QMetaObject::invokeMethod(this->model_, "Resize", Qt::AutoConnection, Q_ARG(int, size));
}
}  // namespace om
