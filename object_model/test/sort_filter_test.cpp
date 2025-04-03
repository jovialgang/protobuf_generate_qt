#include "test_object.h"

#include <gui/object_model/comparator.h>
#include <gui/object_model/comparison_filter.h>
#include <gui/object_model/enumeration_filter.h>
#include <gui/object_model/object_model.h>
#include <gui/object_model/object_model_qml.h>
#include <gui/object_model/range_filter.h>
#include <gui/object_model/regular_expression_filter.h>
#include <gui/object_model/sort_filter_proxy_model.h>
#include <QCoreApplication>
#include <QPointer>
#include <QSignalSpy>
#include <array>
#include <gtest/gtest.h>
#include <log.h>
#include <memory>
#include <type_traits>

#include <cmath>

using namespace om;

class SortFilterModelFixture : public ::testing::Test
{
public:
    void SetUp() override
    {
        int   args = 1;
        char* argv = "";
        om::RegisterQmlTypes();
        app_          = std::make_unique<QCoreApplication>(args, &argv);
        model_        = std::make_unique<SortFilterProxyModel>();
        source_model_ = std::make_unique<ObjectModel>();
        for (int i = 0; i < objects_.size(); ++i)
        {
            objects_[i] = new TestObject(i, QString::number(2 - i), dummy_parent_.get());
            objects_[i]->setObjectName(QString("objectName%1").arg(i));
            objects_[i]->SetCoord(new CoordObject(std::floor(i / 2), std::sin(i * M_PI / 2), objects_[i]));
            objects_[i]->GetCoord()->SetType(new CoordTypeObject(static_cast<CoordTypeObject::Type>(3 - i), objects_[i]));
        }
        source_model_->AppendVector({ objects_.begin(), objects_.end() });
    }

    void TearDown() override
    {
        objects_.fill(nullptr);
        source_model_.reset();
        model_.reset();
        dummy_parent_.reset();
        app_.reset();
    }

    QVector<QObject*> GetItems() const
    {
        QVector<QObject*> res;
        for (int i = 0; i < model_->rowCount(); ++i)
        {
            res.push_back(model_->data(model_->index(i, 0), AbstractObjectModel::kItemRole).value<QObject*>());
        }
        return res;
    }

protected:
    // COMPONENT_LOGGER("test");
    std::unique_ptr<QCoreApplication>     app_;
    std::unique_ptr<ObjectModel>          source_model_;
    std::unique_ptr<SortFilterProxyModel> model_;
    std::unique_ptr<QObject>              dummy_parent_ = nullptr;
    std::array<TestObject*, 3>            objects_;
};

// TODO: добавить тест на нотифаер на нескольких свойствах

TEST_F(SortFilterModelFixture, SetModel)
{
    QSignalSpy model_reset_signal(model_.get(), &SortFilterProxyModel::modelReset);
    QSignalSpy model_changed_signal(model_.get(), &SortFilterProxyModel::modelChanged);
    model_->setSourceModel(source_model_.get());
    EXPECT_EQ(GetItems(), source_model_->GetItems());
    EXPECT_EQ(model_reset_signal.count(), 1);
    EXPECT_EQ(model_changed_signal.count(), 1);
}

TEST_F(SortFilterModelFixture, ResetModel)
{
    QSignalSpy layout_changed_signal(model_.get(), &SortFilterProxyModel::layoutChanged);
    model_->setSourceModel(source_model_.get());
    model_->SetSortRole("id");
    objects_[0]->SetId(100);
    QCoreApplication::processEvents();
    EXPECT_EQ(layout_changed_signal.count(), 1);
    layout_changed_signal.takeFirst();
    model_->setSourceModel(nullptr);
    objects_[1]->SetId(200);
    EXPECT_EQ(layout_changed_signal.count(), 0);  // мы отключили модель, новой сортировки не должно было произойти
}

TEST_F(SortFilterModelFixture, Sort)
{
    model_->SetSortRole("name");
    model_->setSourceModel(source_model_.get());
    QCoreApplication::processEvents();
    EXPECT_EQ(GetItems(), QVector<QObject*>({ objects_[2], objects_[1], objects_[0] }));
    model_->SetSortRole("id");
    QCoreApplication::processEvents();
    EXPECT_EQ(GetItems(), QVector<QObject*>({ objects_[0], objects_[1], objects_[2] }));
    model_->SetSortRoles({ "coord.x", "coord.type.type" });
    QCoreApplication::processEvents();
    EXPECT_EQ(GetItems(), QVector<QObject*>({ objects_[1], objects_[0], objects_[2] }));
}

TEST_F(SortFilterModelFixture, OnItemsPropertiesChangedSort)
{
    model_->setSourceModel(source_model_.get());
    model_->SetSortRoles({ "coord.x", "coord.type.type" });
    QCoreApplication::processEvents();
    EXPECT_EQ(GetItems(), QVector<QObject*>({ objects_[1], objects_[0], objects_[2] }));
    objects_[1]->GetCoord()->SetX(1);
    QCoreApplication::processEvents();
    EXPECT_EQ(GetItems(), QVector<QObject*>({ objects_[0], objects_[2], objects_[1] }));
    objects_[1]->GetCoord()->GetType()->SetType(CoordTypeObject::Type::Type0);
    QCoreApplication::processEvents();
    EXPECT_EQ(GetItems(), QVector<QObject*>({ objects_[0], objects_[1], objects_[2] }));
    model_->SetSortRole("id");
    objects_[0]->SetId(100);
    QCoreApplication::processEvents();
    EXPECT_EQ(GetItems(), QVector<QObject*>({ objects_[1], objects_[2], objects_[0] }));
}

TEST_F(SortFilterModelFixture, OnItemsChangedSort)
{
    model_->setSourceModel(source_model_.get());
    model_->SetSortRoles({ "coord.x", "coord.type.type" });
    source_model_->Take(1);
    QCoreApplication::processEvents();
    EXPECT_EQ(GetItems(), QVector<QObject*>({ objects_[0], objects_[2] }));
    objects_[1]->GetCoord()->SetX(1);
    source_model_->Append(objects_[1]);
    QCoreApplication::processEvents();
    EXPECT_EQ(GetItems(), QVector<QObject*>({ objects_[0], objects_[2], objects_[1] }));
}

TEST_F(SortFilterModelFixture, SortOrder)
{
    model_->setSourceModel(source_model_.get());
    model_->SetSortRole("id");
    model_->ChangeSortOrder();
    QCoreApplication::processEvents();
    EXPECT_EQ(GetItems(), QVector<QObject*>({ objects_[2], objects_[1], objects_[0] }));
    model_->ChangeSortOrder();
    QCoreApplication::processEvents();
    EXPECT_EQ(GetItems(), QVector<QObject*>({ objects_[0], objects_[1], objects_[2] }));
}

TEST_F(SortFilterModelFixture, Comparator)
{
    model_->setSourceModel(source_model_.get());
    model_->SetSortRole("id");
    auto comparator = new Comparator(
        [](const QModelIndex& l_index, const QModelIndex& r_index, const Role& role) {
            auto l   = l_index.data(role.id).toInt();
            auto r   = r_index.data(role.id).toInt();
            auto res = AbstractFilterComparatorBase::DefaultVariantCompare(l % 2, r % 2);  // нечетные числа будут считаться больше четных
            return res == AbstractFilterComparatorBase::ComparisonResult::EQUAL ? AbstractFilterComparatorBase::DefaultVariantCompare(l, r) : res;  // иначе просто сравниваем числа
        },
        model_.get());
    comparator->SetRole("id");
    model_->SetComparator(comparator);
    QCoreApplication::processEvents();
    EXPECT_EQ(GetItems(), QVector<QObject*>({ objects_[0], objects_[2], objects_[1] }));
}

TEST_F(SortFilterModelFixture, SignalRoleComparator)
{
    model_->setSourceModel(source_model_.get());
    model_->SetSortRole("coord.type.changed");
    auto comparator = new Comparator(
        [](const QModelIndex& l_index, const QModelIndex& r_index, const Role& role) {
            auto l = l_index.data(role.id).value<CoordTypeObject*>();
            auto r = r_index.data(role.id).value<CoordTypeObject*>();
            if (l == r)
                return AbstractFilterComparatorBase::ComparisonResult::EQUAL;
            if (!l)
                return AbstractFilterComparatorBase::ComparisonResult::LESS;
            if (!r)
                return AbstractFilterComparatorBase::ComparisonResult::GREATER;
            return AbstractFilterComparatorBase::DefaultVariantCompare(static_cast<int>(l->GetType()), static_cast<int>(r->GetType()));
        },
        model_.get());
    comparator->SetRole("coord.type.changed");
    model_->SetComparator(comparator);
    QCoreApplication::processEvents();
    EXPECT_EQ(GetItems(), QVector<QObject*>({ objects_[2], objects_[1], objects_[0] }));
    objects_[0]->GetCoord()->GetType()->SetType(CoordTypeObject::Type::Type0);
    QCoreApplication::processEvents();
    EXPECT_EQ(GetItems(), QVector<QObject*>({ objects_[2], objects_[1], objects_[0] }));  // ничего не поменялось, так как не вызывали сигнал
    objects_[0]->GetCoord()->GetType()->EmitChanged();
    QCoreApplication::processEvents();
    EXPECT_EQ(GetItems(), QVector<QObject*>({ objects_[0], objects_[2], objects_[1] }));
    delete objects_[2]->GetCoord();
    objects_[2]->SetCoord(nullptr);
    QCoreApplication::processEvents();
    EXPECT_EQ(GetItems(), QVector<QObject*>({ objects_[2], objects_[0], objects_[1] }));
}

TEST_F(SortFilterModelFixture, ComparisonFilter)
{
    model_->setSourceModel(source_model_.get());
    auto filter = new ComparisonFilter(model_.get());
    model_->SetFilter(filter);
    filter->SetRole("id");
    filter->SetComparisonValue(1);
    filter->SetComparisonOperator(ComparisonFilter::ComparisonOperator::LESS);
    QCoreApplication::processEvents();
    EXPECT_EQ(GetItems(), QVector<QObject*>({ objects_[0] }));
    filter->SetComparisonOperator(ComparisonFilter::ComparisonOperator::GREATER_OR_EQUAL);
    QCoreApplication::processEvents();
    EXPECT_EQ(GetItems(), QVector<QObject*>({ objects_[1], objects_[2] }));
}

TEST_F(SortFilterModelFixture, EnumerationFilter)
{
    auto filter = new EnumerationFilter(model_.get());
    model_->SetFilter(filter);
    filter->SetRole("id");
    filter->SetValues({ 2 });
    QCoreApplication::processEvents();
    model_->setSourceModel(source_model_.get());
    QCoreApplication::processEvents();
    EXPECT_EQ(GetItems(), QVector<QObject*>({ objects_[2] }));
    filter->AddValue(0);
    QCoreApplication::processEvents();
    EXPECT_EQ(GetItems(), QVector<QObject*>({ objects_[0], objects_[2] }));
}

TEST_F(SortFilterModelFixture, RangeFilter)
{
    model_->setSourceModel(source_model_.get());
    auto filter = new RangeFilter(model_.get());
    model_->SetFilter(filter);
    filter->SetRangeCheckType(AbstractFilterComparatorBase::RangeCheckType::INSIDE_OR_EQUAL);
    filter->SetRole("id");
    filter->SetFrom(0);
    filter->SetTo(1);
    QCoreApplication::processEvents();
    EXPECT_EQ(GetItems(), QVector<QObject*>({ objects_[0], objects_[1] }));
    filter->SetRole("coord.x");
    QCoreApplication::processEvents();
    EXPECT_EQ(GetItems(), QVector<QObject*>({ objects_[0], objects_[1], objects_[2] }));
}

TEST_F(SortFilterModelFixture, RegularExpressionFilter)
{
    model_->setSourceModel(source_model_.get());
    auto filter = new RegularExpressionFilter(model_.get());
    model_->SetFilter(filter);
    filter->SetRole("objectName");
    filter->SetPattern(".*1");
    QCoreApplication::processEvents();
    EXPECT_EQ(GetItems(), QVector<QObject*>({ objects_[1] }));
    filter->SetPattern("objectName[0|2]");
    QCoreApplication::processEvents();
    EXPECT_EQ(GetItems(), QVector<QObject*>({ objects_[0], objects_[2] }));
}
