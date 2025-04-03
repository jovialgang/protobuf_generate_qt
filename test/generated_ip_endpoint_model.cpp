#include "data_test.h"
#include "test_qt_pb.h"
#include "qt_core_test.h"

#include <google/protobuf/repeated_ptr_field.h>
#include <memory>

namespace prototest
{
enum IpEndpointModelSignals { kChanged = 0 };

class IpEndpointModelFixture : public QtCoreTest
{
protected:
    void SetUp() override;
    void TearDown() override;
    void SignalsTrackingSetUp() override;
    // Увеличение счетчика всех сигналов, индексы которых переданы
    void IncSignals(const std::set<IpEndpointModelSignals>& signals_set, int count = 1);

    // Проверка совпадения ожидаемого количества сигналов и пойманного
    void CheckSignals() override;

protected:
    void SetGetProtoMessage(bool emit_all_signals);
    void SetGet(bool is_rvalue);

protected:
    std::unique_ptr<protogeneratorqt::IpEndpointModel>                                ip_endpoint_model_;
    std::unique_ptr<google::protobuf::RepeatedPtrField<protogeneratorqt::IpEndpoint>> data_;
};

void IpEndpointModelFixture::SetUp()
{
    AppSetUp();
    data_ = std::make_unique<google::protobuf::RepeatedPtrField<protogeneratorqt::IpEndpoint>>();
    // Нет дефолтного конструктора
    ip_endpoint_model_ = std::make_unique<protogeneratorqt::IpEndpointModel>(data_.get());
    SignalsTrackingSetUp();
}

void IpEndpointModelFixture::TearDown()
{
    AppTearDown();
    ip_endpoint_model_.reset();
    data_.reset();
}

void IpEndpointModelFixture::SignalsTrackingSetUp()
{
    signals_counts_ = std::vector(1, 0);
    signals_spies_.push_back(std::make_unique<QSignalSpy>(ip_endpoint_model_.get(), &protogeneratorqt::IpEndpointModel::changed));
}

void IpEndpointModelFixture::IncSignals(const std::set<IpEndpointModelSignals>& signals_set, int count /*= 1*/)
{
    for (auto i : signals_set) 
        signals_counts_[i] += count;
}

void IpEndpointModelFixture::CheckSignals()
{
    EXPECT_EQ(signals_spies_[IpEndpointModelSignals::kChanged]->count(), signals_counts_[IpEndpointModelSignals::kChanged]);
}

void IpEndpointModelFixture::SetGetProtoMessage(bool emit_all_signals)
{
    google::protobuf::RepeatedPtrField<protogeneratorqt::IpEndpoint> data_in_first;
    data_in_first.Add(CreateIpEndpoint(kAddress1, kPort1));
    data_in_first.Add(CreateIpEndpoint(kAddress2, kPort2));
    data_in_first.Add(CreateIpEndpoint(kAddress3, kPort3));
    ip_endpoint_model_->SetProtoMessage(&data_in_first, emit_all_signals);

    auto data_first = ip_endpoint_model_->GetProtoMessage();
    EXPECT_EQ(data_first, &data_in_first);
    IncAllSignals();
    ApplicationCheckSignals();

    google::protobuf::RepeatedPtrField<protogeneratorqt::IpEndpoint> data_in_second;
    data_in_second.Add(CreateIpEndpoint(kAddress3, kPort3));
    data_in_second.Add(CreateIpEndpoint(kAddress2, kPort2));
    data_in_second.Add(CreateIpEndpoint(kAddress1, kPort1));
    ip_endpoint_model_->SetProtoMessage(&data_in_second, emit_all_signals);

    auto data_second = ip_endpoint_model_->GetProtoMessage();
    EXPECT_EQ(data_second, &data_in_second);
    if (emit_all_signals)
        IncAllSignals();
    IncAllSignals();
    ApplicationCheckSignals();

    ip_endpoint_model_->SetProtoMessage(&data_in_second, emit_all_signals);

    auto data_third = ip_endpoint_model_->GetProtoMessage();
    EXPECT_EQ(data_third, &data_in_second);
    if (emit_all_signals)
        IncAllSignals();
    IncAllSignals();
    ApplicationCheckSignals();
}

void IpEndpointModelFixture::SetGet(bool is_rvalue)
{
    // Создание и установка первого google::protobuf::RepeatedPtrField<protogeneratorqt::IpEndpoint>
    google::protobuf::RepeatedPtrField<protogeneratorqt::IpEndpoint> data_in_first;
    data_in_first.Add(CreateIpEndpoint(kAddress1, kPort1));
    data_in_first.Add(CreateIpEndpoint(kAddress2, kPort2));
    data_in_first.Add(CreateIpEndpoint(kAddress3, kPort3));

    if (is_rvalue)
        ip_endpoint_model_->Set(std::move(data_in_first));
    else
        ip_endpoint_model_->Set(data_in_first);
    // Проверка значений и сигналов
    auto data_out_first = ip_endpoint_model_->Get();
    EXPECT_EQ(data_out_first[0].address(), kAddress1);
    EXPECT_EQ(data_out_first[0].port(), kPort1);
    EXPECT_EQ(data_out_first[1].address(), kAddress2);
    EXPECT_EQ(data_out_first[1].port(), kPort2);
    EXPECT_EQ(data_out_first[2].address(), kAddress3);
    EXPECT_EQ(data_out_first[2].port(), kPort3);
    IncAllSignals();
    ApplicationCheckSignals();

    // Создание и установка второго google::protobuf::RepeatedPtrField<protogeneratorqt::IpEndpoint>
    google::protobuf::RepeatedPtrField<protogeneratorqt::IpEndpoint> data_in_second;
    data_in_second.Add(CreateIpEndpoint(kAddress3, kPort3));
    data_in_second.Add(CreateIpEndpoint(kAddress2, kPort2));
    data_in_second.Add(CreateIpEndpoint(kAddress1, kPort1));

    if (is_rvalue)
        ip_endpoint_model_->Set(std::move(data_in_second));
    else
        ip_endpoint_model_->Set(data_in_second);
    // Проверка значений и сигналов
    auto data_out_second = ip_endpoint_model_->Get();
    EXPECT_EQ(data_out_second[0].address(), kAddress3);
    EXPECT_EQ(data_out_second[0].port(), kPort3);
    EXPECT_EQ(data_out_second[1].address(), kAddress2);
    EXPECT_EQ(data_out_second[1].port(), kPort2);
    EXPECT_EQ(data_out_second[2].address(), kAddress1);
    EXPECT_EQ(data_out_second[2].port(), kPort1);
    IncAllSignals(2);
    ApplicationCheckSignals();

    // Создание и установка третьего google::protobuf::RepeatedPtrField<protogeneratorqt::IpEndpoint> с данными второго
    // google::protobuf::RepeatedPtrField<protogeneratorqt::IpEndpoint>
    google::protobuf::RepeatedPtrField<protogeneratorqt::IpEndpoint> data_in_third;
    data_in_third.Add(CreateIpEndpoint(kAddress3, kPort3));
    data_in_third.Add(CreateIpEndpoint(kAddress2, kPort2));
    data_in_third.Add(CreateIpEndpoint(kAddress1, kPort1));

    if (is_rvalue)
        ip_endpoint_model_->Set(std::move(data_in_third));
    else
        ip_endpoint_model_->Set(data_in_third);
    // Проверка значений и сигналов
    auto data_out_third = ip_endpoint_model_->Get();
    EXPECT_EQ(data_out_third[0].address(), kAddress3);
    EXPECT_EQ(data_out_third[0].port(), kPort3);
    EXPECT_EQ(data_out_third[1].address(), kAddress2);
    EXPECT_EQ(data_out_third[1].port(), kPort2);
    EXPECT_EQ(data_out_third[2].address(), kAddress1);
    EXPECT_EQ(data_out_third[2].port(), kPort1);
    IncAllSignals();
    ApplicationCheckSignals();
}
};  // namespace prototest

// Тесты модели TestIpEndpointModel
using namespace prototest;
//> Одиночные
// @@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@
//+> Конструктор
// vvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvv
TEST_F(IpEndpointModelFixture, Constructor)
{
    EXPECT_NO_FATAL_FAILURE(protogeneratorqt::IpEndpointModel(data_.get()));
}

//+> Публичные функции
// vvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvv

//++> SetGetProtoMessageEmitAllSignalFalse
// Проверка функций SetProtoMessage с параметром emit_all_signals = false и GetProtoMessage
// Ожидается отсутствие сигналов
// ----------------------------------------------------------------------------------------------------

TEST_F(IpEndpointModelFixture, SetGetProtoMessageEmitAllSignalFalse)
{
    SetGetProtoMessage(false);
}

//++> SetGetProtoMessageEmitAllSignalTrue
// Проверка функций SetProtoMessage с параметром emit_all_signals = true и GetProtoMessage
// Ожидается получение всех сигналов
// ----------------------------------------------------------------------------------------------------

TEST_F(IpEndpointModelFixture, SetGetProtoMessageEmitAllSignalTrue)
{
    SetGetProtoMessage(true);
}

//++> CheckForChangedAndSetProtoMessage
// Проверка функций CheckForChangedAndSetProtoMessage
// Ожидается получение сигналов об изменениях
// ----------------------------------------------------------------------------------------------------

TEST_F(IpEndpointModelFixture, CheckForChangedAndSetProtoMessage)
{
    google::protobuf::RepeatedPtrField<protogeneratorqt::IpEndpoint> data_in_first;
    data_in_first.Add(CreateIpEndpoint(kAddress1, kPort1));
    data_in_first.Add(CreateIpEndpoint(kAddress2, kPort2));
    data_in_first.Add(CreateIpEndpoint(kAddress3, kPort3));
    ip_endpoint_model_->CheckForChangedAndSetProtoMessage(&data_in_first);

    auto data_first = ip_endpoint_model_->GetProtoMessage();
    EXPECT_EQ(data_first, &data_in_first);
    IncAllSignals();
    ApplicationCheckSignals();

    google::protobuf::RepeatedPtrField<protogeneratorqt::IpEndpoint> data_in_second;
    data_in_second.Add(CreateIpEndpoint(kAddress3, kPort3));
    data_in_second.Add(CreateIpEndpoint(kAddress2, kPort2));
    data_in_second.Add(CreateIpEndpoint(kAddress1, kPort1));
    ip_endpoint_model_->CheckForChangedAndSetProtoMessage(&data_in_second);

    auto data_second = ip_endpoint_model_->GetProtoMessage();
    EXPECT_EQ(data_second, &data_in_second);
    IncAllSignals(2);
    ApplicationCheckSignals();

    ip_endpoint_model_->CheckForChangedAndSetProtoMessage(&data_in_second);

    auto data_third = ip_endpoint_model_->GetProtoMessage();
    EXPECT_EQ(data_third, &data_in_second);
    IncAllSignals();
    ApplicationCheckSignals();
}

//++> SetGetLvalue
// Проверка функций Set с аргументов Lvalue и Get
// Ожидается получение сигналов при различии соответствующих полей аргумента и тестируемого объекта
// ----------------------------------------------------------------------------------------------------

TEST_F(IpEndpointModelFixture, SetGetLvalue)
{
    SetGet(false);
}

//++> SetGetRvalue
// Проверка функций Set с аргументов Rvalue и Get
// Ожидается получение сигналов при различии соответствующих полей аргумента и тестируемого объекта
// ----------------------------------------------------------------------------------------------------

TEST_F(IpEndpointModelFixture, SetGetRvalue)
{
    SetGet(true);
}

// ++> SyncData
// ----------------------------------------------------------------------------------------------------
TEST_F(IpEndpointModelFixture, SyncData)
{
    ip_endpoint_model_->SyncData();
    IncAllSignals();
    ApplicationCheckSignals();

    ip_endpoint_model_->SyncData();
    IncAllSignals();
    ApplicationCheckSignals();
}

//++> Iterator
// Проверка Iterator
// ----------------------------------------------------------------------------------------------------
TEST_F(IpEndpointModelFixture, Iterator)
{
    google::protobuf::RepeatedPtrField<protogeneratorqt::IpEndpoint> data_in_first;
    data_in_first.Add(CreateIpEndpoint(kAddress1, kPort1));
    data_in_first.Add(CreateIpEndpoint(kAddress2, kPort2));
    data_in_first.Add(CreateIpEndpoint(kAddress3, kPort3));
    ip_endpoint_model_->SetProtoMessage(&data_in_first);

    auto it = ip_endpoint_model_->begin();
    EXPECT_EQ((*it)->GetAddress(), kAddress1);
    EXPECT_EQ((*it)->GetPort(), kPort1);
    ++it;
    EXPECT_EQ((*it)->GetAddress(), kAddress2);
    EXPECT_EQ((*it)->GetPort(), kPort2);
    ++it;
    EXPECT_EQ((*it)->GetAddress(), kAddress3);
    EXPECT_EQ((*it)->GetPort(), kPort3);
    ++it;
    EXPECT_EQ(it, ip_endpoint_model_->end());
}

//++> ConstIteratorC
// Проверка const Iterator
// ----------------------------------------------------------------------------------------------------
TEST_F(IpEndpointModelFixture, ConstIterator)
{
    google::protobuf::RepeatedPtrField<protogeneratorqt::IpEndpoint> data_in_first;
    data_in_first.Add(CreateIpEndpoint(kAddress1, kPort1));
    data_in_first.Add(CreateIpEndpoint(kAddress2, kPort2));
    data_in_first.Add(CreateIpEndpoint(kAddress3, kPort3));
    ip_endpoint_model_->SetProtoMessage(&data_in_first);

    auto it = ip_endpoint_model_->cbegin();
    EXPECT_EQ((*it)->GetAddress(), kAddress1);
    EXPECT_EQ((*it)->GetPort(), kPort1);
    ++it;
    EXPECT_EQ((*it)->GetAddress(), kAddress2);
    EXPECT_EQ((*it)->GetPort(), kPort2);
    ++it;
    EXPECT_EQ((*it)->GetAddress(), kAddress3);
    EXPECT_EQ((*it)->GetPort(), kPort3);
    ++it;
    EXPECT_EQ(it, ip_endpoint_model_->cend());
}

//++> At
// Проверка At
// ----------------------------------------------------------------------------------------------------
TEST_F(IpEndpointModelFixture, At)
{
    google::protobuf::RepeatedPtrField<protogeneratorqt::IpEndpoint> data_in_first;
    data_in_first.Add(CreateIpEndpoint(kAddress1, kPort1));
    data_in_first.Add(CreateIpEndpoint(kAddress2, kPort2));
    data_in_first.Add(CreateIpEndpoint(kAddress3, kPort3));
    ip_endpoint_model_->SetProtoMessage(&data_in_first);

    EXPECT_EQ(ip_endpoint_model_->At(0)->GetAddress(), kAddress1);
    EXPECT_EQ(ip_endpoint_model_->At(0)->GetPort(), kPort1);
    EXPECT_EQ(ip_endpoint_model_->At(1)->GetAddress(), kAddress2);
    EXPECT_EQ(ip_endpoint_model_->At(1)->GetPort(), kPort2);
    EXPECT_EQ(ip_endpoint_model_->At(2)->GetAddress(), kAddress3);
    EXPECT_EQ(ip_endpoint_model_->At(2)->GetPort(), kPort3);
    EXPECT_EQ(ip_endpoint_model_->At(-1), nullptr);
    EXPECT_EQ(ip_endpoint_model_->At(3), nullptr);
    EXPECT_EQ(ip_endpoint_model_->At(32768), nullptr);
    EXPECT_EQ(ip_endpoint_model_->At(32769), nullptr);
    EXPECT_EQ(ip_endpoint_model_->At(65535), nullptr);
    EXPECT_EQ(ip_endpoint_model_->At(65536), nullptr);
    EXPECT_EQ(ip_endpoint_model_->At(-32768), nullptr);
    EXPECT_EQ(ip_endpoint_model_->At(-32769), nullptr);
    EXPECT_EQ(ip_endpoint_model_->At(-65535), nullptr);
    EXPECT_EQ(ip_endpoint_model_->At(-65536), nullptr);

    IncAllSignals();
    ApplicationCheckSignals();
}

//++> First
// Проверка First
// ----------------------------------------------------------------------------------------------------
TEST_F(IpEndpointModelFixture, First)
{
    google::protobuf::RepeatedPtrField<protogeneratorqt::IpEndpoint> data_in_first;
    data_in_first.Add(CreateIpEndpoint(kAddress1, kPort1));
    data_in_first.Add(CreateIpEndpoint(kAddress2, kPort2));
    data_in_first.Add(CreateIpEndpoint(kAddress3, kPort3));
    ip_endpoint_model_->SetProtoMessage(&data_in_first);

    // Первый вызов First
    auto first_element = ip_endpoint_model_->First();
    EXPECT_EQ(first_element->GetAddress(), kAddress1);
    EXPECT_EQ(first_element->GetPort(), kPort1);
    // Повторный вызов First
    EXPECT_EQ(ip_endpoint_model_->First()->GetAddress(), kAddress1);
    EXPECT_EQ(ip_endpoint_model_->First()->GetPort(), kPort1);

    // Изменение значения первого элемента
    ip_endpoint_model_->First()->SetAddress(kAddress2);
    ip_endpoint_model_->First()->SetPort(kPort2);

    // Проверка изменения
    EXPECT_EQ(ip_endpoint_model_->First()->GetAddress(), kAddress2);
    EXPECT_EQ(ip_endpoint_model_->First()->GetPort(), kPort2);
    // Повторный вызов
    EXPECT_EQ(ip_endpoint_model_->First()->GetAddress(), kAddress2);
    EXPECT_EQ(ip_endpoint_model_->First()->GetPort(), kPort2);

    google::protobuf::RepeatedPtrField<protogeneratorqt::IpEndpoint> data_in_second;
    data_in_second.Add(CreateIpEndpoint(kAddress3, kPort3));
    data_in_second.Add(CreateIpEndpoint(kAddress2, kPort2));
    data_in_second.Add(CreateIpEndpoint(kAddress1, kPort1));
    ip_endpoint_model_->SetProtoMessage(&data_in_second);

    // Проверка после установки data
    EXPECT_EQ(ip_endpoint_model_->First()->GetAddress(), kAddress3);
    EXPECT_EQ(ip_endpoint_model_->First()->GetPort(), kPort3);
    // Изменение первого элемента
    data_in_second[0].set_address(kAddress1);
    data_in_second[0].set_port(kPort1);
    // Повторный вызов для проверки изменений
    EXPECT_EQ(ip_endpoint_model_->First()->GetAddress(), kAddress1);
    EXPECT_EQ(ip_endpoint_model_->First()->GetPort(), kPort1);

    IncAllSignals(2);
    ApplicationCheckSignals();
}

//++> Last
// Проверка Last
// ----------------------------------------------------------------------------------------------------
TEST_F(IpEndpointModelFixture, Last)
{
    google::protobuf::RepeatedPtrField<protogeneratorqt::IpEndpoint> data_in_first;
    data_in_first.Add(CreateIpEndpoint(kAddress1, kPort1));
    data_in_first.Add(CreateIpEndpoint(kAddress2, kPort2));
    data_in_first.Add(CreateIpEndpoint(kAddress3, kPort3));
    ip_endpoint_model_->CheckForChangedAndSetProtoMessage(&data_in_first);

    auto last_element_first = ip_endpoint_model_->Last();
    EXPECT_EQ(last_element_first->GetAddress(), kAddress3);
    EXPECT_EQ(last_element_first->GetPort(), kPort3);
    IncAllSignals();
    ApplicationCheckSignals();

    // Установка нового ProtoMessage и проверка равенства последненего элемента
    google::protobuf::RepeatedPtrField<protogeneratorqt::IpEndpoint> data_in_second;
    data_in_second.Add(CreateIpEndpoint(kAddress4, kPort4));
    data_in_second.Add(CreateIpEndpoint(kAddress3, kPort3));
    data_in_second.Add(CreateIpEndpoint(kAddress2, kPort2));
    data_in_second.Add(CreateIpEndpoint(kAddress1, kPort1));
    ip_endpoint_model_->CheckForChangedAndSetProtoMessage(&data_in_second);

    auto last_element_second = ip_endpoint_model_->Last();
    EXPECT_EQ(last_element_second->GetAddress(), kAddress1);
    EXPECT_EQ(last_element_second->GetPort(), kPort1);
    IncAllSignals(2);
    ApplicationCheckSignals();

    ip_endpoint_model_->Clear();
    auto last_element_third = ip_endpoint_model_->Last();
    EXPECT_EQ(last_element_third, nullptr);
}

// // ++> Take
// // ----------------------------------------------------------------------------------------------------
// TEST_F(IpEndpointModelFixture, Take)
// {
//     // Установка нового ProtoMessage и проверка равенства последненего элемента
//     google::protobuf::RepeatedPtrField<protogeneratorqt::IpEndpoint> data_in_first;
//     data_in_first.Add(CreateIpEndpoint(kAddressA, kPortA));
//     data_in_first.Add(CreateIpEndpoint(kAddressB, kPortB));
//     data_in_first.Add(CreateIpEndpoint(kAddressC, kPortC));
//     data_in_first.Add(CreateIpEndpoint(kAddressD, kPortD));
//     ip_endpoint_model_->CheckForChangedAndSetProtoMessage(&data_in_first);
// 
//     auto element_second = ip_endpoint_model_->Take(1);
//     EXPECT_EQ(element_second->GetAddress(), kAddressB);
//     EXPECT_EQ(element_second->GetPort(), kPortB);
// }