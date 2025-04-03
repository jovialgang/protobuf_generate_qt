#include "value_list_comparator.h"

#include "abstract_object_model.h"

#include <QAbstractListModel>
#include <QVariant>

using namespace om;

ValueListComparator::ValueListComparator(QObject* parent) : AbstractRoleComparator(parent)
{}

ValueListComparator::ComparisonResult ValueListComparator::Compare(const QModelIndex& source_left, const QModelIndex& source_right, const Role& role) const
{
    const auto lhs_data = source_left.data(role.id);
    const auto rhs_data = source_right.data(role.id);

    const auto* lhs_model = qvariant_cast<QAbstractListModel*>(lhs_data);
    const auto* rhs_model = qvariant_cast<QAbstractListModel*>(rhs_data);

    if (!lhs_model && !rhs_model)
        return ComparisonResult::UNKNOWN;
    if (!lhs_model)
        return ComparisonResult::GREATER;
    if (!rhs_model)
        return ComparisonResult::LESS;

    const auto lhs_size = lhs_model->rowCount();
    const auto rhs_size = rhs_model->rowCount();

    const auto size = qMin(lhs_size, rhs_size);
    for (int row = 0; row < size; ++row)
    {
        const auto lhs = lhs_model->data(lhs_model->index(row), Qt::UserRole);
        const auto rhs = rhs_model->data(rhs_model->index(row), Qt::UserRole);

        if (const auto result = AbstractComparator::DefaultVariantCompare(lhs, rhs); result != ComparisonResult::EQUAL)
            return result;
    }

    if (lhs_size < rhs_size)
        return ComparisonResult::LESS;
    else if (lhs_size > rhs_size)
        return ComparisonResult::GREATER;
    else
        return ComparisonResult::EQUAL;
}
