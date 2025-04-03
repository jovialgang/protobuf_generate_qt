#pragma once

#include "abstract_filter.h"

#include <boost/container/flat_set.hpp>

namespace om
{
class EnumerationFilter : public AbstractRoleFilter
{
    Q_OBJECT
    Q_PROPERTY(QVariantList values READ GetValues WRITE SetValues RESET ClearValues NOTIFY valuesChanged)
public:
    EnumerationFilter(QObject* parent = nullptr);

    QVariantList GetValues() const;
    void         SetValues(const QVariantList& values);

public slots:
    bool HasValue(int val);
    void SetValue(int val, bool enabled);
    void AddValue(int val);
    void RemoveValue(int val);
    void ClearValues();

signals:
    void valuesChanged();

protected:
    bool Accepts(const QModelIndex& index, const std::pair<QByteArray, int>& role) const override;

private:
    boost::container::flat_set<int> values_;
};

class StringEnumerationFilter : public AbstractRoleFilter
{
    Q_OBJECT
    Q_PROPERTY(QStringList values READ GetValues WRITE SetValues RESET ClearValues NOTIFY valuesChanged)
public:
    StringEnumerationFilter(QObject* parent = nullptr);

    QStringList GetValues() const;
    void        SetValues(const QStringList& values);

public slots:
    bool HasValue(const QString& val);
    void SetValue(const QString& val, bool enabled);
    void AddValue(const QString& val);
    void RemoveValue(const QString& val);
    void ClearValues();

signals:
    void valuesChanged();

protected:
    bool Accepts(const QModelIndex& index, const std::pair<QByteArray, int>& role) const override;

private:
    boost::container::flat_set<QString> values_;
};
}  // namespace om
