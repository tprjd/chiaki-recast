#include "qmlnativeservices.h"
#include "qmlsvgprovider.h"

QmlNativeServices::QmlNativeServices(Settings *settings)
    : backend_(settings)
{
    engine_.addImageProvider(QStringLiteral("svg"), new QmlSvgProvider);
}
