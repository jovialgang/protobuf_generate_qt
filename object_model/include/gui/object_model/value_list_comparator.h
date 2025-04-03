#pragma once

#include "abstract_comparator.h"

namespace om
{
// Компаратор для списков "простых" значений - в индексах ожидаются наследники QAbstractListModel
class ValueListComparator : public AbstractRoleComparator
{
    Q_OBJECT
    // роль для сравнения у моделей единственная - "item" - Qt::UserRole
public:
    ValueListComparator(QObject* parent = Q_NULLPTR);

    ComparisonResult Compare(const QModelIndex& source_left, const QModelIndex& source_right, const Role& role) const override;
};
}  // namespace om
