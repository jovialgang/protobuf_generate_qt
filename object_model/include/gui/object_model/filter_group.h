#pragma once

#include "abstract_filter.h"
#include <QQmlListProperty>
#include <QVector>

namespace om
{
class FilterGroup : public AbstractFilter
{
    Q_OBJECT
    Q_PROPERTY(int count READ GetCount NOTIFY filtersChanged)
    Q_PROPERTY(QVector<om::AbstractFilter*> filterList READ GetFilters WRITE SetFilters)
    // шаблонный аргумент QQmlListProperty<...> (видимо) должен быть прямым наследником QObject, иначе property будет невозможно пользоваться
    // ещё он не должен быть абстрактным (хотя здесь он абстрактный?) т.к. при регистрации типа нужен указатель на функцию создания QQmlPrivate::createInto<T>
    // template<typename T> void QQmlPrivate::createInto(void* memory) { new (memory) QQmlElement<T>; }
    Q_PROPERTY(QQmlListProperty<om::AbstractFilter> filters READ GetFiltersQmlListProperty)
    Q_CLASSINFO("DefaultProperty", "filters")
public:
    FilterGroup(QObject* parent = nullptr);
    ~FilterGroup() = default;

    bool AcceptsRow(const QModelIndex& index, const IdsMap& role_ids_map) const override;

    int GetCount() const;

    QVector<AbstractFilter*> GetFilters() const;
    void                     SetFilters(const QVector<AbstractFilter*>& val);

public slots:
    AbstractFilter* At(int index) const;
    void            Append(AbstractFilter* val);
    void            Clear();
    void            Remove(int index);
    void            Remove(AbstractFilter* val);

signals:
    void filtersChanged();

private:
    // Doesn't need implementation. All logic implemented in AcceptsRow
    bool Accepts(const QModelIndex&, const std::pair<QByteArray, int>&) const override { return true; }
    void UpdateRoles() override;

    void ConnectFilter(AbstractFilter* filter);
    void DisconnectFilter(AbstractFilter* filter);

    QQmlListProperty<AbstractFilter> GetFiltersQmlListProperty();
    static void                      Append(QQmlListProperty<AbstractFilter>* list, AbstractFilter* val);
    static int                       GetCount(QQmlListProperty<AbstractFilter>* list);
    static AbstractFilter*           At(QQmlListProperty<AbstractFilter>* list, int index);
    static void                      Clear(QQmlListProperty<AbstractFilter>* list);

    QVector<AbstractFilter*> filters_;
};
}  // namespace om
