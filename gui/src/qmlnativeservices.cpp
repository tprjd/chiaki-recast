#include "qmlnativeservices.h"
#include "qmlsvgprovider.h"

QmlNativeServices::QmlNativeServices(Settings *settings, PsnRequests requests, QmlSettings::ProcessLauncher launcher, DiscoveryRequests discovery)
    : backend_(settings, nullptr, std::move(requests), std::move(launcher), std::move(discovery))
{
    engine_.addImageProvider(QStringLiteral("svg"), new QmlSvgProvider);
}
