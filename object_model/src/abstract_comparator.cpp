#include "abstract_comparator.h"

using namespace om;

AbstractComparator::AbstractComparator(QObject* parent /*= nullptr*/) : AbstractFilterComparatorBase(parent)
{
    connect(this, &AbstractRoleComparator::rolesChanged, this, &AbstractRoleComparator::EmitComparatorChanged);
}

AbstractComparator::ComparisonResult AbstractComparator::CompareRows(const QModelIndex& source_left, const QModelIndex& source_right, const Role& role) const
{
    return enabled_ && roles_.contains(role.name) ? Compare(source_left, source_right, role) : ComparisonResult::UNKNOWN;
}

void AbstractComparator::EmitComparatorChanged()
{
    if (!enabled_)
        return;
    emit comparatorChanged();
}

AbstractRoleComparator::AbstractRoleComparator(QObject* parent /*= nullptr*/) : AbstractComparator(parent)
{}

void AbstractRoleComparator::SetRole(const QByteArray& val)
{
    SetRoles(val);
}

void AbstractRoleComparator::SetRoleNames(const QStringList& val)
{
    SetRoles(val);
}
