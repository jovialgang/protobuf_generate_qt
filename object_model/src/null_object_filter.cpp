#include "null_object_filter.h"
#include "object_meta_data.h"

using namespace om;

NullObjectFilter::NullObjectFilter(QObject* parent /*= nullptr*/) : AbstractRoleFilter(parent)
{
    roles_ = { ObjectMetaData::kItemRoleName };
}

bool NullObjectFilter::Accepts(const QModelIndex& index, const std::pair<QByteArray, int>& role) const
{
    return index.data(role.second).value<QObject*>();
}
