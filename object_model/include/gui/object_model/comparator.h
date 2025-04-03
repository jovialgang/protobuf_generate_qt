#pragma once

#include "abstract_comparator.h"

namespace om
{
class Comparator : public AbstractRoleComparator
{
public:
    Comparator(std::function<ComparisonResult(const QModelIndex&, const QModelIndex&, const Role&)> comparison_function, QObject* parent = nullptr);
    ~Comparator() = default;

protected:
    ComparisonResult Compare(const QModelIndex& source_left, const QModelIndex& source_right, const Role& role) const override;

    std::function<ComparisonResult(const QModelIndex&, const QModelIndex&, const Role&)> comparison_function_;
};
}  // namespace om
