#pragma once

#include "abstract_filter_comparator_base.h"
#include <QModelIndex>
#include <QObject>

namespace om
{
class AbstractComparator : public AbstractFilterComparatorBase
{
    Q_OBJECT

public:
    AbstractComparator(QObject* parent = nullptr);
    virtual ~AbstractComparator() = default;

    ComparisonResult CompareRows(const QModelIndex& source_left, const QModelIndex& source_right, const Role& role) const;

signals:
    void comparatorChanged();

protected:
    virtual ComparisonResult Compare(const QModelIndex& source_left, const QModelIndex& source_right, const Role& role) const = 0;
    void                     EmitComparatorChanged();
};

class AbstractRoleComparator : public AbstractComparator
{
    Q_OBJECT
    Q_PROPERTY(QByteArray role READ GetRole WRITE SetRole NOTIFY rolesChanged)
    Q_PROPERTY(QStringList roles READ GetRoleNames WRITE SetRoleNames NOTIFY rolesChanged)
public:
    AbstractRoleComparator(QObject* parent = nullptr);

    void SetRole(const QByteArray& val);
    void SetRoleNames(const QStringList& val);
};
}  // namespace om
