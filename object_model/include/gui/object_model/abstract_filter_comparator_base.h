#pragma once

#include "roles.h"
#include <QObject>
#include <QVariant>

namespace om
{
class AbstractFilterComparatorBase : public QObject
{
    Q_OBJECT
    Q_PROPERTY(bool enabled READ IsEnabled WRITE SetEnabled NOTIFY enabledChanged)
public:
    enum class LogicalOperator { AND, OR };
    Q_ENUM(LogicalOperator)

    enum class ComparisonResult { UNKNOWN, EQUAL, LESS, GREATER };
    Q_ENUM(ComparisonResult)

    enum class RangeCheckType { INSIDE = 0, OUTSIDE = 1, INSIDE_OR_EQUAL = 2, OUTSIDE_OR_EQUAL = 3 };
    Q_ENUM(RangeCheckType)

    AbstractFilterComparatorBase(QObject* parent = nullptr);

    const Names& GetRoles() const;
    bool         HasRole(const QByteArray& role) const;

    bool IsEnabled() const;
    void SetEnabled(bool val);

    QByteArray  GetRole() const;
    QStringList GetRoleNames() const;

    // Compares two variants of default types
    static ComparisonResult DefaultVariantCompare(const QVariant& lhs, const QVariant& rhs);

signals:
    void enabledChanged();
    void rolesChanged();

protected:
    // accepts QString, QStringList, QByteArray and QByteArrayList
    void SetRoles(const QVariant& val);

    // converts QString, QStringList, QByteArray and QByteArrayList to RoleNames
    static Names RolesFromVariant(const QVariant& val);

    // calls UpdateRoles and emits rolesChanged signal if is enabled
    void         EmitRolesChanged();
    virtual void UpdateRoles(){};

    bool  enabled_ = true;
    Names roles_;
};
}  // namespace om
