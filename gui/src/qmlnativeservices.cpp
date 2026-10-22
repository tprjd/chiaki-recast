#include "qmlnativeservices.h"
#include "qmlsvgprovider.h"

QmlNativeServices::QmlNativeServices(Settings *settings, PsnRequests requests)
    : backend_(settings, nullptr, std::move(requests))
{
    engine_.addImageProvider(QStringLiteral("svg"), new QmlSvgProvider);
}
