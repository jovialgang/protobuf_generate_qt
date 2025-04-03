#pragma once

#include "abstract_comparator.h"

#include <QObject>

namespace om
{
// Компаратор для сравнения списков объектов - в индексах ожидаются наследники AbstractObjectModel
// сравнение конечных значений происходит дефолтным способом
class ObjectListComparator : public AbstractRoleComparator
{
    Q_OBJECT
    // роль ObjectModel, по которой значения моделей будут сравниваться
    Q_PROPERTY(QByteArray valueModelRole READ GetValueModelRole WRITE SetValueModelRole NOTIFY valueModelRoleChanged)
public:
    ObjectListComparator(QObject* parent = Q_NULLPTR);

    QByteArray GetValueModelRole() const;
    void       SetValueModelRole(const QByteArray& role);

signals:
    void valueModelRoleChanged();

protected:
    ComparisonResult Compare(const QModelIndex& source_left, const QModelIndex& source_right, const Role& role) const override;

    QByteArray value_model_role_;
};
}  // namespace om
