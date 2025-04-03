#pragma once

#include "model_access.h"
#include "roles.h"

#include <QAbstractListModel>

namespace om
{
class ListModelValue
{
public:
    ListModelValue(QAbstractListModel* model, QModelIndex index, Id role_id);

    bool IsValid() const;

    QVariant Get() const;
    bool     Set(const QVariant& val);

    QModelIndex GetIndex() const;
    void        SetIndex(QModelIndex val);

    ListModelValue& operator++();
    ListModelValue& operator--();

private:
    QAbstractListModel* model_;
    QModelIndex         index_;
    Id                  role_id_;
};

class ListModelAccessValue
{
    ListModelAccessValue(ListModelAccess* model, int row, Id role_id);
    ListModelAccessValue(ListModelAccess* model, int row, const QByteArray& role_name);

    bool IsValid() const;

    QVariant Get() const;
    bool     Set(const QVariant& val);

    int  GetRow() const;
    void SetRow(int val);

    ListModelAccessValue& operator++();
    ListModelAccessValue& operator--();

private:
    ListModelAccess* model_;
    int              row_;
    Id               role_id_;
};
}  // namespace om