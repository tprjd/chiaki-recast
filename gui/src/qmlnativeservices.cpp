#include "qmlnativeservices.h"
#include "qmlsvgprovider.h"

QmlNativeServices::QmlNativeServices(Settings *settings, PsnRequests requests, QmlSettings::ProcessLauncher launcher)
    : backend_(settings, nullptr, std::move(requests), std::move(launcher))
{
    engine_.addImageProvider(QStringLiteral("svg"), new QmlSvgProvider);
}
