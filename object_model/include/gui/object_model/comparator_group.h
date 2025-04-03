#pragma once

#include "abstract_comparator.h"

#include <QQmlListProperty>
#include <QVector>

namespace om
{
class ComparatorGroup : public AbstractComparator
{
    Q_OBJECT
    Q_PROPERTY(int count READ GetCount NOTIFY comparatorsChanged)
    Q_PROPERTY(QVector<AbstractComparator*> comparatorList READ GetComparators WRITE SetComparators NOTIFY comparatorsChanged)
    Q_PROPERTY(QQmlListProperty<om::AbstractComparator> comparators READ GetComparatorsQmlListProperty)
    Q_CLASSINFO("DefaultProperty", "comparators")

public:
    ComparatorGroup(QObject* parent = nullptr);
    ~ComparatorGroup() = default;

    int GetCount() const;

    QVector<AbstractComparator*> GetComparators() const;
    void                         SetComparators(const QVector<AbstractComparator*>& val);

    AbstractComparator* GetComparator(const QByteArray& role) const;

public slots:
    AbstractComparator* At(int index) const;
    void                Append(AbstractComparator* val);
    void                Clear();
    void                Remove(int index);
    void                Remove(AbstractComparator* val);

signals:
    void comparatorsChanged();

protected:
    ComparisonResult Compare(const QModelIndex& source_left, const QModelIndex& source_right, const Role& role) const override;

    void UpdateRoles() override;
    void ConnectComparator(AbstractComparator* comparator);
    void DisconnectComparator(AbstractComparator* comparator);

private:
    QQmlListProperty<AbstractComparator> GetComparatorsQmlListProperty();

    static void                Append(QQmlListProperty<AbstractComparator>* list, AbstractComparator* val);
    static int                 GetCount(QQmlListProperty<AbstractComparator>* list);
    static AbstractComparator* At(QQmlListProperty<AbstractComparator>* list, int index);
    static void                Clear(QQmlListProperty<AbstractComparator>* list);

    QVector<AbstractComparator*> comparators_;
};
}  // namespace om
