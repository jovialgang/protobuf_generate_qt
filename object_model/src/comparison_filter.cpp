#include "comparison_filter.h"

#include <stdexcept>

using namespace om;

ComparisonFilter::ComparisonFilter(QObject* parent) : AbstractRoleFilter(parent)
{}

QVariant ComparisonFilter::GetComparisonValue() const
{
    return comparison_value_;
}

void ComparisonFilter::SetComparisonValue(const QVariant& val)
{
    if (val == comparison_value_)
        return;
    comparison_value_ = val;
    emit comparisonValueChanged();
    EmitFilterChanged();
}

ComparisonFilter::ComparisonOperator ComparisonFilter::GetComparisonOperator() const
{
    return comparison_operation_;
}

void ComparisonFilter::SetComparisonOperator(ComparisonOperator val)
{
    if (val == comparison_operation_)
        return;
    comparison_operation_ = val;
    emit comparisonOperatorChanged();
    EmitFilterChanged();
}

bool ComparisonFilter::Accepts(const QModelIndex& index, const std::pair<QByteArray, int>& role) const
{
    if (!comparison_value_.isValid())
        return false;

    auto res = AbstractFilterComparatorBase::DefaultVariantCompare(index.data(role.second), comparison_value_);
    if (res == AbstractFilterComparatorBase::ComparisonResult::UNKNOWN)
        throw std::logic_error("Unknown result of comparison");

    switch (comparison_operation_)
    {
    case ComparisonOperator::EQUAL:
        return res == ComparisonResult::EQUAL;
    case ComparisonOperator::INEQUAL:
        return res != ComparisonResult::EQUAL;
    case ComparisonOperator::LESS:
        return res == ComparisonResult::LESS;
    case ComparisonOperator::LESS_OR_EQUAL:
        return res == ComparisonResult::LESS || res == ComparisonResult::EQUAL;
    case ComparisonOperator::GREATER:
        return res == ComparisonResult::GREATER;
    case ComparisonOperator::GREATER_OR_EQUAL:
        return res == ComparisonResult::GREATER || res == ComparisonResult::EQUAL;
    default:
        return false;
    }
}
