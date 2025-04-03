#include "abstract_filter.h"

#include <stdexcept>

using namespace om;

AbstractFilter::AbstractFilter(QObject* parent /*= nullptr*/) : AbstractFilterComparatorBase(parent)
{}

bool AbstractFilter::AcceptsRow(const QModelIndex& index, const IdsMap& role_ids_map) const
{
    if (!enabled_)
        return true;

    if (roles_.empty())
    {
        return inverted_ ^ Accepts(index, {});
    }

    const auto f = [&](const QByteArray& role) {
        auto role_id = role_ids_map.find(role);
        if (role_id == role_ids_map.end())
            throw std::logic_error("Filter role is not set in model: " + role.toStdString());
        return Accepts(index, *role_id);
    };

    switch (logical_operator_)
    {
    case LogicalOperator::AND:
        return inverted_ ^ std::all_of(roles_.begin(), roles_.end(), f);
    case LogicalOperator::OR:
        return inverted_ ^ std::any_of(roles_.begin(), roles_.end(), f);
    default:
        throw std::runtime_error("Unsupported LogicOperator");
    }
}

AbstractFilter::LogicalOperator AbstractFilter::GetLogicalOperator() const
{
    return logical_operator_;
}

void AbstractFilter::SetLogicalOperator(LogicalOperator val)
{
    if (val == logical_operator_)
        return;
    logical_operator_ = val;
    emit logicalOperatorChanged();
    EmitFilterChanged();
}

bool AbstractFilter::IsInveted() const
{
    return inverted_;
}

void AbstractFilter::SetInverted(bool val)
{
    if (inverted_ == val)
        return;
    inverted_ = val;
    emit invertedChanged();
}

void AbstractFilter::EmitFilterChanged()
{
    if (!enabled_)
        return;
    emit filterChanged();
}

AbstractRoleFilter::AbstractRoleFilter(QObject* parent /*= nullptr*/) : AbstractFilter(parent)
{}

void AbstractRoleFilter::SetRole(const QByteArray& val)
{
    SetRoles(val);
}

void AbstractRoleFilter::SetRoleNames(const QStringList& val)
{
    SetRoles(val);
}
