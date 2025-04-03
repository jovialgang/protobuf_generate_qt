#pragma once

#include <QtQml>

#include "comparator_group.h"
#include "comparison_filter.h"
#include "enumeration_filter.h"
#include "filter_group.h"
#include "null_object_filter.h"
#include "object_list_comparator.h"
#include "object_model.h"
#include "range_filter.h"
#include "regular_expression_filter.h"
#include "settings.h"
#include "signal_timer.h"
#include "sort_filter_proxy_model.h"
#include "value_list_comparator.h"

namespace om
{
static void RegisterQmlTypes()
{
    qRegisterMetaType<om::Ids>("OmIds");
    qRegisterMetaType<om::IdsMap>("OmIdsMap");
    qRegisterMetaType<om::Names>("OmNames");
    qRegisterMetaType<om::NamesMap>("OmNamesMap");
    qRegisterMetaType<om::RolesVector>("RolesVector");
    qRegisterMetaType<om::ObjectRefModel *>("ObjectRefModel*");
    qRegisterMetaType<om::ObjectModel *>("ObjectModel*");

    qmlRegisterType<om::ObjectRefModel>("Om", 1, 0, "QtObjectRefModel");
    qmlRegisterType<om::ObjectModel>("Om", 1, 0, "QtObjectModel");
    qmlRegisterType<om::Settings>("Om", 1, 0, "Settings");
    qmlRegisterType<om::PropertyChangeListener>("Om", 1, 0, "PropertyChangeListener");
    qmlRegisterType<om::SettingsListener>("Om", 1, 0, "SettingsListener");
    qmlRegisterType<om::SortFilterProxyModel>("Om", 1, 0, "SortFilterProxyModel");
    qmlRegisterType<om::FilterGroup>("Om", 1, 0, "FilterGroup");
    qmlRegisterType<om::ComparatorGroup>("Om", 1, 0, "ComparatorGroup");
    qmlRegisterType<om::ComparisonFilter>("Om", 1, 0, "ComparisonFilter");
    qmlRegisterType<om::RangeFilter>("Om", 1, 0, "RangeFilter");
    qmlRegisterType<om::NullObjectFilter>("Om", 1, 0, "NullObjectFilter");
    qmlRegisterType<om::EnumerationFilter>("Om", 1, 0, "EnumerationFilter");
    qmlRegisterType<om::StringEnumerationFilter>("Om", 1, 0, "StringEnumerationFilter");
    qmlRegisterType<om::SubstringFilter>("Om", 1, 0, "SubstringFilter");
    qmlRegisterType<om::RegularExpressionFilter>("Om", 1, 0, "RegularExpressionFilter");
    qmlRegisterType<om::ObjectListComparator>("Om", 1, 0, "ObjectListComparator");
    qmlRegisterType<om::ValueListComparator>("Om", 1, 0, "ValueListComparator");

    qmlRegisterUncreatableType<om::SignalTimer>("Om", 1, 0, "SignalTimer", "Can not create gadget from qml");
    qmlRegisterUncreatableType<om::Role>("Om", 1, 0, "Role", "Can not create gadget from qml");
    qmlRegisterUncreatableType<om::AbstractFilterComparatorBase>("Om", 1, 0, "FilterComparator", "Filter is abstract!");
    qmlRegisterUncreatableType<om::AbstractFilter>("Om", 1, 0, "Filter", "Filter is abstract!");
    qmlRegisterUncreatableType<om::AbstractComparator>("Om", 1, 0, "Comparator", "Comparator is abstract!");
}
}  // namespace om