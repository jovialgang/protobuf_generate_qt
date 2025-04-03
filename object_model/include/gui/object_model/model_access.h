#pragma once

#include "roles.h"
#include <QByteArray>
#include <QHash>
#include <QVariant>

namespace om
{
class ListModelAccess
{
public:
    virtual ~ListModelAccess() = default;

    virtual QHash<QByteArray, int> roleIds() const = 0;
    virtual Id                     GetRoleId(const QByteArray& role_name) const
    {
        const auto& role_ids = roleIds();
        auto        role_id  = role_ids.find(role_name);
        return role_id != role_ids.end() ? role_id.value() : kInvalidId;
    }

    virtual int      GetCount() const                 = 0;
    virtual QVariant GetData(int row, int role) const = 0;
    virtual QVariant GetData(int row, const QByteArray& role_name) const { return GetData(row, GetRoleId(role_name)); };
    virtual bool     SetData(int, const QVariant&, int) { return false; };
    virtual bool     SetData(int row, const QVariant& value, const QByteArray& role_name) { return SetData(row, value, GetRoleId(role_name)); };
};
}  // namespace om
