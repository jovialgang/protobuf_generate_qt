#pragma once

#include "abstract_filter.h"

namespace om
{
class ComparisonFilter : public AbstractRoleFilter
{
    Q_OBJECT
    Q_PROPERTY(QVariant value READ GetComparisonValue WRITE SetComparisonValue NOTIFY comparisonValueChanged)
    Q_PROPERTY(ComparisonOperator operator READ GetComparisonOperator WRITE SetComparisonOperator NOTIFY comparisonOperatorChanged)
public:
    enum class ComparisonOperator { EQUAL, INEQUAL, LESS, LESS_OR_EQUAL, GREATER, GREATER_OR_EQUAL };
    Q_ENUM(ComparisonOperator)

    ComparisonFilter(QObject* parent = nullptr);

    QVariant GetComparisonValue() const;
    void     SetComparisonValue(const QVariant& val);

    ComparisonOperator GetComparisonOperator() const;
    void               SetComparisonOperator(ComparisonOperator val);

signals:
    void comparisonValueChanged();
    void comparisonOperatorChanged();

protected:
    bool Accepts(const QModelIndex& index, const std::pair<QByteArray, int>& role) const override;

private:
    QVariant           comparison_value_;
    ComparisonOperator comparison_operation_ = ComparisonOperator::EQUAL;
};
}  // namespace om
