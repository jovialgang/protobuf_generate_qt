#pragma once

#include "data_test.h"
#include "test_qt_pb.h"

#include <gtest/gtest.h>
#include <QGuiApplication>
#include <QCoreApplication>
#include <QSignalSpy>
#include <QEventLoop>

#include <vector>

namespace prototest
{
class QtCoreTest : public ::testing::Test
{
public:
    // Запуск приложения
    void AppSetUp(int args = 1, char* argv = "");
    // Отсановка приложения
    void AppTearDown();

    // Метод переопределяется под нужные сигналы
    virtual void SignalsTrackingSetUp() = 0;
    void SignalsTrackingTearDown();

    // Вызывется с задержкой по умолчанию 5000 мс, т.к. нужно убедится что не приходит лишних сигналов
    // В том числе вызванных через QMetaObject::invokeMethod
    void ProcessEvents(int timeout = 5000);

    void IncAllSignals(int count = 1);

    // Сверка signals_spies_ и signals_counts_
    virtual void CheckSignals();
    void         ApplicationCheckSignals(int timeout = 5000);

    protogeneratorqt::IpEndpoint CreateIpEndpoint(const char* address, const int port);


protected:
    std::unique_ptr<QGuiApplication> app_;

    // Для сравниния сигналов, signals_spies_ - отслеживают приходящие сигналы, signals_counts_ ожидаемое количество сигналов
    std::vector<std::unique_ptr<QSignalSpy>> signals_spies_;
    std::vector<int>                         signals_counts_;
};
}; // namespace prototest