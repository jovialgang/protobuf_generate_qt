#include "filter_group.h"

#include <algorithm>
#include <stdexcept>

using namespace om;

FilterGroup::FilterGroup(QObject* parent /*= nullptr*/) : AbstractFilter(parent)
{
    connect(this, &FilterGroup::filtersChanged, this, &FilterGroup::EmitRolesChanged);
}

void FilterGroup::UpdateRoles()
{
    roles_.clear();
    for (auto filter : filters_)
    {
        if (!filter->IsEnabled())
            continue;
        const auto& roles = filter->GetRoles();
        roles_.insert(roles.begin(), roles.end());
    }
}

void FilterGroup::ConnectFilter(AbstractFilter* filter)
{
    connect(filter, &AbstractFilter::filterChanged, this, &FilterGroup::filterChanged);
    connect(filter, &AbstractFilter::rolesChanged, this, &FilterGroup::rolesChanged);
    connect(filter, &AbstractFilter::destroyed, this, [=] { Remove(filter); });
}

void FilterGroup::DisconnectFilter(AbstractFilter* filter)
{
    disconnect(filter, 0, this, 0);
}

bool FilterGroup::AcceptsRow(const QModelIndex& index, const IdsMap& role_ids_map) const
{
    if (!enabled_)
        return true;
    switch (logical_operator_)
    {
    case LogicalOperator::AND:
        return std::all_of(filters_.begin(), filters_.end(), [&](AbstractFilter* f) { return f->AcceptsRow(index, role_ids_map); });
    case LogicalOperator::OR:
        // In order to not change final result disabled filters must return false
        return std::any_of(filters_.begin(), filters_.end(), [&](AbstractFilter* f) { return f->IsEnabled() ? f->AcceptsRow(index, role_ids_map) : false; });
    default:
        throw std::runtime_error("Unsupported LogicOperator");
    }
}

void FilterGroup::Append(AbstractFilter* val)
{
    if (!val)
        throw std::invalid_argument("Filter can't be null");

    filters_.append(val);
    ConnectFilter(val);
    emit filtersChanged();
}

void FilterGroup::Clear()
{
    for (auto filter : filters_) DisconnectFilter(filter);
    filters_.clear();
    emit filtersChanged();
}

int FilterGroup::GetCount() const
{
    return filters_.size();
}

AbstractFilter* FilterGroup::At(int index) const
{
    if (index < 0 || index >= filters_.size())
        return nullptr;
    return filters_[index];
}

QVector<AbstractFilter*> FilterGroup::GetFilters() const
{
    return filters_;
}

void FilterGroup::SetFilters(const QVector<AbstractFilter*>& val)
{
    filters_ = val;
    for (auto filter : filters_) ConnectFilter(filter);
    emit filtersChanged();
}

void FilterGroup::Remove(int index)
{
    if (index < 0 || index >= filters_.size())
        return;
    DisconnectFilter(filters_[index]);
    filters_.remove(index);
    emit filtersChanged();
}

void FilterGroup::Remove(AbstractFilter* val)
{
    Remove(filters_.indexOf(val));
}

QQmlListProperty<AbstractFilter> FilterGroup::GetFiltersQmlListProperty()
{
    return QQmlListProperty<AbstractFilter>(this, this, &FilterGroup::Append, &FilterGroup::GetCount, &FilterGroup::At, &FilterGroup::Clear);
}

void FilterGroup::Append(QQmlListProperty<AbstractFilter>* list, AbstractFilter* val)
{
    reinterpret_cast<FilterGroup*>(list->data)->Append(val);
}

void FilterGroup::Clear(QQmlListProperty<AbstractFilter>* list)
{
    reinterpret_cast<FilterGroup*>(list->data)->Clear();
}

int FilterGroup::GetCount(QQmlListProperty<AbstractFilter>* list)
{
    return reinterpret_cast<FilterGroup*>(list->data)->GetCount();
}

AbstractFilter* FilterGroup::At(QQmlListProperty<AbstractFilter>* list, int index)
{
    return reinterpret_cast<FilterGroup*>(list->data)->At(index);
}
