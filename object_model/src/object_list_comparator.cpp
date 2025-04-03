#include "object_list_comparator.h"

#include "abstract_object_model.h"

using namespace om;

ObjectListComparator::ObjectListComparator(QObject* parent) : AbstractRoleComparator(parent)
{
    connect(this, &ObjectListComparator::valueModelRoleChanged, this, &ObjectListComparator::EmitComparatorChanged);
}

QByteArray ObjectListComparator::GetValueModelRole() const
{
    return value_model_role_;
}

void ObjectListComparator::SetValueModelRole(const QByteArray& role)
{
    if (role == value_model_role_)
        return;
    value_model_role_ = role;

    emit valueModelRoleChanged();
}

ObjectListComparator::ComparisonResult ObjectListComparator::Compare(const QModelIndex& source_left, const QModelIndex& source_right, const Role& role) const
{
    const auto lhs_data = source_left.data(role.id);
    const auto rhs_data = source_right.data(role.id);

    const auto* lhs_model = qvariant_cast<AbstractObjectModel*>(lhs_data);
    const auto* rhs_model = qvariant_cast<AbstractObjectModel*>(rhs_data);

    if (!lhs_model && !rhs_model)
        return ComparisonResult::UNKNOWN;
    if (!lhs_model)
        return ComparisonResult::GREATER;
    if (!rhs_model)
        return ComparisonResult::LESS;

    if (lhs_model->ItemClassName() != rhs_model->ItemClassName())
        return ComparisonResult::UNKNOWN;

    const auto lhs_size = lhs_model->rowCount();
    const auto rhs_size = rhs_model->rowCount();

    const auto role_id = lhs_model->roleIds().value(value_model_role_, -1);
    if (role_id < 0)
        return ComparisonResult::UNKNOWN;

    const auto size = qMin(lhs_size, rhs_size);
    for (int row = 0; row < size; ++row)
    {
        const auto lhs = lhs_model->data(lhs_model->index(row), role_id);
        const auto rhs = rhs_model->data(rhs_model->index(row), role_id);

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
