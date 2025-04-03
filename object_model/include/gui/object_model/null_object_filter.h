#pragma once

#include "abstract_filter.h"
#include <QVariant>

namespace om
{
class NullObjectFilter : public AbstractRoleFilter
{
    Q_OBJECT
public:
    // default role of this filter is ObjectMetaData::kItemRoleName
    NullObjectFilter(QObject* parent = nullptr);

protected:
    bool Accepts(const QModelIndex& index, const std::pair<QByteArray, int>& role) const override;
};
}  // namespace om
