#pragma once

#include "abstract_filter.h"

#include <QRegularExpression>

namespace om
{
class SubstringFilter : public AbstractRoleFilter
{
    Q_OBJECT
    Q_PROPERTY(bool caseInsensitive READ IsCaseInsensitive WRITE SetCaseInsensitive NOTIFY caseSensitivityChanged)
    Q_PROPERTY(QString substring READ GetSubstring WRITE SetSubstring NOTIFY substringChanged)
public:
    SubstringFilter(QObject* parent = Q_NULLPTR);

    QString GetSubstring() const;
    void    SetSubstring(const QString& val);

    bool IsCaseInsensitive() const;
    void SetCaseInsensitive(bool val);

signals:
    void substringChanged();
    void caseSensitivityChanged();

protected:
    bool Accepts(const QModelIndex& index, const std::pair<QByteArray, int>& role) const override;

private:
    Qt::CaseSensitivity case_sensitivity_ = Qt::CaseInsensitive;
    QString             substring_;
};

class RegularExpressionFilter : public AbstractRoleFilter
{
    Q_OBJECT
    Q_PROPERTY(QString pattern READ GetPattern WRITE SetPattern NOTIFY regularExpressionChanged)
    Q_PROPERTY(bool caseInsensitive READ IsCaseInsensitive WRITE SetCaseInsensitive NOTIFY regularExpressionChanged)
    Q_PROPERTY(QRegularExpression::PatternOptions patternOptions READ GetPatternOptions WRITE SetPatternOptions NOTIFY regularExpressionChanged)
    Q_PROPERTY(QRegularExpression regularExpression READ GetRegularExpression WRITE SetRegularExpression NOTIFY regularExpressionChanged)
public:
    RegularExpressionFilter(QObject* parent = Q_NULLPTR);

    QRegExp GetRegExp() const;
    void    SetRegExp(const QRegExp& val);

    bool IsCaseInsensitive() const;
    void SetCaseInsensitive(bool val);

    QString GetPattern() const;
    void    SetPattern(const QString& val);

    QRegularExpression::PatternOptions GetPatternOptions() const;
    void                               SetPatternOptions(QRegularExpression::PatternOptions val);

    QRegularExpression GetRegularExpression() const;
    void               SetRegularExpression(const QRegularExpression& val);

signals:
    void regularExpressionChanged();

protected:
    bool Accepts(const QModelIndex& index, const std::pair<QByteArray, int>& role) const override;

private:
    QRegularExpression regular_expression_;
};
}  // namespace om
