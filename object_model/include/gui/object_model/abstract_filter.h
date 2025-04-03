#pragma once

#include "abstract_filter_comparator_base.h"
#include <QModelIndex>
#include <QObject>

namespace om
{
class AbstractFilter : public AbstractFilterComparatorBase
{
    Q_OBJECT
    Q_PROPERTY(LogicalOperator logicalOperator READ GetLogicalOperator WRITE SetLogicalOperator NOTIFY logicalOperatorChanged)
    Q_PROPERTY(bool inverted READ IsInveted WRITE SetInverted NOTIFY invertedChanged)
public:
    AbstractFilter(QObject* parent = nullptr);

    LogicalOperator GetLogicalOperator() const;
    void            SetLogicalOperator(LogicalOperator val);

    bool IsInveted() const;
    void SetInverted(bool val);

    virtual bool AcceptsRow(const QModelIndex& index, const IdsMap& role_ids_map) const;

signals:
    void filterChanged();
    void logicalOperatorChanged();
    void invertedChanged();

protected:
    virtual bool Accepts(const QModelIndex& index, const std::pair<QByteArray, int>& role) const = 0;
    void         EmitFilterChanged();

    LogicalOperator logical_operator_ = LogicalOperator::AND;
    bool            inverted_         = false;
};

class AbstractRoleFilter : public AbstractFilter
{
    Q_OBJECT
    Q_PROPERTY(QByteArray role READ GetRole WRITE SetRole NOTIFY rolesChanged)
    Q_PROPERTY(QStringList roles READ GetRoleNames WRITE SetRoleNames NOTIFY rolesChanged)
public:
    AbstractRoleFilter(QObject* parent = nullptr);

    void SetRole(const QByteArray& val);
    void SetRoleNames(const QStringList& val);
};
}  // namespace om
