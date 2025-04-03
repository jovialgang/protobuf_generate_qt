#include "abstract_filter_comparator_base.h"

#include <QDateTime>
#include <QMetaType>
#include <QUrl>

using namespace om;

namespace
{
const om::Names kEmptyRoles = {};
}

AbstractFilterComparatorBase::AbstractFilterComparatorBase(QObject* parent /*= nullptr*/) : QObject(parent)
{
    connect(this, &AbstractFilterComparatorBase::enabledChanged, this, &AbstractFilterComparatorBase::rolesChanged);
}

const Names& AbstractFilterComparatorBase::GetRoles() const
{
    return enabled_ ? roles_ : kEmptyRoles;
}

void AbstractFilterComparatorBase::SetRoles(const QVariant& val)
{
    roles_ = RolesFromVariant(val);
    EmitRolesChanged();
}

bool AbstractFilterComparatorBase::HasRole(const QByteArray& role) const
{
    return roles_.contains(role);
}

bool AbstractFilterComparatorBase::IsEnabled() const
{
    return enabled_;
}

void AbstractFilterComparatorBase::SetEnabled(bool val)
{
    if (enabled_ == val)
        return;
    enabled_ = val;
    emit enabledChanged();
}

QByteArray AbstractFilterComparatorBase::GetRole() const
{
    return roles_.size() > 0 ? *roles_.begin() : QByteArray();
}

QStringList AbstractFilterComparatorBase::GetRoleNames() const
{
    return QStringList(roles_.begin(), roles_.end());
}

void AbstractFilterComparatorBase::EmitRolesChanged()
{
    if (!enabled_)
        return;
    UpdateRoles();
    emit rolesChanged();
}

// static
AbstractFilterComparatorBase::ComparisonResult AbstractFilterComparatorBase::DefaultVariantCompare(const QVariant& lhs, const QVariant& rhs)
{
    if (lhs == rhs)
        return ComparisonResult::EQUAL;

    if (lhs.type() != rhs.type())
        return lhs.type() < rhs.type() ? ComparisonResult::LESS : ComparisonResult::GREATER;

    switch (const auto type = lhs.userType())
    {
    case QMetaType::Bool:
        return lhs.toBool() < rhs.toBool() ? ComparisonResult::LESS : ComparisonResult::GREATER;
    case QMetaType::Int:
        return lhs.toInt() < rhs.toInt() ? ComparisonResult::LESS : ComparisonResult::GREATER;
    case QMetaType::UInt:
        return lhs.toUInt() < rhs.toUInt() ? ComparisonResult::LESS : ComparisonResult::GREATER;
    case QMetaType::LongLong:
        return lhs.toLongLong() < rhs.toLongLong() ? ComparisonResult::LESS : ComparisonResult::GREATER;
    case QMetaType::ULongLong:
        return lhs.toULongLong() < rhs.toULongLong() ? ComparisonResult::LESS : ComparisonResult::GREATER;
    case QMetaType::Float:
        return lhs.toFloat() < rhs.toFloat() ? ComparisonResult::LESS : ComparisonResult::GREATER;
    case QMetaType::Double:
        return lhs.toDouble() < rhs.toDouble() ? ComparisonResult::LESS : ComparisonResult::GREATER;
    case QMetaType::Char:
        return lhs.toChar() < rhs.toChar() ? ComparisonResult::LESS : ComparisonResult::GREATER;
    case QMetaType::QString:
        return lhs.toString() < rhs.toString() ? ComparisonResult::LESS : ComparisonResult::GREATER;
    case QMetaType::QByteArray:
        return lhs.toByteArray() < rhs.toByteArray() ? ComparisonResult::LESS : ComparisonResult::GREATER;
    case QMetaType::QDate:
        return lhs.toDate() < rhs.toDate() ? ComparisonResult::LESS : ComparisonResult::GREATER;
    case QMetaType::QTime:
        return lhs.toTime() < rhs.toTime() ? ComparisonResult::LESS : ComparisonResult::GREATER;
    case QMetaType::QDateTime:
        return lhs.toDateTime() < rhs.toDateTime() ? ComparisonResult::LESS : ComparisonResult::GREATER;
    case QMetaType::QUrl:
        return lhs.toUrl() < rhs.toUrl() ? ComparisonResult::LESS : ComparisonResult::GREATER;
    default:
        if (QMetaType::hasRegisteredComparators(type))
        {
            int        result{ 0 };
            const bool success = QMetaType::compare(lhs.data(), rhs.data(), type, &result);
            if (!success)
                return ComparisonResult::UNKNOWN;
            if (result < 0)
                return ComparisonResult::LESS;
            else if (result > 0)
                return ComparisonResult::GREATER;
            else
                return ComparisonResult::EQUAL;
        }
        return lhs.toString() < rhs.toString() ? ComparisonResult::LESS : ComparisonResult::GREATER;
    }
}

Names AbstractFilterComparatorBase::RolesFromVariant(const QVariant& val)
{
    switch (val.userType())
    {
    case QMetaType::QString:
    case QMetaType::QByteArray:
        return { val.toByteArray() };
    case QMetaType::QByteArrayList: {
        auto byte_array = val.value<QByteArrayList>();
        return Names(byte_array.begin(), byte_array.end());
    }
    case QMetaType::QStringList: {
        Names res;
        for (const auto& role : val.toStringList()) res.insert(role.toUtf8());
        return res;
    }
    }
    return {};
}
