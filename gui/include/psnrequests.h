#pragma once

#include "jsonrequester.h"
#include <chiaki/remote/holepunch.h>
#include <functional>

class StreamSession;

// External request boundaries; parsing, persistence and attempt ownership stay native.
struct PsnRequests
{
    JsonRequester::ManagerFactory networkManager;
    std::function<ChiakiErrorCode(const char *, ChiakiHolepunchConsoleType, ChiakiHolepunchDeviceInfo **, size_t *, ChiakiLog *)> listDevices = chiaki_holepunch_list_devices;
    using Connect = std::function<ChiakiErrorCode(StreamSession *, const QString &, bool)>;
    Connect connect;
};
