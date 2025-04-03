#include "comparator_group.h"

#include <stdexcept>

using namespace om;

ComparatorGroup::ComparatorGroup(QObject* parent /*= nullptr*/) : AbstractComparator(parent)
{
    connect(this, &ComparatorGroup::comparatorsChanged, this, &ComparatorGroup::EmitRolesChanged);
}

AbstractComparator::ComparisonResult ComparatorGroup::Compare(const QModelIndex& source_left, const QModelIndex& source_right, const Role& role) const
{
    auto* comparator = GetComparator(role.name);
    return comparator ? comparator->CompareRows(source_left, source_right, role) : ComparisonResult::UNKNOWN;
}

void ComparatorGroup::UpdateRoles()
{
    roles_.clear();
    for (auto comparator : comparators_)
    {
        if (!comparator->IsEnabled())
            continue;
        const auto& roles = comparator->GetRoles();
        // TODO: show warning if comparators have same roles
        roles_.insert(roles.begin(), roles.end());
    }
}

QVector<AbstractComparator*> ComparatorGroup::GetComparators() const
{
    return comparators_;
}

AbstractComparator* ComparatorGroup::GetComparator(const QByteArray& role) const
{
    auto cmp_it = std::find_if(comparators_.cbegin(), comparators_.cend(), [=](auto* cmp) { return cmp->HasRole(role); });
    return cmp_it == comparators_.cend() ? nullptr : *cmp_it;
}

void ComparatorGroup::SetComparators(const QVector<AbstractComparator*>& val)
{
    comparators_ = val;
    for (auto* c : comparators_) ConnectComparator(c);
    emit comparatorsChanged();
}

AbstractComparator* ComparatorGroup::At(int index) const
{
    if (index < 0 || index >= comparators_.size())
        return nullptr;
    return comparators_[index];
}

void ComparatorGroup::Append(AbstractComparator* val)
{
    if (!val)
        throw std::invalid_argument("Comparator can't be null");

    comparators_.append(val);
    ConnectComparator(val);
    emit comparatorsChanged();
}

void ComparatorGroup::Clear()
{
    for (auto* c : comparators_) DisconnectComparator(c);
    comparators_.clear();
    emit comparatorsChanged();
}

void ComparatorGroup::Remove(int index)
{
    if (index < 0 || index >= comparators_.size())
        return;
    DisconnectComparator(comparators_[index]);
    comparators_.remove(index);
    emit comparatorsChanged();
}

void ComparatorGroup::Remove(AbstractComparator* val)
{
    Remove(comparators_.indexOf(val));
}

int ComparatorGroup::GetCount() const
{
    return comparators_.size();
}

void ComparatorGroup::ConnectComparator(AbstractComparator* comparator)
{
    connect(comparator, &AbstractComparator::rolesChanged, this, &ComparatorGroup::EmitRolesChanged);
    connect(comparator, &AbstractComparator::comparatorChanged, this, &ComparatorGroup::EmitComparatorChanged);
    connect(comparator, &AbstractComparator::destroyed, this, [=] { Remove(comparator); });
}

void ComparatorGroup::DisconnectComparator(AbstractComparator* comparator)
{
    disconnect(comparator, 0, this, 0);
}

QQmlListProperty<AbstractComparator> ComparatorGroup::GetComparatorsQmlListProperty()
{
    return QQmlListProperty<AbstractComparator>(this, this, &ComparatorGroup::Append, &ComparatorGroup::GetCount, &ComparatorGroup::At, &ComparatorGroup::Clear);
}

void ComparatorGroup::Append(QQmlListProperty<AbstractComparator>* list, AbstractComparator* val)
{
    reinterpret_cast<ComparatorGroup*>(list->data)->Append(val);
}

void ComparatorGroup::Clear(QQmlListProperty<AbstractComparator>* list)
{
    reinterpret_cast<ComparatorGroup*>(list->data)->Clear();
}

int ComparatorGroup::GetCount(QQmlListProperty<AbstractComparator>* list)
{
    return reinterpret_cast<ComparatorGroup*>(list->data)->GetCount();
}

AbstractComparator* ComparatorGroup::At(QQmlListProperty<AbstractComparator>* list, int index)
{
    return reinterpret_cast<ComparatorGroup*>(list->data)->At(index);
}
