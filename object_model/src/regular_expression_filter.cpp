#include "regular_expression_filter.h"

using namespace om;

SubstringFilter::SubstringFilter(QObject* parent /*= Q_NULLPTR*/) : AbstractRoleFilter(parent)
{}

QString SubstringFilter::GetSubstring() const
{
    return substring_;
}

void SubstringFilter::SetSubstring(const QString& val)
{
    if (substring_ == val)
        return;
    substring_ = val;
    emit substringChanged();
    EmitFilterChanged();
}

bool SubstringFilter::IsCaseInsensitive() const
{
    return case_sensitivity_ == Qt::CaseInsensitive;
}

void SubstringFilter::SetCaseInsensitive(bool val)
{
    if (IsCaseInsensitive() == val)
        return;
    case_sensitivity_ = val ? Qt::CaseInsensitive : Qt::CaseSensitive;
    emit caseSensitivityChanged();
    EmitFilterChanged();
}

bool SubstringFilter::Accepts(const QModelIndex& index, const std::pair<QByteArray, int>& role) const
{
    return substring_.length() ? index.data(role.second).toString().contains(substring_, case_sensitivity_) : true;
}

//

RegularExpressionFilter::RegularExpressionFilter(QObject* parent /*= Q_NULLPTR*/) : AbstractRoleFilter(parent)
{}

QString RegularExpressionFilter::GetPattern() const
{
    return regular_expression_.pattern();
}

void RegularExpressionFilter::SetPattern(const QString& val)
{
    if (regular_expression_.pattern() == val)
        return;
    regular_expression_.setPattern(val);
    regular_expression_.optimize();
    emit regularExpressionChanged();
    EmitFilterChanged();
}

QRegularExpression::PatternOptions RegularExpressionFilter::GetPatternOptions() const
{
    return regular_expression_.patternOptions();
}

void RegularExpressionFilter::SetPatternOptions(QRegularExpression::PatternOptions val)
{
    if (regular_expression_.patternOptions() == val)
        return;
    regular_expression_.setPatternOptions(val);
    emit regularExpressionChanged();
    EmitFilterChanged();
}

bool RegularExpressionFilter::IsCaseInsensitive() const
{
    return regular_expression_.patternOptions().testFlag(QRegularExpression::CaseInsensitiveOption);
}

void RegularExpressionFilter::SetCaseInsensitive(bool val)
{
    if (IsCaseInsensitive() == val)
        return;
    regular_expression_.setPatternOptions(regular_expression_.patternOptions().setFlag(QRegularExpression::CaseInsensitiveOption, val));
    emit regularExpressionChanged();
    EmitFilterChanged();
}

QRegularExpression RegularExpressionFilter::GetRegularExpression() const
{
    return regular_expression_;
}

void RegularExpressionFilter::SetRegularExpression(const QRegularExpression& val)
{
    if (regular_expression_ == val)
        return;
    regular_expression_ = val;
    regular_expression_.optimize();
    emit regularExpressionChanged();
    EmitFilterChanged();
}

bool RegularExpressionFilter::Accepts(const QModelIndex& index, const std::pair<QByteArray, int>& role) const
{
    return regular_expression_.match(index.data(role.second).toString()).hasMatch();
}
