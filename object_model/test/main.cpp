#ifdef _WIN32
#include <application/veh_handler.h>

#include <core/strings.h>
#include <boost/dll/runtime_symbol_info.hpp>
#endif

#ifdef _WIN32
#include <log_app_common.h>
#else
#include <log.h>
#include <log_async_frontend.h>
#include <log_file_rolling_backend.h>
#include <string.h>
#endif

#include <gtest/gtest.h>

int main(int argc, char* argv[])
{
#ifdef _WIN32
    SetErrorMode(SEM_FAILCRITICALERRORS | SEM_NOGPFAULTERRORBOX);

    COMMON_APP_LOGGER(ltTrace, "");

    const auto& application_location = boost::dll::program_location().parent_path();
#endif
    
#ifdef _WIN32
    application::InitializeApplicationDumpLocation(strings::WStringToUtf8String(application_location.wstring()).c_str());
    application::InitializeVehHandler();
    application::InitializeCrtErrorHandlers();
#endif

    ::testing::InitGoogleTest(&argc, argv);

#ifdef _WIN32
    if (IsDebuggerPresent())
        ::testing::GTEST_FLAG(break_on_failure) = true;
#endif  // _WIN32

    return RUN_ALL_TESTS();
}
