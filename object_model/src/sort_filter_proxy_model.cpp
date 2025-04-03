#include "sort_filter_proxy_model.h"

//#include <log_qt.h>

#include <QHash>
#include <QtQml>

using namespace om;

SortFilterProxyModel::SortFilterProxyModel(QObject* parent /*= nullptr*/) : QSortFilterProxyModel(parent), ListModelAccess(), AbstractDynamicRolesProvider()
{
    connect(this, &QAbstractItemModel::rowsInserted, this, &SortFilterProxyModel::rowCountChanged);
    connect(this, &QAbstractItemModel::rowsRemoved, this, &SortFilterProxyModel::rowCountChanged);
    connect(this, &QAbstractItemModel::modelReset, this, &SortFilterProxyModel::rowCountChanged);
    setDynamicSortFilter(false);
}

bool SortFilterProxyModel::IsEnabled() const
{
    return enabled_;
}

void SortFilterProxyModel::SetEnabled(bool val)
{
    if (enabled_ == val)
        return;
    enabled_ = val;
    emit enabledChanged();
    if (sorting_required_)
        PrivateSort();
    if (filtering_required_)
        PrivateFilter();
}

void SortFilterProxyModel::setSourceModel(QAbstractItemModel* val)
{
    if (val == sourceModel())
        return;

    if (sourceModel())
    {
        role_ids_.clear();
        disconnect(sourceModel(), 0, this, 0);
    }

    source_object_model_ = qobject_cast<AbstractObjectModel*>(val);

    if (val)
    {
        if (source_object_model_)
        {
            connect(source_object_model_, &AbstractObjectModel::itemDataChanged, this, &SortFilterProxyModel::OnItemDataChanged);
            if (source_object_model_->IsInitialized())
                role_ids_ = source_object_model_->roleIds();
            else
                connect(source_object_model_, &AbstractObjectModel::initialized, this, [=] {
                    role_ids_ = source_object_model_->roleIds();
                    UpdateRoleIds();
                });
        }
        else
        {
            auto role_names = val->roleNames();
            for (auto role = role_names.begin(); role != role_names.end(); ++role)
            {
                role_ids_[role.value()] = role.key();
            }
        }

        connect(val, &QAbstractItemModel::rowsInserted, this, &SortFilterProxyModel::Invalidate);
        connect(val, &QAbstractItemModel::rowsRemoved, this, &SortFilterProxyModel::Invalidate);
        connect(val, &QAbstractItemModel::modelReset, this, &SortFilterProxyModel::Invalidate);
        connect(val, &QAbstractItemModel::dataChanged, this, &SortFilterProxyModel::OnDataChanged);
    }

    UpdateFilterRoleIds();
    UpdateSortRoleIds();
    SetDynamicRolesReceiver(source_object_model_);
    QSortFilterProxyModel::setSourceModel(val);

    if (val)
        Invalidate();

    emit modelChanged();
}

// Filter
AbstractFilter* SortFilterProxyModel::GetFilter() const
{
    return filter_;
}

void SortFilterProxyModel::SetFilter(AbstractFilter* val)
{
    if (val == filter_)
        return;

    if (filter_)
        disconnect(filter_, 0, this, 0);

    filter_ = val;
    connect(filter_, &AbstractFilter::filterChanged, this, &SortFilterProxyModel::Invalidate);
    connect(filter_, &AbstractFilter::rolesChanged, this, &SortFilterProxyModel::OnFilterRolesChanged);
    OnFilterRolesChanged();
    emit filterChanged();
}

// Comparator
AbstractComparator* SortFilterProxyModel::GetComparator() const
{
    return comparator_;
}

void SortFilterProxyModel::SetComparator(AbstractComparator* val)
{
    if (val == comparator_)
        return;

    if (comparator_)
        disconnect(comparator_, 0, this, 0);

    comparator_ = val;
    connect(comparator_, &AbstractComparator::comparatorChanged, this, &SortFilterProxyModel::Sort);
    // comparator doesn't provide dynamic roles itself, so we don't need to connect to its rolesChanged signal. dynamic roles for sort is sort_roles_
    Sort();
    emit comparatorChanged();
}

// Dynamic roles
void SortFilterProxyModel::OnFilterRolesChanged()
{
    filter_roles_.clear();
    if (filter_)
        for (const auto& role : filter_->GetRoles()) filter_roles_[role] = 0;
    UpdateFilterRoleIds();
    Filter();
    OnDynamicRolesChanged();
}

QByteArray SortFilterProxyModel::GetSortRole() const
{
    return sort_roles_.size() > 0 ? sort_roles_.front().name : QByteArray();
}

void SortFilterProxyModel::SetSortRole(const QByteArray& val)
{
    if (val.isEmpty())
        SetSortRoles({});
    else
        SetSortRoles({ val });
}

QStringList SortFilterProxyModel::GetSortRoles() const
{
    QStringList res;
    for (auto role : sort_roles_) res.push_back(role.name);
    return res;
}

void SortFilterProxyModel::SetSortRoles(const QStringList& val)
{
    sort_roles_.clear();
    for (const auto& role : val)
    {
        sort_roles_.push_back({ 0, role.toUtf8() });
    }
    UpdateSortRoleIds();
    Sort();
    OnDynamicRolesChanged();
    emit sortRolesChanged();
}

void SortFilterProxyModel::UpdateRoleIds()
{
    UpdateSortRoleIds();
    UpdateFilterRoleIds();
    Invalidate();
}

void SortFilterProxyModel::UpdateSortRoleIds()
{
    sort_role_ids_.clear();
    if (role_ids_.empty())
        return;
    for (auto& role : sort_roles_)
    {
        auto role_id = role_ids_.find(role.name);
        if (role_id == role_ids_.end())
            qDebug() << (std::logic_error("Source model doesn't have sort role "), QString("Source model doesn't have sort role %1").arg(QString::fromUtf8(role.name)));

        role.id = role_id.value();
        sort_role_ids_.insert(role.id);
    }
}

void SortFilterProxyModel::UpdateFilterRoleIds()
{
    filter_role_ids_.clear();
    if (role_ids_.empty())
        return;
    for (auto& role : filter_roles_)
    {
        auto role_id = role_ids_.find(role.first);
        if (role_id == role_ids_.end())
            qDebug() << (std::logic_error("Source model doesn't have filter role "), QString("Source model doesn't have sort role %1").arg(QString::fromUtf8(role.first)));

        role.second = role_id.value();
        filter_role_ids_.insert(role.second);
    }
}

Ids SortFilterProxyModel::GetDynamicRoles() const
{
    Ids res(sort_role_ids_.begin(), sort_role_ids_.end());
    res.insert(filter_role_ids_.begin(), filter_role_ids_.end());
    return res;
}

void SortFilterProxyModel::OnDynamicRolesChanged()
{
    AbstractDynamicRolesProvider::OnDynamicRolesChanged();
    emit dynamicRolesChanged();
}

// Sort order
Qt::SortOrder SortFilterProxyModel::GetSortOrder() const
{
    return sort_order_;
}

void SortFilterProxyModel::SetSortOrder(Qt::SortOrder val)
{
    if (val == sort_order_)
        return;
    sort_order_ = val;
    Sort();
    emit sortOrderChanged();
}

void SortFilterProxyModel::ChangeSortOrder()
{
    SetSortOrder(sort_order_ == Qt::AscendingOrder ? Qt::DescendingOrder : Qt::AscendingOrder);
}

// Data
int SortFilterProxyModel::GetCount() const
{
    return rowCount();
}

QVariant SortFilterProxyModel::sourceData(const QModelIndex& index, int role /*= Qt::DisplayRole*/) const
{
    return sourceModel()->data(index, role);
}

QVariant SortFilterProxyModel::GetData(int row, int role /*= AbstractObjectModel::kItemRole*/) const
{
    return data(index(row, 0), role);
}

QVariant SortFilterProxyModel::GetData(int row, const QByteArray& role_name) const
{
    return ListModelAccess::GetData(row, role_name);
}

bool SortFilterProxyModel::SetData(int row, const QVariant& value, int role)
{
    return setData(index(row, 0), value, role);
}

bool SortFilterProxyModel::SetData(int row, const QVariant& val, const QByteArray& role_name /*= AbstractObjectModel::kItemRoleName*/)
{
    return ListModelAccess::SetData(row, val, role_name);
}

int SortFilterProxyModel::IndexOf(const QByteArray& property_name, const QVariant& val) const
{
    // TODO: передлать через IndexOf обычной модели
    for (int i = 0; i < GetCount(); ++i)
        if (GetData(i, property_name) == val)
            return i;
    return -1;
}

int SortFilterProxyModel::MapFromSource(int source_row)
{
    if (!sourceModel())
        return -1;
    return mapFromSource(sourceModel()->index(source_row, 0)).row();
}

int SortFilterProxyModel::MapToSource(int row)
{
    if (!sourceModel())
        return -1;
    return mapToSource(index(row, 0)).row();
}

QHash<QByteArray, int> SortFilterProxyModel::roleIds() const
{
    return role_ids_;
}

// Sort/filter
void SortFilterProxyModel::Invalidate()
{
    Filter();
    Sort();
}

void SortFilterProxyModel::Sort()
{
    if (sorting_required_ || !sourceModel() || sort_roles_.empty())
        return;
    sorting_required_ = true;
    QMetaObject::invokeMethod(this, &SortFilterProxyModel::PrivateSort, Qt::QueuedConnection);
}

void SortFilterProxyModel::PrivateSort()
{
    if (!enabled_)
        return;
    sort(0, sort_order_);
    sorting_required_ = false;
}

void SortFilterProxyModel::Filter()
{
    if (filtering_required_ || !sourceModel())
        return;
    filtering_required_ = true;
    QMetaObject::invokeMethod(this, &SortFilterProxyModel::PrivateFilter, Qt::QueuedConnection);
}

void SortFilterProxyModel::PrivateFilter()
{
    if (!enabled_)
        return;
    invalidateFilter();
    filtering_required_ = false;
}

void SortFilterProxyModel::OnDataChanged(const QModelIndex&, const QModelIndex&, const QVector<int>& roles /*= QVector<int>()*/)
{
    if (roles.isEmpty())
    {
        Invalidate();
    }
    else
    {
        OnItemDataChanged(Ids(roles.begin(), roles.end()));
    }
}

void SortFilterProxyModel::OnItemDataChanged(const Ids& roles)
{
    if (!filtering_required_ && SetsIntersect(filter_role_ids_, roles))
        PrivateFilter();

    if (!sorting_required_ && SetsIntersect(sort_role_ids_, roles))
        PrivateSort();
}

bool SortFilterProxyModel::lessThan(const QModelIndex& source_left, const QModelIndex& source_right) const
{
    for (const auto& role : sort_roles_)
    {
        auto comparison_result = comparator_ ? comparator_->CompareRows(source_left, source_right, role) : AbstractFilterComparatorBase::ComparisonResult::UNKNOWN;
        if (comparison_result == AbstractFilterComparatorBase::ComparisonResult::UNKNOWN)
            comparison_result = AbstractComparator::DefaultVariantCompare(source_left.data(role.id), source_right.data(role.id));

        switch (comparison_result)
        {
        case AbstractFilterComparatorBase::ComparisonResult::LESS:
            return true;
        case AbstractFilterComparatorBase::ComparisonResult::GREATER:
            return false;
        case AbstractFilterComparatorBase::ComparisonResult::EQUAL:
        case AbstractFilterComparatorBase::ComparisonResult::UNKNOWN:
        default:
            continue;
        }
    }
    return false;
}

bool SortFilterProxyModel::filterAcceptsRow(int source_row, const QModelIndex& source_parent) const
{
    return filter_ ? filter_->AcceptsRow(sourceModel()->index(source_row, 0, source_parent), filter_roles_) : true;
}
