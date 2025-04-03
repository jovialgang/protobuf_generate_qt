#include "enumeration_filter.h"

#include <stdexcept>

using namespace om;

// EnumerationFilter
EnumerationFilter::EnumerationFilter(QObject* parent) : AbstractRoleFilter(parent)
{}

QVariantList EnumerationFilter::GetValues() const
{
    return QVariantList(values_.begin(), values_.end());
}

void EnumerationFilter::SetValues(const QVariantList& values)
{
    values_.clear();
    for (const auto& val : values)
    {
        bool ok      = true;
        auto int_val = val.toInt(&ok);
        if (ok)
            values_.insert(int_val);
        else
            throw std::logic_error("Can not convert value to int");
    }
    emit valuesChanged();
    EmitFilterChanged();
}

bool EnumerationFilter::HasValue(int val)
{
    return values_.contains(val);
}

void EnumerationFilter::SetValue(int val, bool enabled)
{
    if (enabled)
        AddValue(val);
    else
        RemoveValue(val);
}

void EnumerationFilter::AddValue(int val)
{
    if (values_.contains(val))
        return;
    values_.insert(val);
    emit valuesChanged();
    EmitFilterChanged();
}

void EnumerationFilter::RemoveValue(int val)
{
    if (!values_.contains(val))
        return;
    values_.erase(val);
    emit valuesChanged();
    EmitFilterChanged();
}

void EnumerationFilter::ClearValues()
{
    values_.clear();
    emit valuesChanged();
    EmitFilterChanged();
}

bool EnumerationFilter::Accepts(const QModelIndex& index, const std::pair<QByteArray, int>& role) const
{
    bool ok  = true;
    auto val = index.data(role.second).toInt(&ok);
    return ok ? values_.contains(val) : false;
}

// StringEnumerationFilter
StringEnumerationFilter::StringEnumerationFilter(QObject* parent /*= nullptr*/) : AbstractRoleFilter(parent)
{}

QStringList StringEnumerationFilter::GetValues() const
{
    return QStringList(values_.begin(), values_.end());
}

void StringEnumerationFilter::SetValues(const QStringList& values)
{
    values_ = boost::container::flat_set<QString>(values.begin(), values.end());
    emit valuesChanged();
    EmitFilterChanged();
}

bool StringEnumerationFilter::HasValue(const QString& val)
{
    return values_.contains(val);
}

void StringEnumerationFilter::SetValue(const QString& val, bool enabled)
{
    if (enabled)
        AddValue(val);
    else
        RemoveValue(val);
}

void StringEnumerationFilter::AddValue(const QString& val)
{
    if (values_.contains(val))
        return;
    values_.insert(val);
    emit valuesChanged();
    EmitFilterChanged();
}

void StringEnumerationFilter::RemoveValue(const QString& val)
{
    if (!values_.contains(val))
        return;
    values_.erase(val);
    emit valuesChanged();
    EmitFilterChanged();
}

void StringEnumerationFilter::ClearValues()
{
    values_.clear();
    emit valuesChanged();
    EmitFilterChanged();
}

bool StringEnumerationFilter::Accepts(const QModelIndex& index, const std::pair<QByteArray, int>& role) const
{
    auto val = index.data(role.second).toString();
    return !val.isEmpty() ? values_.contains(val) : false;
}
