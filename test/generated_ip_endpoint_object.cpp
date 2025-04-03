#include <QByteArray>
#include <QDebug>
#include <QSignalSpy>
#include <QString>
#include <QTest>
#include <gtest/gtest.h>

#include "test_qt_pb.h"
#include "qt_core_test.h"

namespace prototest
{
enum IpEndpointObjectSignals { kChanged = 0, kAddressChanged = 1, kPortChanged = 2 };

class IpEndpointObjectFixture : public QtCoreTest
{
protected:
    void SetUp() override;
    void TearDown() override;
    void SignalsTrackingSetUp() override;
    void IncSignals(const std::set<IpEndpointObjectSignals>& signals_set, int count = 1);
    // Проверка совпадения ожидаемого количества сигналов и пойманного
    void CheckSignals() override;
    void CheckAddressAndPort(QString address, int port);

protected:
    void SetGetProtoMessage(bool emit_all_signals);
    void SetGet(bool is_rvalue);
    void CopyMerge(bool is_merge);
    void ParseSerialized(bool is_empty);

protected:
    std::unique_ptr<protogeneratorqt::IpEndpointObject> ip_endpoint_object_;

    std::unique_ptr<protogeneratorqt::IpEndpoint> message_;
};

void prototest::IpEndpointObjectFixture::SetUp()
{
    AppSetUp();
    message_            = std::make_unique<protogeneratorqt::IpEndpoint>();
    ip_endpoint_object_ = std::make_unique<protogeneratorqt::IpEndpointObject>();
    SignalsTrackingSetUp();
}

void prototest::IpEndpointObjectFixture::TearDown()
{
    AppTearDown();
    ip_endpoint_object_.reset();
    message_.reset();
}

void prototest::IpEndpointObjectFixture::IncSignals(const std::set<prototest::IpEndpointObjectSignals>& signals_set, int count /*= 1*/)
{
    for (auto i : signals_set) signals_counts_[i] += count;
}

void prototest::IpEndpointObjectFixture::SignalsTrackingSetUp()
{
    // Заполняем вектор элементами слушающими сигналы
    signals_counts_ = std::vector<int>(3, 0);
    signals_spies_.push_back(std::make_unique<QSignalSpy>(ip_endpoint_object_.get(), &protogeneratorqt::IpEndpointObject::changed));
    signals_spies_.push_back(std::make_unique<QSignalSpy>(ip_endpoint_object_.get(), &protogeneratorqt::IpEndpointObject::addressChanged));
    signals_spies_.push_back(std::make_unique<QSignalSpy>(ip_endpoint_object_.get(), &protogeneratorqt::IpEndpointObject::portChanged));
}

void prototest::IpEndpointObjectFixture::CheckSignals()
{
    // Выполняем проверку пришедшего(через .count()) и ожидаемого количества сигналов
    EXPECT_EQ(signals_spies_[IpEndpointObjectSignals::kChanged]->count(), signals_counts_[IpEndpointObjectSignals::kChanged]);
    EXPECT_EQ(signals_spies_[IpEndpointObjectSignals::kAddressChanged]->count(), signals_counts_[IpEndpointObjectSignals::kAddressChanged]);
    EXPECT_EQ(signals_spies_[IpEndpointObjectSignals::kPortChanged]->count(), signals_counts_[IpEndpointObjectSignals::kPortChanged]);
}

void prototest::IpEndpointObjectFixture::CheckAddressAndPort(QString address, int port)
{
    EXPECT_EQ(ip_endpoint_object_->GetAddress(), address);
    EXPECT_EQ(ip_endpoint_object_->GetPort(), port);
}

void prototest::IpEndpointObjectFixture::SetGetProtoMessage(bool emit_all_signals)
{
    // Создание и установка первого message
    protogeneratorqt::IpEndpoint message_in_first = CreateIpEndpoint(kAddress1, kPort1);
    ip_endpoint_object_->SetProtoMessage(&message_in_first, emit_all_signals);
    // Проверка установки и сигналов
    protogeneratorqt::IpEndpoint* message_out_first = ip_endpoint_object_->GetProtoMessage();
    EXPECT_EQ(message_out_first, &message_in_first);
    if (emit_all_signals)
        IncAllSignals();
    ApplicationCheckSignals();

    // Создание и установка второго message
    protogeneratorqt::IpEndpoint message_in_second = CreateIpEndpoint(kAddress2, kPort2);
    ip_endpoint_object_->SetProtoMessage(&message_in_second, emit_all_signals);
    // Проверка установки и сигналов
    protogeneratorqt::IpEndpoint* message_out_second = ip_endpoint_object_->GetProtoMessage();
    EXPECT_EQ(message_out_second, &message_in_second);
    if (emit_all_signals)
        IncAllSignals();
    ApplicationCheckSignals();

    // Повторная установка второго message
    ip_endpoint_object_->SetProtoMessage(&message_in_second, emit_all_signals);
    // Проверка установки и сигналов
    protogeneratorqt::IpEndpoint* message_out_third = ip_endpoint_object_->GetProtoMessage();
    EXPECT_EQ(message_out_third, &message_in_second);
    if (emit_all_signals)
        IncAllSignals();
    ApplicationCheckSignals();
}

void prototest::IpEndpointObjectFixture::SetGet(bool is_rvalue)
{
    // Создание и установка первого ip_endpoint
    protogeneratorqt::IpEndpoint ip_endpoint_in_first = CreateIpEndpoint(kAddress1, kPort1);

    if (is_rvalue)
        ip_endpoint_object_->Set(std::move(ip_endpoint_in_first));
    else
        ip_endpoint_object_->Set(ip_endpoint_in_first);
    // Проверка значений и сигналов
    protogeneratorqt::IpEndpoint ip_endpoint_out_first = ip_endpoint_object_->Get();
    EXPECT_EQ(ip_endpoint_out_first.address(), kAddress1);
    EXPECT_EQ(ip_endpoint_out_first.port(), kPort1);
    IncAllSignals();
    ApplicationCheckSignals();

    // Создание и установка второго ip_endpoint
    protogeneratorqt::IpEndpoint ip_endpoint_in_second = CreateIpEndpoint(kAddress2, kPort2);

    if (is_rvalue)
        ip_endpoint_object_->Set(std::move(ip_endpoint_in_second));
    else
        ip_endpoint_object_->Set(ip_endpoint_in_second);
    // Проверка значений и сигналов
    protogeneratorqt::IpEndpoint ip_endpoint_out_second = ip_endpoint_object_->Get();
    EXPECT_EQ(ip_endpoint_out_second.address(), kAddress2);
    EXPECT_EQ(ip_endpoint_out_second.port(), kPort2);
    IncAllSignals();
    ApplicationCheckSignals();

    // Создание и установка третьего ip_endpoint с данными второго ip_endpoint
    protogeneratorqt::IpEndpoint ip_endpoint_in_third = CreateIpEndpoint(kAddress2, kPort2);

    if (is_rvalue)
        ip_endpoint_object_->Set(std::move(ip_endpoint_in_third));
    else
        ip_endpoint_object_->Set(ip_endpoint_in_third);
    // Проверка значений и отсутсвия новых сигналов
    protogeneratorqt::IpEndpoint ip_endpoint_out_third = ip_endpoint_object_->Get();
    EXPECT_EQ(ip_endpoint_out_third.address(), kAddress2);
    EXPECT_EQ(ip_endpoint_out_third.port(), kPort2);
    ApplicationCheckSignals();
}

void prototest::IpEndpointObjectFixture::CopyMerge(bool is_merge)
{
    // Создание и Merge/Copy первого ip_endpoint_object
    protogeneratorqt::IpEndpointObject ip_endpoint_object_first = CreateIpEndpoint(kAddress1, kPort1);

    if (is_merge)
        ip_endpoint_object_->MergeFrom(&ip_endpoint_object_first);
    else
        ip_endpoint_object_->CopyFrom(&ip_endpoint_object_first);
    // Проверка значений и сигналов
    EXPECT_EQ(ip_endpoint_object_->GetAddress(), kAddress1);
    EXPECT_EQ(ip_endpoint_object_->GetPort(), kPort1);
    IncAllSignals();
    ApplicationCheckSignals();

    // Создание и Merge/Copy второго ip_endpoint_object
    protogeneratorqt::IpEndpointObject ip_endpoint_object_second = CreateIpEndpoint(kAddress2, kPort2);

    if (is_merge)
        ip_endpoint_object_->MergeFrom(&ip_endpoint_object_second);
    else
        ip_endpoint_object_->CopyFrom(&ip_endpoint_object_second);
    // Проверка значений и сигналов
    EXPECT_EQ(ip_endpoint_object_->GetAddress(), kAddress2);
    EXPECT_EQ(ip_endpoint_object_->GetPort(), kPort2);
    IncAllSignals();
    ApplicationCheckSignals();

    // Создание и Merge/Copy третьего ip_endpoint_object с данными второго ip_endpoint_object
    protogeneratorqt::IpEndpointObject ip_endpoint_object_third = CreateIpEndpoint(kAddress2, kPort2);

    if (is_merge)
        ip_endpoint_object_->MergeFrom(&ip_endpoint_object_third);
    else
        ip_endpoint_object_->CopyFrom(&ip_endpoint_object_third);
    // Проверка значений и отсутствия новых сигналов
    EXPECT_EQ(ip_endpoint_object_->GetAddress(), kAddress2);
    EXPECT_EQ(ip_endpoint_object_->GetPort(), kPort2);
    ApplicationCheckSignals();
}

void prototest::IpEndpointObjectFixture::ParseSerialized(bool is_empty)
{
    if (!is_empty)
    {
        ip_endpoint_object_->SetAddress(kAddress1);
        ip_endpoint_object_->SetPort(kPort1);
        IncAllSignals();
    }
    auto serialized = ip_endpoint_object_->Serialize();
    ip_endpoint_object_->Parse(serialized);
    IncAllSignals();
    ApplicationCheckSignals();
}

};  // namespace prototest

using namespace prototest;

//> Одиночные
// @@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@

//+> Конструкторы
// ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
TEST_F(IpEndpointObjectFixture, Constructor)
{
    EXPECT_NO_FATAL_FAILURE(protogeneratorqt::IpEndpointObject());
    EXPECT_NO_FATAL_FAILURE(protogeneratorqt::IpEndpointObject(message_.get()));
}

//+> Публичные функции
// ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

//++> SetGetProtoMessageEmitAllSignalFalse
// Проверка функций SetProtoMessage с параметром emit_all_signals = false и GetProtoMessage
// Ожидается отсутствие сигналов
// ----------------------------------------------------------------------------------------------------

TEST_F(IpEndpointObjectFixture, SetGetProtoMessageEmitAllSignalFalse)
{
    SetGetProtoMessage(false);
}

//++> SetGetProtoMessageEmitAllSignalTrue
// Проверка функций SetProtoMessage с параметром emit_all_signals = true и GetProtoMessage
// Ожидается получение всех сигналов
// ----------------------------------------------------------------------------------------------------

TEST_F(IpEndpointObjectFixture, SetGetProtoMessageEmitAllSignalTrue)
{
    SetGetProtoMessage(true);
}

//++> SetGetLvalue
// Проверка функций Set с аргументов Lvalue и Get
// Ожидается получение сигналов при различии соответствующих полей аргумента и тестируемого объекта
// ----------------------------------------------------------------------------------------------------

TEST_F(IpEndpointObjectFixture, SetGetLvalue)
{
    SetGet(false);
}

//++> SetGetRvalue
// Проверка функций Set с аргументов Rvalue и Get
// Ожидается получение сигналов при различии соответствующих полей аргумента и тестируемого объекта
// ----------------------------------------------------------------------------------------------------

TEST_F(IpEndpointObjectFixture, SetGetRvalue)
{
    SetGet(true);
}

//++> SetGetAddress
// Проверка функций SetAddress и GetAddress
// Ожидается получение сигналов при различии соответствующих полей аргумента и тестируемого объекта
// ----------------------------------------------------------------------------------------------------

TEST_F(IpEndpointObjectFixture, SetGetAddress)
{
    ip_endpoint_object_->SetAddress(kAddress1);
    EXPECT_EQ(ip_endpoint_object_->GetAddress(), kAddress1);
    IncSignals({ IpEndpointObjectSignals::kChanged, IpEndpointObjectSignals::kAddressChanged });
    ApplicationCheckSignals();

    ip_endpoint_object_->SetAddress(kAddress2);
    EXPECT_EQ(ip_endpoint_object_->GetAddress(), kAddress2);
    IncSignals({ IpEndpointObjectSignals::kChanged, IpEndpointObjectSignals::kAddressChanged });
    ApplicationCheckSignals();

    ip_endpoint_object_->SetAddress(kAddress2);
    EXPECT_EQ(ip_endpoint_object_->GetAddress(), kAddress2);
    ApplicationCheckSignals();
}

//++> SetGetPort
// Проверка функций SetPort и GetPort
// Ожидается получение сигналов при различии соответствующих полей аргумента и тестируемого объекта
// ----------------------------------------------------------------------------------------------------

TEST_F(IpEndpointObjectFixture, SetGetPort)
{
    ip_endpoint_object_->SetPort(kPort1);
    EXPECT_EQ(ip_endpoint_object_->GetPort(), kPort1);
    IncSignals({ IpEndpointObjectSignals::kChanged, IpEndpointObjectSignals::kPortChanged });
    ApplicationCheckSignals();

    ip_endpoint_object_->SetPort(kPort2);
    EXPECT_EQ(ip_endpoint_object_->GetPort(), kPort2);
    IncSignals({ IpEndpointObjectSignals::kChanged, IpEndpointObjectSignals::kPortChanged });
    ApplicationCheckSignals();

    ip_endpoint_object_->SetPort(kPort2);
    EXPECT_EQ(ip_endpoint_object_->GetPort(), kPort2);
    ApplicationCheckSignals();
}

//++> SetGetAddressPort
// Проверка функций SetAdress, SetPort, GetAddress, GetPort
// Ожидается получение сигналов при различии соответствующих полей аргумента и тестируемого объекта
// ----------------------------------------------------------------------------------------------------

TEST_F(IpEndpointObjectFixture, SetGetAddressPort)
{
    ip_endpoint_object_->SetAddress(kAddress1);
    ip_endpoint_object_->SetPort(kPort1);
    EXPECT_EQ(ip_endpoint_object_->GetAddress(), kAddress1);
    EXPECT_EQ(ip_endpoint_object_->GetPort(), kPort1);
    IncSignals({ IpEndpointObjectSignals::kChanged, IpEndpointObjectSignals::kAddressChanged, IpEndpointObjectSignals::kPortChanged });
    ApplicationCheckSignals();

    ip_endpoint_object_->SetAddress(kAddress2);
    ip_endpoint_object_->SetPort(kPort2);
    EXPECT_EQ(ip_endpoint_object_->GetAddress(), kAddress2);
    EXPECT_EQ(ip_endpoint_object_->GetPort(), kPort2);
    IncSignals({ IpEndpointObjectSignals::kChanged, IpEndpointObjectSignals::kAddressChanged, IpEndpointObjectSignals::kPortChanged });
    ApplicationCheckSignals();

    ip_endpoint_object_->SetAddress(kAddress2);
    ip_endpoint_object_->SetPort(kPort2);
    EXPECT_EQ(ip_endpoint_object_->GetAddress(), kAddress2);
    EXPECT_EQ(ip_endpoint_object_->GetPort(), kPort2);
    ApplicationCheckSignals();

    ip_endpoint_object_->SetAddress(kAddress1);
    EXPECT_EQ(ip_endpoint_object_->GetAddress(), kAddress1);
    EXPECT_EQ(ip_endpoint_object_->GetPort(), kPort2);
    IncSignals({ IpEndpointObjectSignals::kChanged, IpEndpointObjectSignals::kAddressChanged});
    ApplicationCheckSignals();

    ip_endpoint_object_->SetPort(kPort1);
    EXPECT_EQ(ip_endpoint_object_->GetPort(), kPort1);
    IncSignals({ IpEndpointObjectSignals::kChanged, IpEndpointObjectSignals::kPortChanged });
    ApplicationCheckSignals();
}

//++> CopyFrom
// Проверка функции CopyFrom
// 
// ----------------------------------------------------------------------------------------------------

TEST_F(IpEndpointObjectFixture, CopyFrom)
{
    CopyMerge(false);
}

//++> MergeFrom
// Проверка функции MergeFrom
// 
// ----------------------------------------------------------------------------------------------------

TEST_F(IpEndpointObjectFixture, MergeFrom)
{
    CopyMerge(true);
}

//++> SyncProtoMessage
// Проверка функции SyncProtoMessage
// После ее вызова должны приходить все сигналы один раз
// Присутствуют проверки на то, чтобы она не изменяла значения полей
// ----------------------------------------------------------------------------------------------------

TEST_F(IpEndpointObjectFixture, SyncProtoMessage)
{
    protogeneratorqt::IpEndpoint* message = ip_endpoint_object_->GetProtoMessage();

    message->set_address(kAddress1);
    CheckAddressAndPort(kAddress1, 0);
    ApplicationCheckSignals();

    ip_endpoint_object_->SyncProtoMessage();
    CheckAddressAndPort(kAddress1, 0);
    IncAllSignals();
    ApplicationCheckSignals();

    message->set_port(kPort1);
    CheckAddressAndPort(kAddress1, kPort1);
    ApplicationCheckSignals();

    ip_endpoint_object_->SyncProtoMessage();
    CheckAddressAndPort(kAddress1, kPort1);
    IncAllSignals();
    ApplicationCheckSignals();

    message->set_address(kAddress2);
    message->set_port(kPort2);
    CheckAddressAndPort(kAddress2, kPort2);
    ApplicationCheckSignals();

    ip_endpoint_object_->SyncProtoMessage();
    CheckAddressAndPort(kAddress2, kPort2);
    IncAllSignals();
    ApplicationCheckSignals();

    message->set_address(kAddress2);
    message->set_port(kPort2);
    CheckAddressAndPort(kAddress2, kPort2);
    ApplicationCheckSignals();

    ip_endpoint_object_->SyncProtoMessage();
    CheckAddressAndPort(kAddress2, kPort2);
    IncAllSignals();
    ApplicationCheckSignals();
}

//++> CheckForChangedAndSetProtoMessage
// Проверка функции CheckForChangedAndSetProtoMessage
// ----------------------------------------------------------------------------------------------------

TEST_F(IpEndpointObjectFixture, CheckForChangedAndSetProtoMessage)
{
    protogeneratorqt::IpEndpoint message_in_first;
    message_in_first.set_address(kAddress1);
    message_in_first.set_port(kPort1);
    ip_endpoint_object_->CheckForChangedAndSetProtoMessage(&message_in_first);

    protogeneratorqt::IpEndpoint* message_first = ip_endpoint_object_->GetProtoMessage();
    EXPECT_EQ(message_first, &message_in_first);
    IncAllSignals();
    ApplicationCheckSignals();

    protogeneratorqt::IpEndpoint message_in_second;
    message_in_first.set_address(kAddress2);
    message_in_first.set_port(kPort2);
    ip_endpoint_object_->CheckForChangedAndSetProtoMessage(&message_in_second);

    protogeneratorqt::IpEndpoint* message_second = ip_endpoint_object_->GetProtoMessage();
    EXPECT_EQ(message_second, &message_in_second);
    IncAllSignals();
    ApplicationCheckSignals();

    ip_endpoint_object_->CheckForChangedAndSetProtoMessage(&message_in_second);

    protogeneratorqt::IpEndpoint* message_third = ip_endpoint_object_->GetProtoMessage();
    EXPECT_EQ(message_third, &message_in_second);
    ApplicationCheckSignals();
}

//++> RevokeProtoMessageOwnership
// Проверка функции RevokeProtoMessageOwnership
// ----------------------------------------------------------------------------------------------------

TEST_F(IpEndpointObjectFixture, RevokeProtoMessageOwnership)
{
    protogeneratorqt::IpEndpoint* ip_endpoint = ip_endpoint_object_->GetProtoMessage();
    ip_endpoint_object_->RevokeProtoMessageOwnership();
    ip_endpoint_object_.reset();
    EXPECT_EQ(ip_endpoint->address(), "");
    EXPECT_EQ(ip_endpoint->port(), 0);
    ApplicationCheckSignals();
}


//++> ParseSerializeEmpty
// Проверка функции ParseSerializeEmpty
// ----------------------------------------------------------------------------------------------------

TEST_F(IpEndpointObjectFixture, ParseSerializeEmpty)
{
    ParseSerialized(true);
}

//++> ParseSerializeFull
// Проверка функции ParseSerializeFull
// ----------------------------------------------------------------------------------------------------

TEST_F(IpEndpointObjectFixture, ParseSerializeEmptyFull)
{
    ParseSerialized(false);
}