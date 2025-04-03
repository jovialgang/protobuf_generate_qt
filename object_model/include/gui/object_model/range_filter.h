#pragma once

#include "abstract_filter.h"

namespace om
{
class RangeFilter : public AbstractRoleFilter
{
    Q_OBJECT
    Q_PROPERTY(double from READ GetFrom WRITE SetFrom NOTIFY fromChanged)
    Q_PROPERTY(double to READ GetTo WRITE SetTo NOTIFY toChanged)
    Q_PROPERTY(RangeCheckType checkType READ GetRangeCheckType WRITE SetRangeCheckType NOTIFY rangeCheckTypeChanged)
public:
    RangeFilter(QObject* parent = Q_NULLPTR);

    double GetFrom() const;
    void   SetFrom(double from);

    double GetTo() const;
    void   SetTo(double to);

    RangeCheckType GetRangeCheckType() const;
    void           SetRangeCheckType(RangeCheckType type);

signals:
    void fromChanged();
    void toChanged();
    void rangeCheckTypeChanged();

protected:
    bool Accepts(const QModelIndex& index, const std::pair<QByteArray, int>& role) const override;

private:
    double from_ = 0;
    double to_   = 100;

    RangeCheckType type_ = RangeCheckType::INSIDE;
};
}  // namespace om
