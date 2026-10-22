#include "qmlnativeservices.h"
#include "qmlsvgprovider.h"

QmlNativeServices::QmlNativeServices(Settings *settings, PsnRequests requests, QmlSettings::ProcessLauncher launcher, RegistrationRequests registration)
    : backend_(settings, nullptr, std::move(requests), std::move(launcher), std::move(registration))
{
    engine_.addImageProvider(QStringLiteral("svg"), new QmlSvgProvider);
}
