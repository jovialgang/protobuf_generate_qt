#pragma once

#include "abstract_comparator.h"
#include "abstract_filter.h"
#include "abstract_object_model.h"
#include "dynamic_roles.h"
#include "model_access.h"

#include <QPointer>
#include <QSortFilterProxyModel>

namespace om
{
class SortFilterProxyModel : public QSortFilterProxyModel, public ListModelAccess, public AbstractDynamicRolesProvider
{
    Q_OBJECT
    Q_PROPERTY(bool enabled READ IsEnabled WRITE SetEnabled NOTIFY enabledChanged)
    Q_PROPERTY(QAbstractItemModel* model READ sourceModel WRITE setSourceModel NOTIFY modelChanged)
    Q_PROPERTY(AbstractComparator* comparator READ GetComparator WRITE SetComparator NOTIFY comparatorChanged)
    Q_PROPERTY(AbstractFilter* filter READ GetFilter WRITE SetFilter NOTIFY filterChanged)
    Q_PROPERTY(QStringList sortRoles READ GetSortRoles WRITE SetSortRoles NOTIFY sortRolesChanged)
    Q_PROPERTY(QByteArray sortRole READ GetSortRole WRITE SetSortRole NOTIFY sortRolesChanged)
    Q_PROPERTY(Qt::SortOrder sortOrder READ GetSortOrder WRITE SetSortOrder NOTIFY sortOrderChanged)
    Q_PROPERTY(int count READ rowCount NOTIFY rowCountChanged)
public:
    SortFilterProxyModel(QObject* parent = nullptr);
    virtual ~SortFilterProxyModel() = default;

    bool IsEnabled() const;
    void SetEnabled(bool val);

    AbstractComparator* GetComparator() const;
    void                SetComparator(AbstractComparator* val);

    AbstractFilter* GetFilter() const;
    void            SetFilter(AbstractFilter* val);

    QByteArray GetSortRole() const;
    void       SetSortRole(const QByteArray& val);

    QStringList GetSortRoles() const;
    void        SetSortRoles(const QStringList& val);

    Ids GetDynamicRoles() const override;

    Qt::SortOrder GetSortOrder() const;
    void          SetSortOrder(Qt::SortOrder val);

    void setSourceModel(QAbstractItemModel* val) override;

    QVariant sourceData(const QModelIndex& index, int role = Qt::DisplayRole) const;

    QVariant GetData(int row, int role) const override;
    bool     SetData(int row, const QVariant& value, int role) override;

    QHash<QByteArray, int> roleIds() const override;

public slots:
    void Sort();
    void Filter();
    void Invalidate();
    void ChangeSortOrder();

    int MapFromSource(int source_row);
    int MapToSource(int row);

    int GetCount() const override;

    QVariant GetData(int row, const QByteArray& role_name = AbstractObjectModel::kItemRoleName) const override;
    bool     SetData(int row, const QVariant& val, const QByteArray& role_name = AbstractObjectModel::kItemRoleName) override;

    int IndexOf(const QByteArray& property_name, const QVariant& val) const;

signals:
    void enabledChanged();
    void modelChanged();
    void comparatorChanged();
    void filterChanged();
    void sortRolesChanged();
    void sortOrderChanged();
    void rowCountChanged();
    void dynamicRolesChanged();

protected slots:
    void PrivateSort();
    void PrivateFilter();
    void UpdateRoleIds();
    void UpdateSortRoleIds();
    void UpdateFilterRoleIds();
    void OnItemDataChanged(const Ids& roles = Ids());
    void OnDataChanged(const QModelIndex& topLeft, const QModelIndex& bottomRight, const QVector<int>& roles = QVector<int>());
    void OnFilterRolesChanged();

protected:
    bool lessThan(const QModelIndex& source_left, const QModelIndex& source_right) const override;
    bool filterAcceptsRow(int source_row, const QModelIndex& source_parent) const override;
    void OnDynamicRolesChanged() override;

private:
//    COMPONENT_LOGGER("ui.sfpm");

    QHash<QByteArray, int> role_ids_;

    QPointer<AbstractObjectModel> source_object_model_;
    QPointer<AbstractComparator>  comparator_;
    QPointer<AbstractFilter>      filter_;

    RolesVector sort_roles_;
    Ids         sort_role_ids_;
    om::IdsMap  filter_roles_;
    Ids         filter_role_ids_;

    Qt::SortOrder sort_order_ = Qt::AscendingOrder;

    bool enabled_            = true;
    bool sorting_required_   = false;
    bool filtering_required_ = false;
};
}  // namespace om
