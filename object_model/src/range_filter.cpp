#include "range_filter.h"

#include <QVariant>

using namespace om;

RangeFilter::RangeFilter(QObject* parent /*= Q_NULLPTR*/) : AbstractRoleFilter(parent)
{}

double RangeFilter::GetFrom() const
{
    return from_;
}

void RangeFilter::SetFrom(double from)
{
    if (from > to_)
    {
        from_ = to_;
        to_   = from;
        emit toChanged();
    }
    else
        from_ = from;
    emit fromChanged();
    EmitFilterChanged();
}

double RangeFilter::GetTo() const
{
    return to_;
}

void RangeFilter::SetTo(double to)
{
    if (to < from_)
    {
        from_ = to;
        to_   = from_;
        emit fromChanged();
    }
    else
        to_ = to;
    emit toChanged();
    EmitFilterChanged();
}

RangeFilter::RangeCheckType RangeFilter::GetRangeCheckType() const
{
    return type_;
}

void RangeFilter::SetRangeCheckType(RangeCheckType val)
{
    if (type_ == val)
        return;
    type_ = val;
    emit rangeCheckTypeChanged();
    EmitFilterChanged();
}

bool RangeFilter::Accepts(const QModelIndex& index, const std::pair<QByteArray, int>& role) const
{
    bool   ok    = false;
    double value = index.data(role.second).toDouble(&ok);
    if (!ok)
        return false;

    switch (type_)
    {
    case AbstractFilterComparatorBase::RangeCheckType::INSIDE:
        return value > from_ && value < to_;
    case AbstractFilterComparatorBase::RangeCheckType::OUTSIDE:
        return value < from_ || value > to_;
    case AbstractFilterComparatorBase::RangeCheckType::INSIDE_OR_EQUAL:
        return value >= from_ && value <= to_;
    case AbstractFilterComparatorBase::RangeCheckType::OUTSIDE_OR_EQUAL:
        return value <= from_ || value >= to_;
    }
    return true;
}
