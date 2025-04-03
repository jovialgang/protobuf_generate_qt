#include "qt_core_test.h"
#include <test_qt_pb.h>

#include <QDebug>
#include <QGuiApplication>
#include <QSignalSpy>
#include <QTest>
#include <QModelIndex>
#include <gtest/gtest.h>
#include <array>
#include <memory>

namespace prototest
{
enum TestEnumModelSignals { kChanged = 0 };

class TestEnumModelFixture : public QtCoreTest
{
protected:
    void SetUp() override;
    void TearDown() override;

    void SignalsTrackingSetUp() override;
    // Увеличение счетчика всех ожидаемых сигналов на count
    void IncSignals(const std::set<TestEnumModelSignals>& signals_set, int count = 1);

    // Проверка совпадения ожидаемого количества сигналов и пойманного
    void CheckSignals() override;

protected:
    void SetGet(bool is_rvalue);

protected:
    std::unique_ptr<google::protobuf::RepeatedField<int>> data_;
    // Тестируемый класс
    std::unique_ptr<protogeneratorqt::TestEnumModel> enum_model_;
};

void TestEnumModelFixture::SetUp()
{
    AppSetUp();
    data_  = std::make_unique<google::protobuf::RepeatedField<int>>();
    enum_model_ = std::make_unique<protogeneratorqt::TestEnumModel>(data_.get());
    SignalsTrackingSetUp();
}

void TestEnumModelFixture::TearDown()
{
    AppTearDown();
    enum_model_.reset();
}

void TestEnumModelFixture::SignalsTrackingSetUp()
{
    signals_counts_ = std::vector(1, 0);
    signals_spies_.push_back(std::make_unique<QSignalSpy>(enum_model_.get(), &protogeneratorqt::TestEnumModel::changed));
}

void TestEnumModelFixture::IncSignals(const std::set<TestEnumModelSignals>& signals_set, int count /*= 1*/)
{
    for (auto i : signals_set) signals_counts_[i] += count;
}

void TestEnumModelFixture::CheckSignals()
{
    EXPECT_EQ(signals_spies_[TestEnumModelSignals::kChanged]->count(), signals_counts_[TestEnumModelSignals::kChanged]);
}

void TestEnumModelFixture::SetGet(bool is_rvalue)
{
    google::protobuf::RepeatedField<int> data_in_first;
    data_in_first.Add(protogeneratorqt::TestEnumEnum::OPTION_1);
    data_in_first.Add(protogeneratorqt::TestEnumEnum::OPTION_2);

    if (is_rvalue)
        enum_model_->Set(std::move(data_in_first));
    else
        enum_model_->Set(data_in_first);

    auto data_out_first = enum_model_->Get();
    EXPECT_EQ(data_out_first[0], protogeneratorqt::TestEnumEnum::OPTION_1);
    EXPECT_EQ(data_out_first[1], protogeneratorqt::TestEnumEnum::OPTION_2);
    IncAllSignals();
    ApplicationCheckSignals();

    google::protobuf::RepeatedField<int> data_in_second;
    data_in_second.Add(protogeneratorqt::TestEnumEnum::OPTION_2);
    data_in_second.Add(protogeneratorqt::TestEnumEnum::OPTION_1);

    if (is_rvalue)
        enum_model_->Set(std::move(data_in_second));
    else
        enum_model_->Set(data_in_second);

    auto data_out_second = enum_model_->Get();
    EXPECT_EQ(data_out_second[0], protogeneratorqt::TestEnumEnum::OPTION_2);
    EXPECT_EQ(data_out_second[1], protogeneratorqt::TestEnumEnum::OPTION_1);
    IncAllSignals();
    ApplicationCheckSignals();

    google::protobuf::RepeatedField<int> data_in_third;
    data_in_third.Add(protogeneratorqt::TestEnumEnum::OPTION_2);
    data_in_third.Add(protogeneratorqt::TestEnumEnum::OPTION_1);

    if (is_rvalue)
        enum_model_->Set(std::move(data_in_third));
    else
        enum_model_->Set(data_in_third);

    auto data_out_third = enum_model_->Get();
    EXPECT_EQ(data_out_third[0], protogeneratorqt::TestEnumEnum::OPTION_2);
    EXPECT_EQ(data_out_third[1], protogeneratorqt::TestEnumEnum::OPTION_1);
    IncAllSignals();
    ApplicationCheckSignals();
}
};  // namespace prototest

// Тесты модели TestEnumModel
using namespace prototest;
//> Одиночные
// @@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@
//+> Конструктор
// vvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvv
TEST_F(TestEnumModelFixture, Constructor)
{
    EXPECT_NO_FATAL_FAILURE(protogeneratorqt::TestEnumModel(data_.get()));
}

//++> SetGetProtoMessage
// Проверка функций SetProtoMessage и GetProtoMessage
// Ожидается отсутствие сигналов
// ----------------------------------------------------------------------------------------------------

TEST_F(TestEnumModelFixture, SetGetProtoMessage)
{
    google::protobuf::RepeatedField<int> data_in_first;
    data_in_first.Add(protogeneratorqt::TestEnumEnum::OPTION_1);
    data_in_first.Add(protogeneratorqt::TestEnumEnum::OPTION_2);
    enum_model_->SetProtoMessage(&data_in_first);

    auto data_first = enum_model_->GetProtoMessage();
    EXPECT_EQ(data_first, &data_in_first);
    IncAllSignals();
    ApplicationCheckSignals();

    google::protobuf::RepeatedField<int> data_in_second;
    data_in_second.Add(protogeneratorqt::TestEnumEnum::OPTION_1);
    data_in_second.Add(protogeneratorqt::TestEnumEnum::OPTION_2);
    enum_model_->SetProtoMessage(&data_in_second);

    auto data_second = enum_model_->GetProtoMessage();
    EXPECT_EQ(data_second, &data_in_second);
    IncAllSignals();
    ApplicationCheckSignals();

    enum_model_->SetProtoMessage(&data_in_second);

    auto data_third = enum_model_->GetProtoMessage();
    EXPECT_EQ(data_third, &data_in_second);
    IncAllSignals();
    ApplicationCheckSignals();
}
//++> SetGetLvalue
// Проверка функций Set с аргументов Lvalue и Get
// Ожидается получение сигналов при различии соответствующих полей аргумента и тестируемого объекта
// ----------------------------------------------------------------------------------------------------

TEST_F(TestEnumModelFixture, SetGetLvalue)
{
    SetGet(false);
}

//++> SetGetRvalue
// Проверка функций Set с аргументов Rvalue и Get
// Ожидается получение сигналов при различии соответствующих полей аргумента и тестируемого объекта
// ----------------------------------------------------------------------------------------------------

TEST_F(TestEnumModelFixture, SetGetRvalue)
{
    SetGet(true);
}

// ++> SyncData
// ----------------------------------------------------------------------------------------------------
TEST_F(TestEnumModelFixture, SyncData)
{
    enum_model_->SyncData();
    IncAllSignals();
    ApplicationCheckSignals();

    enum_model_->SyncData();
    IncAllSignals();
    ApplicationCheckSignals();
}

// ++> rowCount
// ----------------------------------------------------------------------------------------------------
TEST_F(TestEnumModelFixture, rowCount)
{
    google::protobuf::RepeatedField<int> data_in_first;
    data_in_first.Add(protogeneratorqt::TestEnumEnum::OPTION_1);
    data_in_first.Add(protogeneratorqt::TestEnumEnum::OPTION_2);
    enum_model_->Set(data_in_first);

    EXPECT_EQ(enum_model_->rowCount(), 2);
    IncAllSignals();
    ApplicationCheckSignals();

    google::protobuf::RepeatedField<int> data_in_second;
    data_in_second.Add(protogeneratorqt::TestEnumEnum::OPTION_1);
    data_in_second.Add(protogeneratorqt::TestEnumEnum::OPTION_2);
    data_in_second.Add(protogeneratorqt::TestEnumEnum::OPTION_2);
    data_in_second.Add(protogeneratorqt::TestEnumEnum::OPTION_2);
    data_in_second.Add(protogeneratorqt::TestEnumEnum::OPTION_1);
    data_in_second.Add(protogeneratorqt::TestEnumEnum::OPTION_1);
    enum_model_->Set(data_in_second);

    EXPECT_EQ(enum_model_->rowCount(), 6);
    IncAllSignals();
    ApplicationCheckSignals();

    google::protobuf::RepeatedField<int> data_in_third;
    data_in_third.Add(protogeneratorqt::TestEnumEnum::OPTION_2);
    enum_model_->Set(data_in_third);

    EXPECT_EQ(enum_model_->rowCount(), 1);
    IncAllSignals();
    ApplicationCheckSignals();
}

//++> Iterator
// Проверка Iterator
// ----------------------------------------------------------------------------------------------------
TEST_F(TestEnumModelFixture, Iterator)
{
    google::protobuf::RepeatedField<int> data_in_first;
    data_in_first.Add(protogeneratorqt::TestEnumEnum::OPTION_1);
    data_in_first.Add(protogeneratorqt::TestEnumEnum::OPTION_2);
    data_in_first.Add(protogeneratorqt::TestEnumEnum::OPTION_2);
    data_in_first.Add(protogeneratorqt::TestEnumEnum::OPTION_1);
    enum_model_->Set(data_in_first);

    protogeneratorqt::TestEnumModel::iterator it = enum_model_->begin();
    EXPECT_EQ(*it, protogeneratorqt::TestEnumEnum::OPTION_1);
    ++it;
    EXPECT_EQ(*it, protogeneratorqt::TestEnumEnum::OPTION_2);
    ++it;
    EXPECT_EQ(*it, protogeneratorqt::TestEnumEnum::OPTION_2);
    ++it;
    EXPECT_EQ(*it, protogeneratorqt::TestEnumEnum::OPTION_1);
    ++it;
    EXPECT_EQ(it, enum_model_->end());
}

//++> Iterator
// Проверка const Iterator begin/end
// ----------------------------------------------------------------------------------------------------
TEST_F(TestEnumModelFixture, ConstIterator)
{
    google::protobuf::RepeatedField<int> data_in_first;
    data_in_first.Add(protogeneratorqt::TestEnumEnum::OPTION_1);
    data_in_first.Add(protogeneratorqt::TestEnumEnum::OPTION_2);
    data_in_first.Add(protogeneratorqt::TestEnumEnum::OPTION_2);
    data_in_first.Add(protogeneratorqt::TestEnumEnum::OPTION_1);
    enum_model_->Set(data_in_first);

    protogeneratorqt::TestEnumModel::const_iterator it = enum_model_->begin();
    EXPECT_EQ(*it, protogeneratorqt::TestEnumEnum::OPTION_1);
    ++it;
    EXPECT_EQ(*it, protogeneratorqt::TestEnumEnum::OPTION_2);
    ++it;
    EXPECT_EQ(*it, protogeneratorqt::TestEnumEnum::OPTION_2);
    ++it;
    EXPECT_EQ(*it, protogeneratorqt::TestEnumEnum::OPTION_1);
    ++it;
    EXPECT_EQ(it, enum_model_->end());
}

//++> ConstIteratorC
// Проверка const Iterator cbegin/cend
// ----------------------------------------------------------------------------------------------------
TEST_F(TestEnumModelFixture, ConstIteratorC)
{
    google::protobuf::RepeatedField<int> data_in_first;
    data_in_first.Add(protogeneratorqt::TestEnumEnum::OPTION_1);
    data_in_first.Add(protogeneratorqt::TestEnumEnum::OPTION_2);
    data_in_first.Add(protogeneratorqt::TestEnumEnum::OPTION_2);
    data_in_first.Add(protogeneratorqt::TestEnumEnum::OPTION_1);
    enum_model_->Set(data_in_first);

    protogeneratorqt::TestEnumModel::const_iterator it = enum_model_->cbegin();
    EXPECT_EQ(*it, protogeneratorqt::TestEnumEnum::OPTION_1);
    ++it;
    EXPECT_EQ(*it, protogeneratorqt::TestEnumEnum::OPTION_2);
    ++it;
    EXPECT_EQ(*it, protogeneratorqt::TestEnumEnum::OPTION_2);
    ++it;
    EXPECT_EQ(*it, protogeneratorqt::TestEnumEnum::OPTION_1);
    ++it;
    EXPECT_EQ(it, enum_model_->cend());
}

//++> At
// Проверка At
// ----------------------------------------------------------------------------------------------------
TEST_F(TestEnumModelFixture, At)
{
    google::protobuf::RepeatedField<int> data_in_first;
    data_in_first.Add(protogeneratorqt::TestEnumEnum::OPTION_1);
    data_in_first.Add(protogeneratorqt::TestEnumEnum::OPTION_2);
    data_in_first.Add(protogeneratorqt::TestEnumEnum::OPTION_2);
    data_in_first.Add(protogeneratorqt::TestEnumEnum::OPTION_1);
    enum_model_->Set(data_in_first);

    EXPECT_EQ(enum_model_->At(0), protogeneratorqt::TestEnumEnum::OPTION_1);
    EXPECT_EQ(enum_model_->At(1), protogeneratorqt::TestEnumEnum::OPTION_2);
    EXPECT_EQ(enum_model_->At(2), protogeneratorqt::TestEnumEnum::OPTION_2);
    EXPECT_EQ(enum_model_->At(3), protogeneratorqt::TestEnumEnum::OPTION_1);

    IncAllSignals();
    ApplicationCheckSignals();
}

//++> IndexOf
// Проверка IndexOf
// ----------------------------------------------------------------------------------------------------
TEST_F(TestEnumModelFixture, IndexOf)
{
    google::protobuf::RepeatedField<int> data_in_first;
    data_in_first.Add(protogeneratorqt::TestEnumEnum::OPTION_1);
    data_in_first.Add(protogeneratorqt::TestEnumEnum::OPTION_2);
    data_in_first.Add(protogeneratorqt::TestEnumEnum::OPTION_2);
    data_in_first.Add(protogeneratorqt::TestEnumEnum::OPTION_1);
    enum_model_->Set(data_in_first);

    EXPECT_EQ(enum_model_->IndexOf(protogeneratorqt::TestEnumEnum::OPTION_1), 0);
    EXPECT_EQ(enum_model_->IndexOf(protogeneratorqt::TestEnumEnum::OPTION_2), 1);
    // Повторная проверка для того чтобы удостовериться,
    // что при повторном вызове он так же берет первое встретившееся значение
    EXPECT_EQ(enum_model_->IndexOf(protogeneratorqt::TestEnumEnum::OPTION_1), 0);
    EXPECT_EQ(enum_model_->IndexOf(protogeneratorqt::TestEnumEnum::OPTION_2), 1);

    IncAllSignals();
    ApplicationCheckSignals();
}

//++> Append
// Проверка Append
// ----------------------------------------------------------------------------------------------------
TEST_F(TestEnumModelFixture, Append)
{
    enum_model_->Append(protogeneratorqt::TestEnumEnum::OPTION_2);

    EXPECT_EQ(enum_model_->rowCount(), 1);
    EXPECT_EQ(enum_model_->At(0), protogeneratorqt::TestEnumEnum::OPTION_2);

    IncAllSignals(2);
    ApplicationCheckSignals();

    enum_model_->Append(protogeneratorqt::TestEnumEnum::OPTION_1);

    EXPECT_EQ(enum_model_->rowCount(), 2);
    EXPECT_EQ(enum_model_->At(0), protogeneratorqt::TestEnumEnum::OPTION_2);
    EXPECT_EQ(enum_model_->At(1), protogeneratorqt::TestEnumEnum::OPTION_1);

    IncAllSignals(2);
    ApplicationCheckSignals();

    enum_model_->Append(protogeneratorqt::TestEnumEnum::OPTION_1);
    enum_model_->Append(protogeneratorqt::TestEnumEnum::OPTION_2);

    EXPECT_EQ(enum_model_->rowCount(), 4);
    EXPECT_EQ(enum_model_->At(0), protogeneratorqt::TestEnumEnum::OPTION_2);
    EXPECT_EQ(enum_model_->At(1), protogeneratorqt::TestEnumEnum::OPTION_1);
    EXPECT_EQ(enum_model_->At(2), protogeneratorqt::TestEnumEnum::OPTION_1);
    EXPECT_EQ(enum_model_->At(3), protogeneratorqt::TestEnumEnum::OPTION_2);

    IncAllSignals(4);
    ApplicationCheckSignals();
}

//++> Remove
// Проверка Remove
// ----------------------------------------------------------------------------------------------------
TEST_F(TestEnumModelFixture, Remove)
{
    google::protobuf::RepeatedField<int> data_in_first;
    data_in_first.Add(protogeneratorqt::TestEnumEnum::OPTION_1);
    data_in_first.Add(protogeneratorqt::TestEnumEnum::OPTION_2);
    data_in_first.Add(protogeneratorqt::TestEnumEnum::OPTION_2);
    data_in_first.Add(protogeneratorqt::TestEnumEnum::OPTION_1);
    data_in_first.Add(protogeneratorqt::TestEnumEnum::OPTION_1);
    data_in_first.Add(protogeneratorqt::TestEnumEnum::OPTION_2);
    enum_model_->Set(data_in_first);

    EXPECT_EQ(enum_model_->rowCount(), 6);

    IncAllSignals();
    ApplicationCheckSignals();

    enum_model_->Remove(0);
    EXPECT_EQ(enum_model_->rowCount(), 5);
    EXPECT_EQ(enum_model_->At(0), protogeneratorqt::TestEnumEnum::OPTION_2);
    EXPECT_EQ(enum_model_->At(1), protogeneratorqt::TestEnumEnum::OPTION_2);
    EXPECT_EQ(enum_model_->At(2), protogeneratorqt::TestEnumEnum::OPTION_1);
    EXPECT_EQ(enum_model_->At(3), protogeneratorqt::TestEnumEnum::OPTION_1);
    EXPECT_EQ(enum_model_->At(4), protogeneratorqt::TestEnumEnum::OPTION_2);

    IncAllSignals();
    ApplicationCheckSignals();

    enum_model_->Remove(4);
    EXPECT_EQ(enum_model_->rowCount(), 4);
    EXPECT_EQ(enum_model_->At(0), protogeneratorqt::TestEnumEnum::OPTION_2);
    EXPECT_EQ(enum_model_->At(1), protogeneratorqt::TestEnumEnum::OPTION_2);
    EXPECT_EQ(enum_model_->At(2), protogeneratorqt::TestEnumEnum::OPTION_1);
    EXPECT_EQ(enum_model_->At(3), protogeneratorqt::TestEnumEnum::OPTION_1);

    IncAllSignals();
    ApplicationCheckSignals();

    enum_model_->Remove(1, 3);
    EXPECT_EQ(enum_model_->rowCount(), 1);
    EXPECT_EQ(enum_model_->At(0), protogeneratorqt::TestEnumEnum::OPTION_2);

    IncAllSignals();
    ApplicationCheckSignals();

    enum_model_->Remove(0, 1);
    EXPECT_EQ(enum_model_->rowCount(), 0);

    IncAllSignals();
    ApplicationCheckSignals();
}

//++> Clear
// Проверка Clear
// ----------------------------------------------------------------------------------------------------
TEST_F(TestEnumModelFixture, Clear)
{
    google::protobuf::RepeatedField<int> data_in_first;
    data_in_first.Add(protogeneratorqt::TestEnumEnum::OPTION_1);
    data_in_first.Add(protogeneratorqt::TestEnumEnum::OPTION_2);
    data_in_first.Add(protogeneratorqt::TestEnumEnum::OPTION_2);
    data_in_first.Add(protogeneratorqt::TestEnumEnum::OPTION_1);
    data_in_first.Add(protogeneratorqt::TestEnumEnum::OPTION_1);
    data_in_first.Add(protogeneratorqt::TestEnumEnum::OPTION_2);
    enum_model_->Set(data_in_first);

    EXPECT_EQ(enum_model_->rowCount(), 6);

    IncAllSignals();
    ApplicationCheckSignals();

    enum_model_->Clear();
    EXPECT_EQ(enum_model_->rowCount(), 0);

    IncAllSignals();
    ApplicationCheckSignals();
}

//++> roleNames
// Проверка roleNames
// ----------------------------------------------------------------------------------------------------
TEST_F(TestEnumModelFixture, roleNames)
{
    auto role_names_first = enum_model_->roleNames();
    EXPECT_EQ(role_names_first[Qt::UserRole], "item");

    // Проверка лишних сигналов
    ApplicationCheckSignals();

    // Проверка отсутсвия изменений после повторного вызова
    auto role_names_second = enum_model_->roleNames();
    EXPECT_EQ(role_names_second[Qt::UserRole], "item");

    // Проверка лишних сигналов
    ApplicationCheckSignals();
}
