#include "psnaccountid.h"
#include "jsonrequester.h"
#include <QJsonObject>
#include <QtEndian>
#include <QRegularExpression>

PSNAccountID::PSNAccountID(Settings *settings, QObject *parent)
    : QObject(parent), settings(settings)
{
    basicAuthHeader = JsonRequester::generateBasicAuthHeader(PSNAuth::CLIENT_ID, PSNAuth::CLIENT_SECRET);
}

void PSNAccountID::GetPsnAccountId(QString redirectCode)
{
    if (finished || !settings) return;
    QString body = QString("grant_type=authorization_code&code=%1&scope=psn:clientapp referenceDataService:countryConfig.read pushNotification:webSocket.desktop.connect sessionManager:remotePlaySession.system.update&redirect_uri=https://remoteplay.dl.playstation.net/remoteplay/redirect&").arg(QString::fromLatin1(QUrl::toPercentEncoding(redirectCode)));
    auto *requester = new JsonRequester(this);
    connect(requester, &JsonRequester::requestFinished, this, &PSNAccountID::handleAccessTokenResponse);
    connect(requester, &JsonRequester::requestError, this, &PSNAccountID::handleErrorResponse);
    requester->makePostRequest(PSNAuth::TOKEN_URL, basicAuthHeader, "application/x-www-form-urlencoded", body);
}

void PSNAccountID::handleAccessTokenResponse(const QString &, const QJsonDocument &document)
{
    if (finished || !access_token.isEmpty()) return;
    const auto object = document.object();
    const auto access = object.value("access_token").toString();
    const auto refresh = object.value("refresh_token").toString();
    const auto seconds = object.value("expires_in").toInt();
    if (access.isEmpty() || refresh.isEmpty() || seconds <= 0) {
        fail(tr("PSN returned incomplete credentials. Start a fresh sign-in."));
        return;
    }
    access_token = access;
    refresh_token = refresh;
    access_token_expiry = QDateTime::currentDateTime().addSecs(seconds).toString(settings->GetTimeFormat());
    emit Progress();
    if (finished) return;
    auto *requester = new JsonRequester(this);
    connect(requester, &JsonRequester::requestFinished, this, &PSNAccountID::handUserIDResponse);
    connect(requester, &JsonRequester::requestError, this, &PSNAccountID::handleErrorResponse);
    requester->makeGetRequest(PSNAuth::TOKEN_URL + "/" + QString::fromLatin1(QUrl::toPercentEncoding(access_token)), basicAuthHeader, "application/json");
}

void PSNAccountID::handUserIDResponse(const QString &, const QJsonDocument &document)
{
    if (finished) return;
    const auto user = document.object().value("user_id").toString();
    bool ok = false;
    const auto id = user.toULongLong(&ok);
    if (!ok || !id || user.isEmpty() || user.contains(QRegularExpression("[^0-9]")) || access_token.isEmpty()) {
        fail(tr("PSN did not return a valid Account ID. Start a fresh sign-in."));
        return;
    }
    const auto little_endian = qToLittleEndian(id);
    const auto account_id = QByteArray(reinterpret_cast<const char *>(&little_endian), sizeof(little_endian)).toBase64();
    finished = true;
    settings->SetPsnAuthToken(access_token);
    settings->SetPsnRefreshToken(refresh_token);
    settings->SetPsnAuthTokenExpiry(access_token_expiry);
    settings->SetPsnAccountId(account_id);
    settings->SetRemotePlayAsk(false);
    emit AccountIDResponse(account_id);
    emit Finished();
}

void PSNAccountID::fail(const QString &message)
{
    if (finished) return;
    finished = true;
    // Network errors can contain the authorization URL or a token in its path.
    emit AccountIDError(QString(), message);
    emit Finished();
}

void PSNAccountID::handleErrorResponse(const QString &, const QString &, const QNetworkReply::NetworkError &error)
{
    fail(error == QNetworkReply::ProtocolInvalidOperationError || error == QNetworkReply::ContentAccessDenied || error == QNetworkReply::AuthenticationRequiredError
        ? tr("The sign-in code may have expired or already been used. Start a fresh sign-in.")
        : tr("Could not reach PSN. Check your connection and try again, or start a fresh sign-in."));
}
