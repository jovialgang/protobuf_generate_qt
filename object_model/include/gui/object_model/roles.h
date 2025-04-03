#pragma once
#include <QByteArray>
#include <QMetaType>
#include <QVariant>
#include <algorithm>
#include <boost/container/flat_map.hpp>
#include <boost/container/flat_set.hpp>
#include <boost/container/small_vector.hpp>
#include <functional>


namespace om
{
using Id = int;
//Q_DECLARE_METATYPE(Id)

using Ids = boost::container::flat_set<int, std::less<int>, boost::container::small_vector<int, 10>>;
//Q_DECLARE_METATYPE(Ids)

using IdsMap = boost::container::flat_map<QByteArray, int>;
//Q_DECLARE_METATYPE(IdsMap)

using Names = boost::container::flat_set<QByteArray, std::less<QByteArray>, boost::container::small_vector<QByteArray, 10>>;
//Q_DECLARE_METATYPE(Names)

using NamesMap = boost::container::flat_map<int, QByteArray>;
//Q_DECLARE_METATYPE(NamesMap)

constexpr Id kInvalidId = -1;

struct Role
{
    Q_GADGET
public:
    enum RolesType {
        ITEM_ROLE         = 0x0,                         //
        OWN_ROLES         = 0x1,                         //
        INHERITED_ROLES   = 0x2,                         //
        OBJECT_NAME_ROLES = 0x4,                         //
        SIGNAL_ROLES      = 0x8,                         //
        ALL_ROLES         = OWN_ROLES | INHERITED_ROLES  //
    };
    Q_ENUM(RolesType)
    Q_DECLARE_FLAGS(Roles, RolesType)

    Role(Id id = 0, const QByteArray& name = QByteArray()) : id(id), name(name) {}

    Id         id;
    QByteArray name;

    bool operator==(const Role& r) { return id == r.id; }
    bool operator<(const Role& r) { return id < r.id; }
};
Q_DECLARE_OPERATORS_FOR_FLAGS(Role::Roles)
//Q_DECLARE_METATYPE(Role::Roles)
//Q_DECLARE_METATYPE(Role)

using RolesVector = boost::container::small_vector<Role, 10>;
//Q_DECLARE_METATYPE(RolesVector)

struct RoleValue
{
    Id       role_id;
    QVariant value;
};
//Q_DECLARE_METATYPE(RoleValue)

using RoleValuesVector = boost::container::small_vector<RoleValue, 10>;
//Q_DECLARE_METATYPE(RoleValuesVector)

template <typename T>
inline bool SetsIntersect(const T& s1, const T& s2)
{
    if (s1.empty() || s2.empty())
        return false;
    for (const auto& i : s1)
        if (s2.contains(i))
            return true;
    return false;
}

template <typename T>
inline bool SetIntersection(const T& s1, const T& s2, T& res_s)
{
    if (s1.empty() || s2.empty())
        return false;
    bool res = false;
    for (const auto& i : s1)
        if (s2.contains(i))
        {
            res_s.insert(i);
            res = true;
        }
    return res;
}
}  // namespace om

Q_DECLARE_METATYPE(om::Id)
Q_DECLARE_METATYPE(om::Ids)
Q_DECLARE_METATYPE(om::IdsMap)
Q_DECLARE_METATYPE(om::Names)
Q_DECLARE_METATYPE(om::NamesMap)
Q_DECLARE_METATYPE(om::Role::Roles)
Q_DECLARE_METATYPE(om::Role)
Q_DECLARE_METATYPE(om::RolesVector)
Q_DECLARE_METATYPE(om::RoleValue)
Q_DECLARE_METATYPE(om::RoleValuesVector)