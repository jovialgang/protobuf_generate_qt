#pragma once

#include <QSignalSpy>

namespace prototest
{

constexpr int  kInteger1   = 1999;
constexpr int  kInteger2   = 0;
constexpr int  kInteger3   = 177512;
constexpr char kString1[]  = { "Lorem ipsum" };
constexpr char kString2[]  = { "Memento mori" };
constexpr char kString3[]  = { "Argentum" };
constexpr char kAddress1[] = { "127.0.0.1" };
constexpr char kAddress2[] = { "192.168.0.1" };
constexpr char kAddress3[] = { "111.111.111.111" };
constexpr char kAddress4[] = { "123.213.139.47" };
constexpr int  kPort1      = 1;
constexpr int  kPort2      = 20;
constexpr int  kPort3      = 300;
constexpr int  kPort4      = 4000;

constexpr int kConstructorRepeat{ 129 };
}; // namespace prototest
