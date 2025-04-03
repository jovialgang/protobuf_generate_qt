#include "comparator.h"

using namespace om;

Comparator::Comparator(std::function<ComparisonResult(const QModelIndex&, const QModelIndex&, const Role&)> comparison_function, QObject* parent /*= nullptr*/)
    : AbstractRoleComparator(parent), comparison_function_(std::move(comparison_function))
{}

Comparator::ComparisonResult Comparator::Compare(const QModelIndex& source_left, const QModelIndex& source_right, const Role& role) const
{
    return comparison_function_ ? comparison_function_(source_left, source_right, role) : ComparisonResult::UNKNOWN;
}
