#include "qt_core_test.h"



void prototest::QtCoreTest::AppSetUp(int args /*= 1*/, char* argv /*= ""*/)
{
    app_ = std::make_unique<QGuiApplication>(args, &argv);
}

void prototest::QtCoreTest::AppTearDown()
{
    app_.reset();
    SignalsTrackingTearDown();
}

void prototest::QtCoreTest::SignalsTrackingTearDown()
{
    signals_spies_.clear();
    signals_counts_.clear();
}

void prototest::QtCoreTest::ProcessEvents(int timeout)
{
    QCoreApplication::processEvents(QEventLoop::AllEvents, timeout);
}

void prototest::QtCoreTest::IncAllSignals(int count /*= 1*/)
{
    for (auto& c : signals_counts_) c += count;
}

void prototest::QtCoreTest::CheckSignals()
{}

void prototest::QtCoreTest::ApplicationCheckSignals(int timeout /*= 5000*/)
{
    ProcessEvents(timeout);
    CheckSignals();
}

protogeneratorqt::IpEndpoint prototest::QtCoreTest::CreateIpEndpoint(const char* address, const int port)
{
    protogeneratorqt::IpEndpoint ip_endpoint;
    ip_endpoint.set_address(address);
    ip_endpoint.set_port(port);
    return ip_endpoint;
}
