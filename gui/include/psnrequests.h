#pragma once

#include "jsonrequester.h"
#include <chiaki/remote/holepunch.h>

// External request boundaries; parsing, persistence and attempt ownership stay native.
struct PsnRequests
{
    JsonRequester::ManagerFactory networkManager;
    decltype(&chiaki_holepunch_list_devices) listDevices = chiaki_holepunch_list_devices;
};
