#include <google/protobuf/util/message_differencer.h>

#include "data_test.h"
#include "qt_core_test.h"
#include <test_qt_pb.h>

#include <QDebug>
#include <QGuiApplication>
#include <QSignalSpy>
#include <QTest>
#include <algorithm>
#include <array>
#include <gtest/gtest.h>
#include <iterator>
#include <memory>
#include <set>
#include <string>
#include <tuple>
#include <type_traits>
#include <utility>
#include <vector>

namespace prototest
{
// Набор "стандартных" значений, для тестов

// Сигналы которые мы отслеживаем
enum TestObjectSignals {
    kChanged            = 0,
    kOptionalIpEndpoint = 1,
    kOptionalInteger    = 2,
    kOptionalString     = 3,
    kOptionalEnum       = 4,
    kIntegerField       = 5,
    kStringField        = 6,
    kEnumField          = 7,
    kState              = 8
};

class TestObjectFixture : public QtCoreTest
{
protected:
    void SetUp() override;
    void TearDown() override;
    void SignalsTrackingSetUp() override;
    // Увеличение счетчика всех сигналов, индексы которых переданы
    void IncSignals(const std::set<TestObjectSignals>& signals_set, int count = 1);

    // Проверка совпадения ожидаемого количества сигналов и пойманного
    void CheckSignals() override;

protected:
    inline void PresetDataOne()
    {
        test_->mutable_optional_ip_endpoint()->set_address(kAddress1);
        test_->mutable_optional_ip_endpoint()->set_port(kPort1);
        test_->set_optional_enum(protogeneratorqt::Test_Enum::Test_Enum_OPTION_2);
        protogeneratorqt::IpEndpoint val;
        val.set_address(kAddress2);
        val.set_port(kPort2);
        test_->mutable_repeated_ip_endpoint()->Add(std::move(val));
        test_->mutable_repeated_string()->Add("Hello");
        test_->mutable_repeated_enum()->Add(protogeneratorqt::Test_Enum::Test_Enum_OPTION_2);
        test_->set_string_field("GG WP");
        test_->set_integer_field(511);
        test_->set_state_integer(993);
    }

    void TestProtoMessageEquals(bool is_empty = true)
    {
        if (!is_empty)
        {
            PresetDataOne();
        }
        ASSERT_EQ(test_object_->EqualsTo(*test_.get()), is_empty);
        ApplicationCheckSignals();
    }

    void TestProtoMessageEquivalent(bool is_empty = true)
    {
        if (!is_empty)
        {
            PresetDataOne();
        }
        ASSERT_EQ(test_object_->EquivalentTo(*test_.get()), is_empty);
        ApplicationCheckSignals();
    }

    void TestProtoMessageApproximatelyEquals(bool is_empty = true)
    {
        if (!is_empty)
        {
            PresetDataOne();
        }
        ASSERT_EQ(test_object_->ApproximatelyEqualsTo(*test_.get()), is_empty);
        ApplicationCheckSignals();
    }

    void TestProtoMessageApproximatelyEquivalent(bool is_empty = true)
    {
        if (!is_empty)
        {
            PresetDataOne();
        }
        ASSERT_EQ(test_object_->ApproximatelyEquivalentTo(*test_.get()), is_empty);
        ApplicationCheckSignals();
    }

    template <typename REPEATED, typename VALUES_CONTAINER, typename SIGNALS_PREDICT>
    void TestSetGetRepeatedIpEndpoint(REPEATED&& value_in, VALUES_CONTAINER values_container, SIGNALS_PREDICT signals_predict)
    {
        protogeneratorqt::IpEndpoint ip_endpoint;
        for (auto it = values_container.begin(); it != values_container.end(); ++it)
        {
            ip_endpoint.set_address((*it).first);
            ip_endpoint.set_port((*it).second);
            value_in.Add(std::move(ip_endpoint));
        }

        test_object_->SetRepeatedIpEndpoint(std::forward<REPEATED>(value_in));

        auto val_out = test_object_->GetRepeatedIpEndpoint();
        EXPECT_EQ(val_out->rowCount(), values_container.size());
        for (auto i = 0; i < values_container.size(); ++i)
        {
            EXPECT_EQ(val_out->At(i)->GetAddress(), QString::fromStdString(values_container[i].first));
            EXPECT_EQ(val_out->At(i)->GetPort(), values_container[i].second);
        }
        IncSignals(signals_predict);
        ApplicationCheckSignals();
    }

    template <typename REPEATED, typename VALUES_CONTAINER, typename SIGNALS_PREDICT>
    void TestSetGetRepeatedString(REPEATED&& value_in, VALUES_CONTAINER values_container, SIGNALS_PREDICT signals_predict)
    {
        for (auto it = values_container.begin(); it != values_container.end(); ++it)
        {
            value_in.Add(*it);
        }

        test_object_->SetRepeatedString(std::forward<REPEATED>(value_in));

        auto val_out = test_object_->GetRepeatedString();
        EXPECT_EQ(val_out->rowCount(), values_container.size());
        for (auto i = 0; i < values_container.size(); ++i)
        {
            EXPECT_EQ(val_out->At(i), values_container[i]);
        }
        IncSignals(signals_predict);
        ApplicationCheckSignals();
    }

    template <typename T>
    void TestSetGetRepeatedEnum(T&& value_in, const std::vector<int>& values_container, const std::set<TestObjectSignals>& signals_expected)
    {
        for (auto it = values_container.begin(); it != values_container.end(); ++it)
        {
            value_in.Add(*it);
        }

        test_object_->SetRepeatedEnum(std::forward<T>(value_in));

        auto val_out = test_object_->GetRepeatedEnum();
        EXPECT_EQ(val_out->rowCount(), values_container.size());
        for (auto i = 0; i < values_container.size(); ++i)
        {
            EXPECT_EQ(val_out->At(i), values_container[i]);
        }
        IncSignals(signals_expected);
        ApplicationCheckSignals();
    }

protected:
    // test_object_ - объект, класса который тестируется
    std::unique_ptr<protogeneratorqt::TestObject> test_object_;

    // test_ - protogeneratorqt::Test является ProtoMessage для protogeneratorqt::TestObject
    std::unique_ptr<protogeneratorqt::Test> test_;
};

void TestObjectFixture::SetUp()
{
    AppSetUp();
    test_        = std::make_unique<protogeneratorqt::Test>();
    test_object_ = std::make_unique<protogeneratorqt::TestObject>();
    SignalsTrackingSetUp();
}

void TestObjectFixture::TearDown()
{
    AppTearDown();
    test_.reset();
    test_object_.reset();
}

void TestObjectFixture::SignalsTrackingSetUp()
{
    signals_counts_ = std::vector<int>(9, 0);
    signals_spies_.push_back(std::make_unique<QSignalSpy>(test_object_.get(), &protogeneratorqt::TestObject::changed));
    signals_spies_.push_back(std::make_unique<QSignalSpy>(test_object_.get(), &protogeneratorqt::TestObject::optionalIpEndpointChanged));
    signals_spies_.push_back(std::make_unique<QSignalSpy>(test_object_.get(), &protogeneratorqt::TestObject::optionalIntegerChanged));
    signals_spies_.push_back(std::make_unique<QSignalSpy>(test_object_.get(), &protogeneratorqt::TestObject::optionalStringChanged));
    signals_spies_.push_back(std::make_unique<QSignalSpy>(test_object_.get(), &protogeneratorqt::TestObject::optionalEnumChanged));
    signals_spies_.push_back(std::make_unique<QSignalSpy>(test_object_.get(), &protogeneratorqt::TestObject::integerFieldChanged));
    signals_spies_.push_back(std::make_unique<QSignalSpy>(test_object_.get(), &protogeneratorqt::TestObject::stringFieldChanged));
    signals_spies_.push_back(std::make_unique<QSignalSpy>(test_object_.get(), &protogeneratorqt::TestObject::enumFieldChanged));
    signals_spies_.push_back(std::make_unique<QSignalSpy>(test_object_.get(), &protogeneratorqt::TestObject::stateChanged));
}

void TestObjectFixture::CheckSignals()
{
    EXPECT_EQ(signals_spies_[TestObjectSignals::kChanged]->count(), signals_counts_[TestObjectSignals::kChanged]);
    EXPECT_EQ(signals_spies_[TestObjectSignals::kOptionalIpEndpoint]->count(), signals_counts_[TestObjectSignals::kOptionalIpEndpoint]);
    EXPECT_EQ(signals_spies_[TestObjectSignals::kOptionalInteger]->count(), signals_counts_[TestObjectSignals::kOptionalInteger]);
    EXPECT_EQ(signals_spies_[TestObjectSignals::kOptionalString]->count(), signals_counts_[TestObjectSignals::kOptionalString]);
    EXPECT_EQ(signals_spies_[TestObjectSignals::kOptionalEnum]->count(), signals_counts_[TestObjectSignals::kOptionalEnum]);
    EXPECT_EQ(signals_spies_[TestObjectSignals::kIntegerField]->count(), signals_counts_[TestObjectSignals::kIntegerField]);
    EXPECT_EQ(signals_spies_[TestObjectSignals::kStringField]->count(), signals_counts_[TestObjectSignals::kStringField]);
    EXPECT_EQ(signals_spies_[TestObjectSignals::kEnumField]->count(), signals_counts_[TestObjectSignals::kEnumField]);
    EXPECT_EQ(signals_spies_[TestObjectSignals::kState]->count(), signals_counts_[TestObjectSignals::kState]);
}

void TestObjectFixture::IncSignals(const std::set<TestObjectSignals>& signals_set, int count /*= 1*/)
{
    for (auto i : signals_set) signals_counts_[i] += count;
}
};  // namespace protogeneratorqt


using namespace prototest;

//> Одиночные
// @@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@
//+> Конструктор
// vvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvv

//++> Constructors
// ----------------------------------------------------------------------------------------------------
TEST_F(TestObjectFixture, Constructors)
{
    // Проверка конструкторов
    std::unique_ptr<protogeneratorqt::Test> message_b = std::make_unique<protogeneratorqt::Test>();
    protogeneratorqt::Test message_c;
    protogeneratorqt::Test message_d;


    EXPECT_NO_FATAL_FAILURE(protogeneratorqt::TestObject());
    EXPECT_NO_FATAL_FAILURE(protogeneratorqt::TestObject(message_b.get()));
    EXPECT_NO_FATAL_FAILURE(protogeneratorqt::TestObject(message_c));
    EXPECT_NO_FATAL_FAILURE(protogeneratorqt::TestObject(std::move(message_d)));
}

//+> Публичные функции
// vvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvv

//++> SyncProtoMessageMultipleOne
// ----------------------------------------------------------------------------------------------------

TEST_F(TestObjectFixture, SyncProtoMessageMultipleOne)
{
    // TODO: Make it
    // Получение proto message, внешнее изменение нескольких полей и использование функции синхронизации, проверка сигналов и значений
    auto message = test_object_->GetProtoMessage();

    message->mutable_optional_ip_endpoint()->set_address(kAddress1);
    message->mutable_optional_ip_endpoint()->set_port(kPort1);
    message->set_optional_enum(protogeneratorqt::Test_Enum_OPTION_1);
    message->set_integer_field(99);
    message->set_enum_field(protogeneratorqt::Test_Enum_OPTION_2);

    test_object_->SyncProtoMessage();

    protogeneratorqt::IpEndpointObject*  optional_ip_endpoint = test_object_->GetOptionalIpEndpoint();
    protogeneratorqt::TestEnumEnum::Enum optional_enum        = test_object_->GetOptionalEnum();
    int                                  i                    = test_object_->GetIntegerField();
    protogeneratorqt::TestEnumEnum::Enum e                    = test_object_->GetEnumField();
    EXPECT_EQ(optional_ip_endpoint->GetAddress(), kAddress1);
    EXPECT_EQ(optional_ip_endpoint->GetPort(), kPort1);
    EXPECT_EQ(optional_enum, protogeneratorqt::Test_Enum_OPTION_1);
    EXPECT_EQ(i, 99);
    EXPECT_EQ(e, protogeneratorqt::Test_Enum_OPTION_2);
    IncAllSignals();
    ApplicationCheckSignals();
}

//++> SyncProtoMessageMultipleTwo
// ----------------------------------------------------------------------------------------------------

TEST_F(TestObjectFixture, SyncProtoMessageMultipleTwo)
{
    // TODO: Make it
    // Получение proto message, внешнее изменение нескольких полей и использование функции синхронизации, проверка сигналов и значений
    auto message = test_object_->GetProtoMessage();

    message->mutable_optional_ip_endpoint()->set_address(kAddress1);
    message->mutable_optional_ip_endpoint()->set_port(kPort1);
    message->set_optional_enum(protogeneratorqt::Test_Enum_OPTION_1);
    message->set_integer_field(99);
    message->set_enum_field(protogeneratorqt::Test_Enum_OPTION_2);

    test_object_->SyncProtoMessage();

    protogeneratorqt::IpEndpointObject*  optional_ip_endpoint = test_object_->GetOptionalIpEndpoint();
    protogeneratorqt::TestEnumEnum::Enum optional_enum        = test_object_->GetOptionalEnum();
    int                                  i                    = test_object_->GetIntegerField();
    protogeneratorqt::TestEnumEnum::Enum e                    = test_object_->GetEnumField();
    EXPECT_EQ(optional_ip_endpoint->GetAddress(), kAddress1);
    EXPECT_EQ(optional_ip_endpoint->GetPort(), kPort1);
    EXPECT_EQ(optional_enum, protogeneratorqt::Test_Enum_OPTION_1);
    EXPECT_EQ(i, 99);
    EXPECT_EQ(e, protogeneratorqt::Test_Enum_OPTION_2);
    IncAllSignals();
    ApplicationCheckSignals();
}

//+> CheckForChangedAndSetProtoMessage
// vvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvv

//++> CheckForChangedAndSetProtoMessageStateMulti
// ----------------------------------------------------------------------------------------------------

TEST_F(TestObjectFixture, CheckForChangedAndSetProtoMessageStateMulti)
{
    // Многократная установка State в proto message, проверка GetStateCase(), выставленных значений и сигналов
    ASSERT_EQ(test_object_->GetStateCase(), protogeneratorqt::TestObject::STATE_NOT_SET);
    ApplicationCheckSignals();

    // state i
    std::unique_ptr<protogeneratorqt::Test> val_i = std::make_unique<protogeneratorqt::Test>();
    val_i->set_state_integer(993);  // outside set value

    test_object_->CheckForChangedAndSetProtoMessage(val_i.release());

    ASSERT_EQ(test_object_->GetStateCase(), protogeneratorqt::TestObject::STATE_INTEGER);
    IncSignals({ TestObjectSignals::kChanged, TestObjectSignals::kState });
    ApplicationCheckSignals();

    // state s
    std::unique_ptr<protogeneratorqt::Test> val_s = std::make_unique<protogeneratorqt::Test>();
    val_s->set_state_string("Hello");  // outside set value

    test_object_->CheckForChangedAndSetProtoMessage(val_s.release());

    ASSERT_EQ(test_object_->GetStateCase(), protogeneratorqt::TestObject::STATE_STRING);
    IncSignals({ TestObjectSignals::kChanged, TestObjectSignals::kState });
    ApplicationCheckSignals();

    // state e
    std::unique_ptr<protogeneratorqt::Test> val_e = std::make_unique<protogeneratorqt::Test>();
    val_e->set_state_enum(protogeneratorqt::Test_Enum::Test_Enum_OPTION_2);  // outside set value

    test_object_->CheckForChangedAndSetProtoMessage(val_e.release());

    ASSERT_EQ(test_object_->GetStateCase(), protogeneratorqt::TestObject::STATE_ENUM);
    IncSignals({ TestObjectSignals::kChanged, TestObjectSignals::kState });
    ApplicationCheckSignals();

    // state o
    std::unique_ptr<protogeneratorqt::Test> val_o = std::make_unique<protogeneratorqt::Test>();
    val_o->mutable_state_ip_endpoint()->set_address(kAddress1);
    val_o->mutable_state_ip_endpoint()->set_port(kPort1);

    test_object_->CheckForChangedAndSetProtoMessage(val_o.release());

    ASSERT_EQ(test_object_->GetStateCase(), protogeneratorqt::TestObject::STATE_IP_ENDPOINT);
    IncSignals({ TestObjectSignals::kChanged, TestObjectSignals::kState });
    ApplicationCheckSignals();
}

//++> CheckForChangedAndSetProtoMessageMultipleOne
// ----------------------------------------------------------------------------------------------------

TEST_F(TestObjectFixture, CheckForChangedAndSetProtoMessageMultipleOne)
{
    // Получение proto message, внешнее изменение нескольких полей и использование функции синхронизации, проверка сигналов и значений
    test_->mutable_optional_ip_endpoint()->set_address(kAddress1);
    test_->mutable_optional_ip_endpoint()->set_port(kPort1);
    test_->set_optional_enum(protogeneratorqt::Test_Enum::Test_Enum_OPTION_2);
    protogeneratorqt::IpEndpoint val;
    val.set_address(kAddress2);
    val.set_port(kPort2);
    test_->mutable_repeated_ip_endpoint()->Add(std::move(val));
    test_->mutable_repeated_string()->Add("Hello");
    test_->mutable_repeated_enum()->Add(protogeneratorqt::Test_Enum::Test_Enum_OPTION_2);
    test_->set_string_field("GG WP");
    test_->set_integer_field(511);
    test_->set_state_integer(993);

    test_object_->CheckForChangedAndSetProtoMessage(test_.get());

    test_object_->EqualsTo(*test_.get());
    IncSignals({ TestObjectSignals::kChanged, TestObjectSignals::kOptionalIpEndpoint, TestObjectSignals::kOptionalEnum, TestObjectSignals::kStringField,
        TestObjectSignals::kIntegerField, TestObjectSignals::kState });
    ApplicationCheckSignals();
}

//++> CheckForChangedAndSetProtoMessageMultipleTwo
// ----------------------------------------------------------------------------------------------------

TEST_F(TestObjectFixture, CheckForChangedAndSetProtoMessageMultipleTwo)
{
    // Получение proto message, внешнее изменение нескольких полей и использование функции синхронизации, проверка сигналов и значений
    test_->set_optional_integer(511);
    test_->set_optional_string("Hello");
    protogeneratorqt::IpEndpoint val;
    val.set_address(kAddress1);
    val.set_port(kPort1);
    test_->mutable_repeated_ip_endpoint()->Add(std::move(val));
    test_->mutable_repeated_string()->Add("Hello");
    test_->mutable_repeated_enum()->Add(protogeneratorqt::Test_Enum::Test_Enum_OPTION_2);
    test_->mutable_ip_endpoint_field()->set_address(kAddress2);
    test_->mutable_ip_endpoint_field()->set_port(kPort2);
    test_->set_enum_field(protogeneratorqt::Test_Enum::Test_Enum_OPTION_2);
    test_->set_state_string("QWERTY");

    test_object_->CheckForChangedAndSetProtoMessage(test_.get());

    test_object_->EqualsTo(*test_.get());
    IncSignals({ TestObjectSignals::kChanged, TestObjectSignals::kOptionalInteger, TestObjectSignals::kOptionalString, TestObjectSignals::kEnumField, TestObjectSignals::kState });
    ApplicationCheckSignals();
}

//+> Public equals
// vvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvv

//++> EqualsToEmpty
// ----------------------------------------------------------------------------------------------------

TEST_F(TestObjectFixture, EqualsToEmpty)
{
    // Равенство пустому protomessage
    TestProtoMessageEquals();
}

//++> EqualsToNonEmpty
// ----------------------------------------------------------------------------------------------------

TEST_F(TestObjectFixture, EqualsToNonEmpty)
{
    // Равенство не пустому protomessage
    TestProtoMessageEquals(false);
}

//++> EquivalentToEmpty
// ----------------------------------------------------------------------------------------------------

TEST_F(TestObjectFixture, EquivalentToEmpty)
{
    TestProtoMessageEquivalent();
}

//++> EquivalentToNonEmpty
// ----------------------------------------------------------------------------------------------------

TEST_F(TestObjectFixture, EquivalentToNonEmpty)
{
    TestProtoMessageEquivalent(false);
}

//++> ApproximatelyEqualsToEmpty
// ----------------------------------------------------------------------------------------------------

TEST_F(TestObjectFixture, ApproximatelyEqualsToEmpty)
{
    TestProtoMessageApproximatelyEquals();
}

//++> ApproximatelyEqualsToNonEmpty
// ----------------------------------------------------------------------------------------------------

TEST_F(TestObjectFixture, ApproximatelyEqualsToNonEmpty)
{
    TestProtoMessageApproximatelyEquals(false);
}

//++> ApproximatelyEquivalentToEmpty
// ----------------------------------------------------------------------------------------------------

TEST_F(TestObjectFixture, ApproximatelyEquivalentToEmpty)
{
    TestProtoMessageApproximatelyEquivalent();
}

//++> ApproximatelyEquivalentToNonEmpty
// ----------------------------------------------------------------------------------------------------

TEST_F(TestObjectFixture, ApproximatelyEquivalentToNonEmpty)
{
    TestProtoMessageApproximatelyEquivalent(false);
}

//++> SetGetProtoMessage
// ----------------------------------------------------------------------------------------------------

TEST_F(TestObjectFixture, SetGetProtoMessage)
{
    // Проверка установки передаваемого protomessage
    test_object_->SetProtoMessage(test_.get());

    auto message = test_object_->GetProtoMessage();
    EXPECT_EQ(message, test_.get());
    ApplicationCheckSignals();

    // Повторная проверка установки передаваемого protomessage
    protogeneratorqt::Test test_local;

    test_object_->SetProtoMessage(&test_local);

    message = test_object_->GetProtoMessage();
    EXPECT_EQ(message, &test_local);
    ApplicationCheckSignals();
}

//++> SetGetLvalue
// ----------------------------------------------------------------------------------------------------

TEST_F(TestObjectFixture, SetGetLvalue)
{
    // Установка значений по lvalue ссылке
    int                                     optional_integer   = 25;
    const char                              optional_string[]  = "set optional s";
    protogeneratorqt::Test_Enum             optional_enum      = protogeneratorqt::Test_Enum_OPTION_2;
    int                                     i                  = 30;
    const char                              s[]                = "set s";
    protogeneratorqt::Test_Enum             e                  = protogeneratorqt::Test_Enum_OPTION_2;
    protogeneratorqt::TestObject::StateCase state        = protogeneratorqt::TestObject::STATE_NOT_SET;
    test_->mutable_optional_ip_endpoint()->set_address(kAddress1);
    test_->mutable_optional_ip_endpoint()->set_port(kPort1);
    test_->set_optional_integer(optional_integer);
    test_->set_optional_string(optional_string);
    test_->set_optional_enum(optional_enum);
    test_->set_integer_field(i);
    test_->set_string_field(s);
    test_->mutable_ip_endpoint_field()->set_address(kAddress2);
    test_->mutable_ip_endpoint_field()->set_port(kPort2);
    test_->set_enum_field(e);

    test_object_->Set(*test_.get());
    protogeneratorqt::Test value = test_object_->Get();
    EXPECT_EQ(value.optional_ip_endpoint().address(), kAddress1);
    EXPECT_EQ(value.optional_ip_endpoint().port(), kPort1);
    EXPECT_EQ(value.optional_integer(), optional_integer);
    EXPECT_EQ(value.optional_string(), optional_string);
    EXPECT_EQ(value.optional_enum(), optional_enum);
    EXPECT_EQ(value.integer_field(), i);
    EXPECT_EQ(value.string_field(), s);
    EXPECT_EQ(value.ip_endpoint_field().address(), kAddress2);
    EXPECT_EQ(value.ip_endpoint_field().port(), kPort2);
    EXPECT_EQ(value.enum_field(), e);
    EXPECT_EQ(value.state_case(), protogeneratorqt::TestObject::STATE_NOT_SET);
    IncSignals({ TestObjectSignals::kChanged, TestObjectSignals::kOptionalEnum, TestObjectSignals::kOptionalInteger, TestObjectSignals::kOptionalIpEndpoint, TestObjectSignals::kOptionalString, TestObjectSignals::kIntegerField, TestObjectSignals::kStringField, TestObjectSignals::kEnumField });
    ApplicationCheckSignals();


    // Повторная установка значений по lvalue ссылке
    test_->set_optional_integer(1928);
    test_->set_enum_field(protogeneratorqt::Test_Enum::Test_Enum_OPTION_1);
    test_->set_string_field("ameno");

    test_object_->Set(*test_.get());

    auto value_second = test_object_->Get();
    EXPECT_EQ(value_second.optional_integer(), 1928);
    EXPECT_EQ(value_second.enum_field(), protogeneratorqt::Test_Enum::Test_Enum_OPTION_1);
    EXPECT_EQ(value_second.string_field(), "ameno");
    IncSignals({ TestObjectSignals::kChanged, TestObjectSignals::kOptionalInteger, TestObjectSignals::kEnumField, TestObjectSignals::kStringField });
    ApplicationCheckSignals();
}

//++> SetGetRvalue
// ----------------------------------------------------------------------------------------------------

TEST_F(TestObjectFixture, SetGetRvalue)
{
    // Установка значений по rvalue ссылке
    test_->mutable_optional_ip_endpoint()->set_address(kAddress1);
    test_->mutable_optional_ip_endpoint()->set_port(kPort1);
    test_->set_optional_integer(25);
    test_->set_optional_string("set optional s");
    test_->set_optional_enum(protogeneratorqt::Test_Enum::Test_Enum_OPTION_1);
    test_->set_integer_field(30);
    test_->set_string_field("set s");
    test_->mutable_ip_endpoint_field()->set_address(kAddress2);
    test_->mutable_ip_endpoint_field()->set_port(kPort2);
    test_->set_enum_field(protogeneratorqt::Test_Enum::Test_Enum_OPTION_2);

    test_object_->Set(std::move(*test_.get()));

    protogeneratorqt::Test value = test_object_->Get();
    EXPECT_EQ(value.optional_ip_endpoint().address(), kAddress1);
    EXPECT_EQ(value.optional_ip_endpoint().port(), kPort1);
    EXPECT_EQ(value.optional_integer(), 25);
    EXPECT_EQ(value.optional_string(), "set optional s");
    EXPECT_EQ(value.optional_enum(), protogeneratorqt::Test_Enum::Test_Enum_OPTION_1);
    EXPECT_EQ(value.integer_field(), 30);
    EXPECT_EQ(value.string_field(), "set s");
    EXPECT_EQ(value.ip_endpoint_field().address(), kAddress2);
    EXPECT_EQ(value.ip_endpoint_field().port(), kPort2);
    EXPECT_EQ(value.enum_field(), protogeneratorqt::Test_Enum::Test_Enum_OPTION_2);
    EXPECT_EQ(value.state_case(), protogeneratorqt::TestObject::STATE_NOT_SET);
    IncSignals({ TestObjectSignals::kChanged, TestObjectSignals::kOptionalEnum, TestObjectSignals::kOptionalInteger, TestObjectSignals::kOptionalIpEndpoint, TestObjectSignals::kOptionalString, TestObjectSignals::kIntegerField, TestObjectSignals::kStringField, TestObjectSignals::kEnumField });
    ApplicationCheckSignals();

    // Повторная установка значений по rvalue ссылке
    test_->mutable_optional_ip_endpoint()->set_address(kAddress1);
    test_->mutable_optional_ip_endpoint()->set_port(kPort1);
    test_->set_optional_integer(1928);
    test_->set_optional_string("set optional s");
    test_->set_optional_enum(protogeneratorqt::Test_Enum::Test_Enum_OPTION_1);
    test_->mutable_ip_endpoint_field()->set_address(kAddress2);
    test_->mutable_ip_endpoint_field()->set_port(kPort2);
    test_->set_enum_field(protogeneratorqt::Test_Enum::Test_Enum_OPTION_1);
    test_->set_integer_field(30);
    test_->set_string_field("ameno");

    test_object_->Set(*test_.get());

    auto value_second = test_object_->Get();
    EXPECT_EQ(value_second.optional_integer(), 1928);
    EXPECT_EQ(value_second.enum_field(), protogeneratorqt::Test_Enum::Test_Enum_OPTION_1);
    EXPECT_EQ(value_second.string_field(), "ameno");
    IncSignals({ TestObjectSignals::kChanged, TestObjectSignals::kOptionalInteger, TestObjectSignals::kEnumField, TestObjectSignals::kStringField });
    ApplicationCheckSignals();
}

//++> RevokeProtoMessageOwnership
// ----------------------------------------------------------------------------------------------------

TEST_F(TestObjectFixture, RevokeProtoMessageOwnership)
{
    auto message = test_object_->GetProtoMessage();
    EXPECT_NO_FATAL_FAILURE(test_object_->RevokeProtoMessageOwnership());
    test_object_.reset();
    EXPECT_EQ(message->integer_field(), 0);
}

//++> SetGetClearStateCase
// ----------------------------------------------------------------------------------------------------

TEST_F(TestObjectFixture, SetGetClearStateCase)
{
    // Проверка установки и сброса State вместе с Clear
    // Проверяем начальный State
    EXPECT_EQ(test_object_->GetStateCase(), protogeneratorqt::TestObject::STATE_NOT_SET);

    // Очищаем State
    test_object_->ClearStateCase();

    // Проверка того, что State сбросился
    EXPECT_EQ(test_object_->GetStateCase(), protogeneratorqt::TestObject::STATE_NOT_SET);
    ApplicationCheckSignals();

    // Установка State
    test_object_->SetStateCase(protogeneratorqt::TestObject::STATE_INTEGER);

    // Проверка изменения State и сигналов
    EXPECT_EQ(test_object_->GetStateCase(), protogeneratorqt::TestObject::STATE_INTEGER);
    IncSignals({ TestObjectSignals::kChanged, TestObjectSignals::kState });
    ApplicationCheckSignals();

    // Очищаем State
    test_object_->ClearStateCase();

    // Проверка того, что State сбросился
    EXPECT_EQ(test_object_->GetStateCase(), protogeneratorqt::TestObject::STATE_NOT_SET);
    IncSignals({ TestObjectSignals::kChanged, TestObjectSignals::kState });
    ApplicationCheckSignals();

    // Установка State
    test_object_->SetStateCase(protogeneratorqt::TestObject::STATE_STRING);

    // Проверка изменения State и сигналов
    EXPECT_EQ(test_object_->GetStateCase(), protogeneratorqt::TestObject::STATE_STRING);
    IncSignals({ TestObjectSignals::kChanged, TestObjectSignals::kState });
    ApplicationCheckSignals();

    // Очищаем State
    test_object_->ClearStateCase();

    // Проверка того, что State сбросился
    EXPECT_EQ(test_object_->GetStateCase(), protogeneratorqt::TestObject::STATE_NOT_SET);
    IncSignals({ TestObjectSignals::kChanged, TestObjectSignals::kState });
    ApplicationCheckSignals();

    // Установка State
    test_object_->SetStateCase(protogeneratorqt::TestObject::STATE_IP_ENDPOINT);

    // Проверка изменения State и сигналов
    EXPECT_EQ(test_object_->GetStateCase(), protogeneratorqt::TestObject::STATE_IP_ENDPOINT);
    IncSignals({ TestObjectSignals::kChanged, TestObjectSignals::kState });
    ApplicationCheckSignals();

    // Очищаем State
    test_object_->ClearStateCase();

    // Проверка того, что State сбросился
    EXPECT_EQ(test_object_->GetStateCase(), protogeneratorqt::TestObject::STATE_NOT_SET);
    IncSignals({ TestObjectSignals::kChanged, TestObjectSignals::kState });
    ApplicationCheckSignals();

    // Установка State
    test_object_->SetStateCase(protogeneratorqt::TestObject::STATE_ENUM);

    // Проверка изменения State и сигналов
    EXPECT_EQ(test_object_->GetStateCase(), protogeneratorqt::TestObject::STATE_ENUM);
    IncSignals({ TestObjectSignals::kChanged, TestObjectSignals::kState });
    ApplicationCheckSignals();

    // Очищаем State
    test_object_->ClearStateCase();

    // Проверка того, что State сбросился
    EXPECT_EQ(test_object_->GetStateCase(), protogeneratorqt::TestObject::STATE_NOT_SET);
    IncSignals({ TestObjectSignals::kChanged, TestObjectSignals::kState });
    ApplicationCheckSignals();
}

//++> SetGetStateCaseNonClear
// ----------------------------------------------------------------------------------------------------

TEST_F(TestObjectFixture, SetGetStateCaseNonClear)
{
    // Проверка установки разных State без сброса
    // Установка State
    test_object_->SetStateCase(protogeneratorqt::TestObject::STATE_INTEGER);

    // Проверка изменения State и сигналов
    EXPECT_EQ(test_object_->GetStateCase(), protogeneratorqt::TestObject::STATE_INTEGER);
    IncSignals({ TestObjectSignals::kChanged, TestObjectSignals::kState });
    ApplicationCheckSignals();

    // Установка State
    test_object_->SetStateCase(protogeneratorqt::TestObject::STATE_STRING);

    // Проверка изменения State и сигналов
    EXPECT_EQ(test_object_->GetStateCase(), protogeneratorqt::TestObject::STATE_STRING);
    IncSignals({ TestObjectSignals::kChanged, TestObjectSignals::kState });
    ApplicationCheckSignals();

    // Установка State
    test_object_->SetStateCase(protogeneratorqt::TestObject::STATE_IP_ENDPOINT);

    // Проверка изменения State и сигналов
    EXPECT_EQ(test_object_->GetStateCase(), protogeneratorqt::TestObject::STATE_IP_ENDPOINT);
    IncSignals({ TestObjectSignals::kChanged, TestObjectSignals::kState });
    ApplicationCheckSignals();

    // Установка State
    test_object_->SetStateCase(protogeneratorqt::TestObject::STATE_ENUM);

    // Проверка изменения State и сигналов
    EXPECT_EQ(test_object_->GetStateCase(), protogeneratorqt::TestObject::STATE_ENUM);
    IncSignals({ TestObjectSignals::kChanged, TestObjectSignals::kState });
    ApplicationCheckSignals();

    // Повторная установка StateI для проверки состояния после StateE
    test_object_->SetStateCase(protogeneratorqt::TestObject::STATE_INTEGER);

    // Проверка изменения State и сигналов
    EXPECT_EQ(test_object_->GetStateCase(), protogeneratorqt::TestObject::STATE_INTEGER);
    IncSignals({ TestObjectSignals::kChanged, TestObjectSignals::kState });
    ApplicationCheckSignals();
}

//++> SetGetStateCaseManualClear
// ----------------------------------------------------------------------------------------------------

TEST_F(TestObjectFixture, SetGetStateCaseManualClear)
{
    // Проверка установки разных State вместе со STATE_NOT_SET
    // Установка State
    test_object_->SetStateCase(protogeneratorqt::TestObject::STATE_INTEGER);

    // Проверка изменения State и сигналов
    EXPECT_EQ(test_object_->GetStateCase(), protogeneratorqt::TestObject::STATE_INTEGER);
    IncSignals({ TestObjectSignals::kChanged, TestObjectSignals::kState });
    ApplicationCheckSignals();

    // Установка State
    test_object_->SetStateCase(protogeneratorqt::TestObject::STATE_STRING);

    // Проверка изменения State и сигналов
    EXPECT_EQ(test_object_->GetStateCase(), protogeneratorqt::TestObject::STATE_STRING);
    IncSignals({ TestObjectSignals::kChanged, TestObjectSignals::kState });
    ApplicationCheckSignals();

    // Установка State
    test_object_->SetStateCase(protogeneratorqt::TestObject::STATE_IP_ENDPOINT);

    // Проверка изменения State и сигналов
    EXPECT_EQ(test_object_->GetStateCase(), protogeneratorqt::TestObject::STATE_IP_ENDPOINT);
    IncSignals({ TestObjectSignals::kChanged, TestObjectSignals::kState });
    ApplicationCheckSignals();

    // Установка State
    test_object_->SetStateCase(protogeneratorqt::TestObject::STATE_ENUM);

    // Проверка изменения State и сигналов
    EXPECT_EQ(test_object_->GetStateCase(), protogeneratorqt::TestObject::STATE_ENUM);
    IncSignals({ TestObjectSignals::kChanged, TestObjectSignals::kState });
    ApplicationCheckSignals();

    // Установка State
    test_object_->SetStateCase(protogeneratorqt::TestObject::STATE_NOT_SET);

    // Проверка изменения State и сигналов
    EXPECT_EQ(test_object_->GetStateCase(), protogeneratorqt::TestObject::STATE_NOT_SET);
    IncSignals({ TestObjectSignals::kChanged, TestObjectSignals::kState });
    ApplicationCheckSignals();

    // Повторная установка StateI для проверки состояния после последнего State
    test_object_->SetStateCase(protogeneratorqt::TestObject::STATE_INTEGER);

    // Проверка изменения State и сигналов
    EXPECT_EQ(test_object_->GetStateCase(), protogeneratorqt::TestObject::STATE_INTEGER);
    IncSignals({ TestObjectSignals::kChanged, TestObjectSignals::kState });
    ApplicationCheckSignals();
}

//++> SetHasOptionalIpEndpoint
// ----------------------------------------------------------------------------------------------------

TEST_F(TestObjectFixture, SetHasOptionalIpEndpoint)
{

    ASSERT_FALSE(test_object_->HasOptionalIpEndpoint());

    test_object_->SetHasOptionalIpEndpoint(false);

    ASSERT_FALSE(test_object_->HasOptionalIpEndpoint());

    test_object_->SetHasOptionalIpEndpoint(true);

    ASSERT_TRUE(test_object_->HasOptionalIpEndpoint());
    EXPECT_EQ(test_object_->GetOptionalIpEndpoint()->GetAddress(), "");
    EXPECT_EQ(test_object_->GetOptionalIpEndpoint()->GetPort(), 0);
    IncSignals({ TestObjectSignals::kChanged, TestObjectSignals::kOptionalIpEndpoint });
    ApplicationCheckSignals();

    test_object_->SetHasOptionalIpEndpoint(false);

    ASSERT_FALSE(test_object_->HasOptionalIpEndpoint());

    IncSignals({ TestObjectSignals::kChanged, TestObjectSignals::kOptionalIpEndpoint });
    ApplicationCheckSignals();

    test_object_->SetHasOptionalIpEndpoint(false);

    ASSERT_FALSE(test_object_->HasOptionalIpEndpoint());

    test_object_->SetHasOptionalIpEndpoint(true);

    ASSERT_TRUE(test_object_->HasOptionalIpEndpoint());
    EXPECT_EQ(test_object_->GetOptionalIpEndpoint()->GetAddress(), "");
    EXPECT_EQ(test_object_->GetOptionalIpEndpoint()->GetPort(), 0);
    IncSignals({ TestObjectSignals::kChanged, TestObjectSignals::kOptionalIpEndpoint });
    ApplicationCheckSignals();

    test_object_->SetHasOptionalIpEndpoint(true);

    ASSERT_TRUE(test_object_->HasOptionalIpEndpoint());
    EXPECT_EQ(test_object_->GetOptionalIpEndpoint()->GetAddress(), "");
    EXPECT_EQ(test_object_->GetOptionalIpEndpoint()->GetPort(), 0);

    test_object_->SetHasOptionalIpEndpoint(false);

    ASSERT_FALSE(test_object_->HasOptionalIpEndpoint());

    IncSignals({ TestObjectSignals::kChanged, TestObjectSignals::kOptionalIpEndpoint });
    ApplicationCheckSignals();

    test_object_->SetHasOptionalIpEndpoint(true);

    ASSERT_TRUE(test_object_->HasOptionalIpEndpoint());
    IncSignals({ TestObjectSignals::kChanged, TestObjectSignals::kOptionalIpEndpoint });
    ApplicationCheckSignals();

    test_object_->SetHasOptionalIpEndpoint(false);

    ASSERT_FALSE(test_object_->HasOptionalIpEndpoint());

    IncSignals({ TestObjectSignals::kChanged, TestObjectSignals::kOptionalIpEndpoint });
    ApplicationCheckSignals();
}

//++> SetGetLvalueOptionalIpEndpoint
// ----------------------------------------------------------------------------------------------------

TEST_F(TestObjectFixture, SetGetLvalueOptionalIpEndpoint)
{
    protogeneratorqt::IpEndpoint optional_o_in;
    optional_o_in.set_address(kAddress1);
    optional_o_in.set_port(kPort1);

    ASSERT_FALSE(test_object_->HasOptionalIpEndpoint());

    test_object_->SetOptionalIpEndpoint(optional_o_in);

    ASSERT_TRUE(test_object_->HasOptionalIpEndpoint());

    protogeneratorqt::IpEndpointObject* optional_o_out = test_object_->GetOptionalIpEndpoint();

    EXPECT_EQ(optional_o_out->GetAddress(), kAddress1);
    EXPECT_EQ(optional_o_out->GetPort(), kPort1);
    IncSignals({ TestObjectSignals::kChanged, TestObjectSignals::kOptionalIpEndpoint });
    ApplicationCheckSignals();

    QSignalSpy optional_o_changed = QSignalSpy(optional_o_out, &protogeneratorqt::IpEndpointObject::changed);
    QSignalSpy optional_o_address = QSignalSpy(optional_o_out, &protogeneratorqt::IpEndpointObject::addressChanged);
    QSignalSpy optional_o_port    = QSignalSpy(optional_o_out, &protogeneratorqt::IpEndpointObject::portChanged);

    protogeneratorqt::IpEndpoint optional_o_in_second;
    optional_o_in_second.set_address(kAddress2);
    optional_o_in_second.set_port(kPort2);

    test_object_->SetOptionalIpEndpoint(optional_o_in_second);

    ASSERT_TRUE(test_object_->HasOptionalIpEndpoint());

    protogeneratorqt::IpEndpointObject* optional_o_out_second = test_object_->GetOptionalIpEndpoint();
    ProcessEvents(3);

    EXPECT_EQ(optional_o_out_second->GetAddress(), kAddress2);
    EXPECT_EQ(optional_o_out_second->GetPort(), kPort2);
    EXPECT_EQ(optional_o_changed.count(), 1);
    EXPECT_EQ(optional_o_address.count(), 1);
    EXPECT_EQ(optional_o_port.count(), 1);
    IncSignals({ TestObjectSignals::kChanged });
    ApplicationCheckSignals();

    QSignalSpy optional_o_changed_no = QSignalSpy(optional_o_out_second, &protogeneratorqt::IpEndpointObject::changed);
    QSignalSpy optional_o_address_no = QSignalSpy(optional_o_out_second, &protogeneratorqt::IpEndpointObject::addressChanged);
    QSignalSpy optional_o_port_no    = QSignalSpy(optional_o_out_second, &protogeneratorqt::IpEndpointObject::portChanged);

    protogeneratorqt::IpEndpoint optional_o_in_third;
    optional_o_in_third.set_address(kAddress2);
    optional_o_in_third.set_port(kPort2);

    test_object_->SetOptionalIpEndpoint(std::move(optional_o_in_third));
    ProcessEvents(3);

    ASSERT_TRUE(test_object_->HasOptionalIpEndpoint());
    protogeneratorqt::IpEndpointObject* optional_o_out_third = test_object_->GetOptionalIpEndpoint();
    EXPECT_EQ(optional_o_out_third->GetAddress(), kAddress2);
    EXPECT_EQ(optional_o_out_third->GetPort(), kPort2);
    EXPECT_EQ(optional_o_changed_no.count(), 0);
    EXPECT_EQ(optional_o_address_no.count(), 0);
    EXPECT_EQ(optional_o_port_no.count(), 0);
    ApplicationCheckSignals();
}

//++> SetGetOptionalIpEndpointRvalue
// ----------------------------------------------------------------------------------------------------

TEST_F(TestObjectFixture, SetGetOptionalIpEndpointRvalue)
{
    protogeneratorqt::IpEndpoint optional_o_in;
    optional_o_in.set_address(kAddress1);
    optional_o_in.set_port(kPort1);

    ASSERT_FALSE(test_object_->HasOptionalIpEndpoint());

    test_object_->SetOptionalIpEndpoint(std::move(optional_o_in));

    ASSERT_TRUE(test_object_->HasOptionalIpEndpoint());
    protogeneratorqt::IpEndpointObject* optional_o_out = test_object_->GetOptionalIpEndpoint();
    EXPECT_EQ(optional_o_out->GetAddress(), kAddress1);
    EXPECT_EQ(optional_o_out->GetPort(), kPort1);
    IncSignals({ TestObjectSignals::kChanged, TestObjectSignals::kOptionalIpEndpoint });
    ApplicationCheckSignals();

    QSignalSpy optional_o_changed = QSignalSpy(optional_o_out, &protogeneratorqt::IpEndpointObject::changed);
    QSignalSpy optional_o_address = QSignalSpy(optional_o_out, &protogeneratorqt::IpEndpointObject::addressChanged);
    QSignalSpy optional_o_port    = QSignalSpy(optional_o_out, &protogeneratorqt::IpEndpointObject::portChanged);

    protogeneratorqt::IpEndpoint optional_o_in_second;
    optional_o_in_second.set_address(kAddress2);
    optional_o_in_second.set_port(kPort2);

    test_object_->SetOptionalIpEndpoint(std::move(optional_o_in_second));
    ProcessEvents(3);

    ASSERT_TRUE(test_object_->HasOptionalIpEndpoint());
    protogeneratorqt::IpEndpointObject* optional_o_out_second = test_object_->GetOptionalIpEndpoint();
    EXPECT_EQ(optional_o_out_second->GetAddress(), kAddress2);
    EXPECT_EQ(optional_o_out_second->GetPort(), kPort2);
    EXPECT_EQ(optional_o_changed.count(), 1);
    EXPECT_EQ(optional_o_address.count(), 1);
    EXPECT_EQ(optional_o_port.count(), 1);
    IncSignals({ TestObjectSignals::kChanged });
    ApplicationCheckSignals();

    QSignalSpy optional_o_changed_no = QSignalSpy(optional_o_out_second, &protogeneratorqt::IpEndpointObject::changed);
    QSignalSpy optional_o_address_no = QSignalSpy(optional_o_out_second, &protogeneratorqt::IpEndpointObject::addressChanged);
    QSignalSpy optional_o_port_no    = QSignalSpy(optional_o_out_second, &protogeneratorqt::IpEndpointObject::portChanged);

    protogeneratorqt::IpEndpoint optional_o_in_third;
    optional_o_in_third.set_address(kAddress2);
    optional_o_in_third.set_port(kPort2);

    test_object_->SetOptionalIpEndpoint(std::move(optional_o_in_third));
    ProcessEvents(3);

    ASSERT_TRUE(test_object_->HasOptionalIpEndpoint());
    protogeneratorqt::IpEndpointObject* optional_o_out_third = test_object_->GetOptionalIpEndpoint();
    EXPECT_EQ(optional_o_out_third->GetAddress(), kAddress2);
    EXPECT_EQ(optional_o_out_third->GetPort(), kPort2);
    EXPECT_EQ(optional_o_changed_no.count(), 0);
    EXPECT_EQ(optional_o_address_no.count(), 0);
    EXPECT_EQ(optional_o_port_no.count(), 0);
    ApplicationCheckSignals();
}

//++> SetHasOptionalInteger
// ----------------------------------------------------------------------------------------------------

TEST_F(TestObjectFixture, SetHasOptionalInteger)
{
    ASSERT_FALSE(test_object_->HasOptionalInteger());

    test_object_->SetHasOptionalInteger(false);

    ASSERT_FALSE(test_object_->HasOptionalInteger());

    test_object_->SetHasOptionalInteger(true);

    ASSERT_TRUE(test_object_->HasOptionalInteger());
    EXPECT_EQ(test_object_->GetOptionalInteger(), 0);
    IncSignals({ TestObjectSignals::kChanged, TestObjectSignals::kOptionalInteger });
    ApplicationCheckSignals();

    test_object_->SetHasOptionalInteger(false);

    ASSERT_FALSE(test_object_->HasOptionalInteger());
    IncSignals({ TestObjectSignals::kChanged, TestObjectSignals::kOptionalInteger });
    ApplicationCheckSignals();

    test_object_->SetHasOptionalInteger(false);

    ASSERT_FALSE(test_object_->HasOptionalInteger());

    test_object_->SetHasOptionalInteger(true);

    ASSERT_TRUE(test_object_->HasOptionalInteger());
    EXPECT_EQ(test_object_->GetOptionalInteger(), 0);
    IncSignals({ TestObjectSignals::kChanged, TestObjectSignals::kOptionalInteger });
    ApplicationCheckSignals();

    test_object_->SetHasOptionalInteger(true);

    ASSERT_TRUE(test_object_->HasOptionalInteger());
    EXPECT_EQ(test_object_->GetOptionalInteger(), 0);

    test_object_->SetHasOptionalInteger(false);

    ASSERT_FALSE(test_object_->HasOptionalInteger());
    IncSignals({ TestObjectSignals::kChanged, TestObjectSignals::kOptionalInteger });
    ApplicationCheckSignals();
}

//++> SetGetOptionalInteger
// ----------------------------------------------------------------------------------------------------

TEST_F(TestObjectFixture, SetGetOptionalInteger)
{
    int optional_i_in{ 25 };

    ASSERT_FALSE(test_object_->HasOptionalInteger());

    test_object_->SetOptionalInteger(optional_i_in);

    ASSERT_TRUE(test_object_->HasOptionalInteger());
    EXPECT_EQ(test_object_->GetOptionalInteger(), optional_i_in);
    IncSignals({ TestObjectSignals::kChanged, TestObjectSignals::kOptionalInteger });
    ApplicationCheckSignals();

    int optional_i_in_second{ 448 };

    test_object_->SetOptionalInteger(optional_i_in_second);

    ASSERT_TRUE(test_object_->HasOptionalInteger());
    EXPECT_EQ(test_object_->GetOptionalInteger(), optional_i_in_second);
    IncSignals({ TestObjectSignals::kChanged, TestObjectSignals::kOptionalInteger });
    ApplicationCheckSignals();

    int optional_i_in_third{ 448 };

    test_object_->SetOptionalInteger(optional_i_in_third);

    ASSERT_TRUE(test_object_->HasOptionalInteger());
    EXPECT_EQ(test_object_->GetOptionalInteger(), optional_i_in_third);
    ApplicationCheckSignals();
}

//++> SetHasOptionalString
// ----------------------------------------------------------------------------------------------------

TEST_F(TestObjectFixture, SetHasOptionalString)
{
    ASSERT_FALSE(test_object_->HasOptionalString());

    test_object_->SetHasOptionalString(false);

    ASSERT_FALSE(test_object_->HasOptionalString());

    test_object_->SetHasOptionalString(true);

    ASSERT_TRUE(test_object_->HasOptionalString());
    EXPECT_EQ(test_object_->GetOptionalString(), "");
    IncSignals({ TestObjectSignals::kChanged, TestObjectSignals::kOptionalString });
    ApplicationCheckSignals();

    test_object_->SetHasOptionalString(false);

    ASSERT_FALSE(test_object_->HasOptionalString());
    IncSignals({ TestObjectSignals::kChanged, TestObjectSignals::kOptionalString });
    ApplicationCheckSignals();

    test_object_->SetHasOptionalString(false);

    ASSERT_FALSE(test_object_->HasOptionalString());

    test_object_->SetHasOptionalString(true);

    ASSERT_TRUE(test_object_->HasOptionalString());
    EXPECT_EQ(test_object_->GetOptionalString(), "");
    IncSignals({ TestObjectSignals::kChanged, TestObjectSignals::kOptionalString });
    ApplicationCheckSignals();

    test_object_->SetHasOptionalString(true);

    ASSERT_TRUE(test_object_->HasOptionalString());
    EXPECT_EQ(test_object_->GetOptionalString(), "");

    test_object_->SetHasOptionalString(false);

    ASSERT_FALSE(test_object_->HasOptionalString());
    IncSignals({ TestObjectSignals::kChanged, TestObjectSignals::kOptionalString });
    ApplicationCheckSignals();
}

//++> SetGetOptionalString
// ----------------------------------------------------------------------------------------------------

TEST_F(TestObjectFixture, SetGetOptionalString)
{
    QString optional_s_in("optional s");

    ASSERT_FALSE(test_object_->HasOptionalString());

    test_object_->SetOptionalString(optional_s_in);

    ASSERT_TRUE(test_object_->HasOptionalString());
    EXPECT_EQ(test_object_->GetOptionalString(), optional_s_in);
    IncSignals({ TestObjectSignals::kChanged, TestObjectSignals::kOptionalString });
    ApplicationCheckSignals();

    QString optional_s_in_second("second");

    test_object_->SetOptionalString(optional_s_in_second);

    ASSERT_TRUE(test_object_->HasOptionalString());
    EXPECT_EQ(test_object_->GetOptionalString(), optional_s_in_second);
    IncSignals({ TestObjectSignals::kChanged, TestObjectSignals::kOptionalString });
    ApplicationCheckSignals();

    QString optional_s_in_third("second");

    test_object_->SetOptionalString(optional_s_in_third);

    ASSERT_TRUE(test_object_->HasOptionalString());
    EXPECT_EQ(test_object_->GetOptionalString(), optional_s_in_third);
    ApplicationCheckSignals();
}

//++> SetHasOptionalEnum
// ----------------------------------------------------------------------------------------------------

TEST_F(TestObjectFixture, SetHasOptionalEnum)
{
    ASSERT_FALSE(test_object_->HasOptionalEnum());

    test_object_->SetHasOptionalEnum(false);

    ASSERT_FALSE(test_object_->HasOptionalEnum());

    test_object_->SetHasOptionalEnum(true);

    ASSERT_TRUE(test_object_->HasOptionalEnum());
    IncSignals({ TestObjectSignals::kChanged, TestObjectSignals::kOptionalEnum });
    ApplicationCheckSignals();

    test_object_->SetHasOptionalEnum(false);

    ASSERT_FALSE(test_object_->HasOptionalEnum());

    IncSignals({ TestObjectSignals::kChanged, TestObjectSignals::kOptionalEnum });
    ApplicationCheckSignals();

    test_object_->SetHasOptionalEnum(false);

    ASSERT_FALSE(test_object_->HasOptionalEnum());

    test_object_->SetHasOptionalEnum(true);

    ASSERT_TRUE(test_object_->HasOptionalEnum());
    IncSignals({ TestObjectSignals::kChanged, TestObjectSignals::kOptionalEnum });
    ApplicationCheckSignals();

    test_object_->SetHasOptionalEnum(true);

    ASSERT_TRUE(test_object_->HasOptionalEnum());

    test_object_->SetHasOptionalEnum(false);

    ASSERT_FALSE(test_object_->HasOptionalEnum());

    IncSignals({ TestObjectSignals::kChanged, TestObjectSignals::kOptionalEnum });
    ApplicationCheckSignals();
}

//++> SetGetOptionalEnum
// ----------------------------------------------------------------------------------------------------

TEST_F(TestObjectFixture, SetGetOptionalEnum)
{
    protogeneratorqt::TestEnumEnum::Enum optional_e_in = protogeneratorqt::TestEnumEnum::Enum::OPTION_2;

    ASSERT_FALSE(test_object_->HasOptionalEnum());

    test_object_->SetOptionalEnum(optional_e_in);

    ASSERT_TRUE(test_object_->HasOptionalEnum());
    EXPECT_EQ(test_object_->GetOptionalEnum(), optional_e_in);
    IncSignals({ TestObjectSignals::kChanged, TestObjectSignals::kOptionalEnum });
    ApplicationCheckSignals();

    protogeneratorqt::TestEnumEnum::Enum optional_e_in_second = protogeneratorqt::TestEnumEnum::Enum::OPTION_1;

    ASSERT_TRUE(test_object_->HasOptionalEnum());

    test_object_->SetOptionalEnum(optional_e_in_second);
    ProcessEvents(3);

    ASSERT_TRUE(test_object_->HasOptionalEnum());
    EXPECT_EQ(test_object_->GetOptionalEnum(), optional_e_in_second);
    IncSignals({ TestObjectSignals::kChanged, TestObjectSignals::kOptionalEnum });
    ApplicationCheckSignals();

    protogeneratorqt::TestEnumEnum::Enum optional_e_in_third = protogeneratorqt::TestEnumEnum::Enum::OPTION_1;

    ASSERT_TRUE(test_object_->HasOptionalEnum());

    test_object_->SetOptionalEnum(optional_e_in_third);
    ProcessEvents(3);

    ASSERT_TRUE(test_object_->HasOptionalEnum());
    EXPECT_EQ(test_object_->GetOptionalEnum(), optional_e_in_third);
    ApplicationCheckSignals();
}

//++> SetGetRepeatedIpEndpointLvalueByTemplate
// ----------------------------------------------------------------------------------------------------

TEST_F(TestObjectFixture, SetGetRepeatedIpEndpointLvalueByTemplate)
{
    google::protobuf::RepeatedPtrField<protogeneratorqt::IpEndpoint> value_in;
    std::vector<std::pair<const std::string, int>>                   values_container{ { kAddress1, kPort1 }, {kAddress2, kPort2} };
    const std::set<TestObjectSignals>                                          signals_predict = { TestObjectSignals::kChanged };
    TestSetGetRepeatedIpEndpoint(value_in, values_container, signals_predict);
}

//++> SetGetRepeatedIpEndpointRvalueByTemplate
// ----------------------------------------------------------------------------------------------------

TEST_F(TestObjectFixture, SetGetRepeatedIpEndpointRvalueByTemplate)
{
    google::protobuf::RepeatedPtrField<protogeneratorqt::IpEndpoint> value_in;
    std::vector<std::pair<const std::string, int>>                   values_container{ { kAddress1, kPort1 }, { kAddress2, kPort2 } };
    const std::set<TestObjectSignals>                                          signals_predict = { TestObjectSignals::kChanged };
    TestSetGetRepeatedIpEndpoint(std::move(value_in), values_container, signals_predict);
}

//++> SetGetRepeatedStringLvalueByTemplate
// ----------------------------------------------------------------------------------------------------

TEST_F(TestObjectFixture, SetGetRepeatedStringLvalueByTemplate)
{
    google::protobuf::RepeatedPtrField<std::string> value_in;
    std::vector<const char*>                        values_container = { "GG WP", "Hello" };
    const std::set<TestObjectSignals>                         signals_predict  = {};
    TestSetGetRepeatedString(value_in, values_container, signals_predict);
}

//++> SetGetRepeatedStringRvalueByTemplate
// ----------------------------------------------------------------------------------------------------

TEST_F(TestObjectFixture, SetGetRepeatedStringRvalueByTemplate)
{
    google::protobuf::RepeatedPtrField<std::string> value_in;
    std::vector<const char*>                        values_container = { "GG WP", "Hello" };
    const std::set<TestObjectSignals>                         signals_predict  = {};
    TestSetGetRepeatedString(std::move(value_in), values_container, signals_predict);
}

//++> SetGetRepeatedEnumLvalueByTemplate
// ----------------------------------------------------------------------------------------------------

TEST_F(TestObjectFixture, SetGetRepeatedEnumLvalueByTemplate)
{
    google::protobuf::RepeatedField<int> value_in;
    std::vector<int>                     values_container = {};
    const std::set<TestObjectSignals>              signals_expected = {};
    TestSetGetRepeatedEnum(std::move(value_in), values_container, signals_expected);
}

//++> SetGetRepeatedEnumRvalueByTemplate
// ----------------------------------------------------------------------------------------------------

TEST_F(TestObjectFixture, SetGetRepeatedEnumRvalueByTemplate)
{
    google::protobuf::RepeatedField<int> value_in;
    std::vector<int>                     values_container = { 27, 511 };
    const std::set<TestObjectSignals>              signals_expected  = {};
    TestSetGetRepeatedEnum(std::move(value_in), values_container, signals_expected);
}

//++> SetGetInteger
// ----------------------------------------------------------------------------------------------------

TEST_F(TestObjectFixture, SetGetInteger)
{
    int value_in{ 44 };

    test_object_->SetIntegerField(value_in);

    int value_out = test_object_->GetIntegerField();
    EXPECT_EQ(value_out, value_in);
    IncSignals({ TestObjectSignals::kChanged, TestObjectSignals::kIntegerField });
    ApplicationCheckSignals();

    int value_in_second{ 9123 };

    test_object_->SetIntegerField(value_in_second);

    int value_out_second = test_object_->GetIntegerField();
    EXPECT_EQ(value_out_second, value_in_second);
    IncSignals({ TestObjectSignals::kChanged, TestObjectSignals::kIntegerField });
    ApplicationCheckSignals();

    int value_in_third{ 9123 };

    test_object_->SetIntegerField(value_in_third);

    int value_out_third = test_object_->GetIntegerField();
    EXPECT_EQ(value_out_third, value_in_third);
    ApplicationCheckSignals();
}

//++> SetGetString
// ----------------------------------------------------------------------------------------------------

TEST_F(TestObjectFixture, SetGetString)
{
    QString value_in("set string");

    test_object_->SetStringField(value_in);

    QString value_out = test_object_->GetStringField();
    EXPECT_EQ(value_out, value_in);
    IncSignals({ TestObjectSignals::kChanged, TestObjectSignals::kStringField });
    ApplicationCheckSignals();

    QString value_in_second("second");

    test_object_->SetStringField(value_in_second);

    QString value_out_second = test_object_->GetStringField();
    EXPECT_EQ(value_out_second, value_in_second);
    IncSignals({ TestObjectSignals::kChanged, TestObjectSignals::kStringField });
    ApplicationCheckSignals();

    QString value_in_third("second");

    test_object_->SetStringField(value_in_third);

    QString value_out_third = test_object_->GetStringField();
    EXPECT_EQ(value_out_third, value_in_third);
    ApplicationCheckSignals();
}

//++> SetGetIpEndpointLvalue
// ----------------------------------------------------------------------------------------------------

TEST_F(TestObjectFixture, SetGetIpEndpointLvalue)
{
    protogeneratorqt::IpEndpoint value_in;
    value_in.set_address(kAddress1);
    value_in.set_port(kPort1);

    test_object_->SetIpEndpointField(value_in);

    protogeneratorqt::IpEndpointObject* value_out = test_object_->GetIpEndpointField();
    EXPECT_EQ(value_out->GetAddress(), kAddress1);
    EXPECT_EQ(value_out->GetPort(), kPort1);
    ApplicationCheckSignals();

    protogeneratorqt::IpEndpoint value_in_second;
    value_in_second.set_address(kAddress2);
    value_in_second.set_port(kPort2);

    test_object_->SetIpEndpointField(value_in_second);

    protogeneratorqt::IpEndpointObject* value_out_second = test_object_->GetIpEndpointField();
    EXPECT_EQ(value_out_second->GetAddress(), kAddress2);
    EXPECT_EQ(value_out_second->GetPort(), kPort2);
    IncSignals({ TestObjectSignals::kChanged });
    ApplicationCheckSignals();
}

//++> SetGetIpEndpointRvalue
// ----------------------------------------------------------------------------------------------------

TEST_F(TestObjectFixture, SetGetIpEndpointRvalue)
{
    protogeneratorqt::IpEndpoint value_in;
    value_in.set_address(kAddress1);
    value_in.set_port(kPort1);

    test_object_->SetIpEndpointField(std::move(value_in));

    protogeneratorqt::IpEndpointObject* value_out = test_object_->GetIpEndpointField();
    EXPECT_EQ(value_out->GetAddress(), kAddress1);
    EXPECT_EQ(value_out->GetPort(), kPort1);
    ApplicationCheckSignals();

    protogeneratorqt::IpEndpoint value_in_second;
    value_in_second.set_address(kAddress2);
    value_in_second.set_port(kPort2);

    test_object_->SetIpEndpointField(std::move(value_in_second));

    protogeneratorqt::IpEndpointObject* value_out_second = test_object_->GetIpEndpointField();
    EXPECT_EQ(value_out_second->GetAddress(), kAddress2);
    EXPECT_EQ(value_out_second->GetPort(), kPort2);
    IncSignals({ TestObjectSignals::kChanged });
    ApplicationCheckSignals();
}

//++> SetGetEnum
// ----------------------------------------------------------------------------------------------------

TEST_F(TestObjectFixture, SetGetEnum)
{
    protogeneratorqt::TestEnumEnum::Enum value_in{ protogeneratorqt::TestEnumEnum::Enum::OPTION_2 };

    test_object_->SetEnumField(value_in);

    protogeneratorqt::TestEnumEnum::Enum value_out = test_object_->GetEnumField();
    EXPECT_EQ(value_out, value_in);
    IncSignals({ TestObjectSignals::kChanged, TestObjectSignals::kEnumField });
    ApplicationCheckSignals();

    protogeneratorqt::TestEnumEnum::Enum value_in_second{ protogeneratorqt::TestEnumEnum::Enum::OPTION_1 };

    test_object_->SetEnumField(value_in_second);

    protogeneratorqt::TestEnumEnum::Enum value_out_second = test_object_->GetEnumField();
    EXPECT_EQ(value_out_second, value_in_second);
    IncSignals({ TestObjectSignals::kChanged, TestObjectSignals::kEnumField });
    ApplicationCheckSignals();

    protogeneratorqt::TestEnumEnum::Enum value_in_third{ protogeneratorqt::TestEnumEnum::Enum::OPTION_1 };

    test_object_->SetEnumField(value_in_third);

    protogeneratorqt::TestEnumEnum::Enum value_out_third = test_object_->GetEnumField();
    EXPECT_EQ(value_out_third, value_in_third);
    ApplicationCheckSignals();
}

//+> Public slots
// vvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvv

//++> CopyFromEmpty
// ----------------------------------------------------------------------------------------------------

TEST_F(TestObjectFixture, CopyFromEmpty)
{
    std::unique_ptr<protogeneratorqt::TestObject> val = std::make_unique<protogeneratorqt::TestObject>();

    test_object_->CopyFrom(val.get());

    EXPECT_TRUE(test_object_->EqualsTo(val.get()));
    ApplicationCheckSignals();

    std::unique_ptr<protogeneratorqt::TestObject> val_second = std::make_unique<protogeneratorqt::TestObject>();

    test_object_->CopyFrom(val_second.get());

    EXPECT_TRUE(test_object_->EqualsTo(val_second.get()));
    ApplicationCheckSignals();
}

//++> CopyFromNonEmpty
// ----------------------------------------------------------------------------------------------------

TEST_F(TestObjectFixture, CopyFromNonEmpty)
{
    std::unique_ptr<protogeneratorqt::TestObject> val = std::make_unique<protogeneratorqt::TestObject>();
    val->SetStateInteger(993);
    protogeneratorqt::IpEndpoint optional_ip_endpoint;
    optional_ip_endpoint.set_address(kAddress1);
    optional_ip_endpoint.set_port(kPort1);
    val->SetOptionalIpEndpoint(optional_ip_endpoint);
    val->SetOptionalInteger(511);
    val->SetOptionalString("Hello");
    val->SetOptionalEnum(protogeneratorqt::TestEnumEnum::OPTION_2);
    std::unique_ptr<protogeneratorqt::IpEndpoint> val_o = std::make_unique<protogeneratorqt::IpEndpoint>();
    val_o->set_address(kAddress2);
    val_o->set_port(kPort2);
    protogeneratorqt::IpEndpoint r_o_0;
    r_o_0.set_address("255.255.255.255");
    r_o_0.set_port(5101);
    google::protobuf::RepeatedPtrField<protogeneratorqt::IpEndpoint> r_o;
    r_o.Add(std::move(r_o_0));
    val->GetRepeatedIpEndpoint()->Set(r_o);
    val->GetRepeatedString()->Append("GG WP");
    val->GetRepeatedString()->Append("GH gl qq.");
    val->GetRepeatedEnum()->Append(protogeneratorqt::TestEnumEnum::OPTION_2);
    val->GetRepeatedEnum()->Append(protogeneratorqt::TestEnumEnum::OPTION_1);
    val->SetIntegerField(1861);
    val->SetStringField("Lorem ipsum");
    protogeneratorqt::IpEndpoint ip_endpoint_field;
    ip_endpoint_field.set_address("0.0.0.0");
    ip_endpoint_field.set_port(443);
    val->SetIpEndpointField(std::move(ip_endpoint_field));
    val->SetEnumField(protogeneratorqt::TestEnumEnum::OPTION_2);

    test_object_->CopyFrom(val.get());

    EXPECT_TRUE(test_object_->EqualsTo(val.get()));
    IncAllSignals();
    ApplicationCheckSignals();

    val->SetStateEnum(protogeneratorqt::TestEnumEnum::OPTION_1);
    val->GetRepeatedString()->Append("Wi-fi");
    val->GetRepeatedString()->Append("Dry");
    val->SetOptionalInteger(525);
    val->SetOptionalEnum(protogeneratorqt::TestEnumEnum::OPTION_1);
    val->SetStringField("Memento mori");
    protogeneratorqt::IpEndpoint ip_endpoint_second;
    ip_endpoint_second.set_address("100.1.2.3");
    ip_endpoint_second.set_port(999);
    val->SetIpEndpointField(std::move(ip_endpoint_second));

    test_object_->CopyFrom(val.get());

    EXPECT_TRUE(test_object_->EqualsTo(val.get()));
    IncSignals({ TestObjectSignals::kChanged, TestObjectSignals::kOptionalInteger, TestObjectSignals::kOptionalEnum, TestObjectSignals::kStringField, TestObjectSignals::kState });
    ApplicationCheckSignals();
}

//++> MergeFromEmpty
// ----------------------------------------------------------------------------------------------------

TEST_F(TestObjectFixture, MergeFromEmpty)
{
    // TODO: by compare functions
    std::unique_ptr<protogeneratorqt::TestObject> val = std::make_unique<protogeneratorqt::TestObject>();

    test_object_->MergeFrom(val.get());
    ProcessEvents(4);

    EXPECT_TRUE(test_object_->EquivalentTo(val.get()));
    ApplicationCheckSignals();

    std::unique_ptr<protogeneratorqt::TestObject> val_second = std::make_unique<protogeneratorqt::TestObject>();
    test_object_->MergeFrom(val_second.get());

    EXPECT_TRUE(test_object_->EquivalentTo(val_second.get()));
    ApplicationCheckSignals();
}

//++> MergeFromNonEmpty
// ----------------------------------------------------------------------------------------------------

TEST_F(TestObjectFixture, MergeFromNonEmpty)
{
    // TODO: by compare functions
    std::unique_ptr<protogeneratorqt::TestObject> val = std::make_unique<protogeneratorqt::TestObject>();
    val->SetStateInteger(993);
    protogeneratorqt::IpEndpoint optional_ip_endpoint;
    optional_ip_endpoint.set_address(kAddress1);
    optional_ip_endpoint.set_port(kPort1);
    val->SetOptionalIpEndpoint(optional_ip_endpoint);
    val->SetOptionalInteger(511);
    val->SetOptionalString("Hello");
    val->SetOptionalEnum(protogeneratorqt::TestEnumEnum::OPTION_2);
    std::unique_ptr<protogeneratorqt::IpEndpoint> val_o = std::make_unique<protogeneratorqt::IpEndpoint>();
    val_o->set_address(kAddress2);
    val_o->set_port(kPort2);
    protogeneratorqt::IpEndpoint r_o_0;
    r_o_0.set_address("255.255.255.255");
    r_o_0.set_port(5101);
    google::protobuf::RepeatedPtrField<protogeneratorqt::IpEndpoint> r_o;
    r_o.Add(std::move(r_o_0));
    val->GetRepeatedIpEndpoint()->Set(r_o);
    val->GetRepeatedString()->Append("GG WP");
    val->GetRepeatedString()->Append("GH gl qq.");
    val->GetRepeatedEnum()->Append(protogeneratorqt::TestEnumEnum::OPTION_2);
    val->GetRepeatedEnum()->Append(protogeneratorqt::TestEnumEnum::OPTION_1);
    val->SetIntegerField(1861);
    val->SetStringField("Lorem ipsum");
    protogeneratorqt::IpEndpoint ip_endpoint_field;
    ip_endpoint_field.set_address("0.0.0.0");
    ip_endpoint_field.set_port(443);
    val->SetIpEndpointField(std::move(ip_endpoint_field));
    val->SetEnumField(protogeneratorqt::TestEnumEnum::OPTION_2);

    test_object_->MergeFrom(val.get());

    EXPECT_TRUE(test_object_->ApproximatelyEqualsTo(val.get()));
    IncAllSignals();
    ApplicationCheckSignals();

    std::unique_ptr<protogeneratorqt::TestObject> val_second = std::make_unique<protogeneratorqt::TestObject>();

    val_second->SetStateEnum(protogeneratorqt::TestEnumEnum::OPTION_1);
    val_second->GetRepeatedString()->Append("Wi-fi");
    val_second->GetRepeatedString()->Append("Dry");
    val_second->SetOptionalInteger(525);
    val_second->SetOptionalEnum(protogeneratorqt::TestEnumEnum::OPTION_1);
    val_second->SetStringField("Memento mori");
    protogeneratorqt::IpEndpoint ip_endpoint_second;
    ip_endpoint_second.set_address("100.1.2.3");
    ip_endpoint_second.set_port(999);
    val_second->SetIpEndpointField(std::move(ip_endpoint_second));

    test_object_->MergeFrom(val_second.get());

    EXPECT_EQ(test_object_->GetStateCase(), protogeneratorqt::TestObject::StateCase::STATE_ENUM);
    EXPECT_EQ(test_object_->GetStateEnum(), protogeneratorqt::TestEnumEnum::OPTION_1);
    EXPECT_EQ(test_object_->GetRepeatedString()->rowCount(), 4);
    auto temp = test_object_->GetRepeatedString();
    EXPECT_EQ(test_object_->GetRepeatedString()->At(2), QString("Wi-fi"));
    EXPECT_EQ(test_object_->GetRepeatedString()->At(3), QString("Dry"));
    EXPECT_TRUE(test_object_->HasOptionalInteger());
    EXPECT_EQ(test_object_->GetOptionalInteger(), 525);
    EXPECT_TRUE(test_object_->HasOptionalEnum());
    EXPECT_EQ(test_object_->GetOptionalEnum(), protogeneratorqt::TestEnumEnum::OPTION_1);
    EXPECT_EQ(test_object_->GetStringField(), "Memento mori");
    EXPECT_EQ(test_object_->GetIpEndpointField()->GetAddress(), "100.1.2.3");
    EXPECT_EQ(test_object_->GetIpEndpointField()->GetPort(), 999);
    IncSignals({ TestObjectSignals::kOptionalString, TestObjectSignals::kOptionalIpEndpoint, TestObjectSignals::kOptionalInteger, TestObjectSignals::kOptionalEnum,
        TestObjectSignals::kStringField, TestObjectSignals::kIntegerField, TestObjectSignals::kEnumField,
        TestObjectSignals::kState });
    IncSignals({ TestObjectSignals::kChanged });
    ApplicationCheckSignals();
}

//++> ParseSerializeEmpty
// ----------------------------------------------------------------------------------------------------

TEST_F(TestObjectFixture, ParseSerializeEmpty)
{
    QByteArray      serialized_object = test_object_->Serialize();
    protogeneratorqt::TestObject test_object;
    bool            parsing_result = test_object.Parse(serialized_object);

    EXPECT_TRUE(parsing_result);
    EXPECT_TRUE(test_object_->EqualsTo(&test_object));
    ApplicationCheckSignals();
}

//++> ParseSerializeNonEmpty
// ----------------------------------------------------------------------------------------------------

TEST_F(TestObjectFixture, ParseSerializeNonEmpty)
{
    protogeneratorqt::TestObject test_object;
    test_object.SetStateInteger(993);
    protogeneratorqt::IpEndpoint optional_ip_endpoint;
    optional_ip_endpoint.set_address(kAddress1);
    optional_ip_endpoint.set_port(kPort1);
    test_object.SetOptionalIpEndpoint(optional_ip_endpoint);
    test_object.SetOptionalInteger(511);
    test_object.SetOptionalString("Hello");
    test_object.SetOptionalEnum(protogeneratorqt::TestEnumEnum::OPTION_2);
    std::unique_ptr<protogeneratorqt::IpEndpoint> val_o = std::make_unique<protogeneratorqt::IpEndpoint>();
    val_o->set_address(kAddress2);
    val_o->set_port(kPort2);
    protogeneratorqt::IpEndpoint r_o_0;
    r_o_0.set_address("255.255.255.255");
    r_o_0.set_port(5101);
    google::protobuf::RepeatedPtrField<protogeneratorqt::IpEndpoint> r_o;
    r_o.Add(std::move(r_o_0));
    test_object.GetRepeatedIpEndpoint()->Set(r_o);
    test_object.GetRepeatedString()->Append("GG WP");
    test_object.GetRepeatedString()->Append("GH gl qq.");
    test_object.GetRepeatedEnum()->Append(protogeneratorqt::TestEnumEnum::OPTION_2);
    test_object.GetRepeatedEnum()->Append(protogeneratorqt::TestEnumEnum::OPTION_1);
    test_object.SetIntegerField(1861);
    test_object.SetStringField("Lorem ipsum");
    protogeneratorqt::IpEndpoint ip_endpoint_field;
    ip_endpoint_field.set_address("0.0.0.0");
    ip_endpoint_field.set_port(443);
    test_object.SetIpEndpointField(std::move(ip_endpoint_field));
    test_object.SetEnumField(protogeneratorqt::TestEnumEnum::OPTION_2);

    QByteArray serialized_object = test_object.Serialize();

    bool parsing_result = test_object_->Parse(serialized_object);

    ASSERT_TRUE(parsing_result);
    EXPECT_TRUE(test_object_->EqualsTo(&test_object));
    IncAllSignals();
    ApplicationCheckSignals();
}
