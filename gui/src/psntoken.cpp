#include "psntoken.h"
#include "psnaccountid.h"
#include <qjsonobject.h>
#include "jsonrequester.h"

#include <QObject>
#include <QDateTime>
#include <QDebug>
#include <QPointer>

PSNToken::PSNToken(Settings *settings, QObject *parent, JsonRequester::ManagerFactory factory)
    : QObject(parent)
    , network_factory(std::move(factory))
    , settings(settings)
{
    basicAuthHeader = JsonRequester::generateBasicAuthHeader(PSNAuth::CLIENT_ID, PSNAuth::CLIENT_SECRET);
}

void PSNToken::RefreshPsnToken(QString refreshToken) {
    QString body = QString("grant_type=refresh_token&refresh_token=%1&scope=psn:clientapp referenceDataService:countryConfig.read pushNotification:webSocket.desktop.connect sessionManager:remotePlaySession.system.update&redirect_uri=https://remoteplay.dl.playstation.net/remoteplay/redirect&").arg(QString::fromLatin1(QUrl::toPercentEncoding(refreshToken)));
    JsonRequester* requester = new JsonRequester(this, network_factory);
    connect(requester, &JsonRequester::requestFinished, this, &PSNToken::handleAccessTokenResponse);
    connect(requester, &JsonRequester::requestError, this, &PSNToken::handleErrorResponse);
    requester->makePostRequest(PSNAuth::TOKEN_URL, basicAuthHeader, "application/x-www-form-urlencoded", std::move(body));
}

void PSNToken::handleAccessTokenResponse(const QString&, const QJsonDocument& jsonDocument) {
    QJsonObject object = jsonDocument.object();
    QString access_token = object.value("access_token").toString();
    QString refresh_token = object.value("refresh_token").toString();
    QDateTime currentTime = QDateTime::currentDateTime();
    auto secondsLeft = object.value("expires_in").toInt();
    if (access_token.trimmed().isEmpty() || refresh_token.trimmed().isEmpty() || secondsLeft <= 0) {
        const QPointer<PSNToken> owned(this);
        emit PSNTokenError(tr("PSN returned incomplete credentials. Sign in again."));
        if (owned) emit Finished();
        return;
    }
    QDateTime expiry = currentTime.addSecs(secondsLeft);
    QString access_token_expiry = expiry.toString(settings->GetTimeFormat());
    settings->SetPsnAuthToken(std::move(access_token));
    settings->SetPsnRefreshToken(std::move(refresh_token));
    settings->SetPsnAuthTokenExpiry(std::move(access_token_expiry));
    const QPointer<PSNToken> owned(this);
    emit PSNTokenSuccess();
    if (owned) emit Finished();
}

void PSNToken::handleErrorResponse(const QString&, const QString&, const QNetworkReply::NetworkError& err) {
    const bool unauthorized = err == QNetworkReply::ProtocolInvalidOperationError
        || err == QNetworkReply::ContentAccessDenied || err == QNetworkReply::AuthenticationRequiredError;
    // Reply URLs and error strings can contain authentication credentials.
    const QPointer<PSNToken> owned(this);
    emit PSNTokenError(unauthorized ? tr("PSN credentials expired. Sign in again.")
        : tr("Could not reach PSN. Check your connection and try again."));
    if (!owned) return;
    if(unauthorized)
        emit UnauthorizedError();
    if (owned) emit Finished();
}
