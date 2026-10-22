#pragma once

#include <chiaki/regist.h>
#include <functional>

// The native asynchronous transport boundary. State and persistence stay in QmlBackend.
struct RegistrationRequests
{
    std::function<ChiakiErrorCode(ChiakiRegist *, ChiakiLog *, const ChiakiRegistInfo *, ChiakiRegistCb, void *)> start = chiaki_regist_start;
    std::function<void(ChiakiRegist *)> stop = chiaki_regist_stop;
    std::function<void(ChiakiRegist *)> fini = chiaki_regist_fini;
};
