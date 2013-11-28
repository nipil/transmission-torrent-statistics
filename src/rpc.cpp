#include <QtGlobal>
#include <QDebug>
#include "common.h"
#include "rpc.h"

Rpc::Rpc(QObject * p, QSettings * s) :
    QObject(p),
    requires_auth(false),
    settings(s)
{
    nam = new QNetworkAccessManager(this);
    connect(nam, SIGNAL(finished(QNetworkReply*)), this, SLOT(finished(QNetworkReply*)));
}

void Rpc::poll()
{
    qDebug() << "RPC poll";
    request("pouet");
}

void Rpc::request(QString request)
{
    qDebug() << "RPC request";

    QUrl url;
    url.setHost(settings->value(TTS_SETTINGS_RPC_HOST).toString());
    url.setPort(settings->value(TTS_SETTINGS_RPC_PORT).toInt());
    if (settings->value(TTS_SETTINGS_RPC_SSL).toBool())
        url.setScheme("https");
    else
        url.setScheme("http");
    url.setPath("/transmission/rpc/");
    qDebug() << "url" << url;

    QNetworkRequest * req = new QNetworkRequest(url);

    req->setHeader(QNetworkRequest::ContentTypeHeader, "application/x-www-form-urlencoded");
    QString ua = TTS_APP_NAME + QString("/") + TTS_APP_VERSION;
    req->setRawHeader(QByteArray("User-Agent"), QByteArray(ua.toUtf8()));

    if (requires_auth)
    {
        QString username = settings->value(TTS_SETTINGS_RPC_USER).toString();
        QString password = settings->value(TTS_SETTINGS_RPC_PASSWORD).toString();
        if (!username.isEmpty() && !password.isEmpty())
        {
            QByteArray basic_auth = "Basic " + QByteArray(QString(username + ":" + password).toUtf8()).toBase64();
            qDebug() << "With authentication: username" << username << "password" << password << "basic_auth" << basic_auth;
            req->setRawHeader(QByteArray("Authorization"),basic_auth);
        }
    }
    else
        qDebug() << "Without authentication";

    if (!auth_token.isEmpty())
    {
        qDebug() << "auth_token" << auth_token;
        req->setRawHeader(QByteArray("X-Transmission-Session-Id"),auth_token);
    }

    QNetworkReply* reply = nam->post(*req,request.toUtf8());
    requests.insert(reply,request);

    if (req) delete req;
}

void Rpc::finished( QNetworkReply * reply )
{
    qDebug() << "RPC finished";

    QString cur_request = requests.take(reply);
    if (cur_request.isEmpty())
    {
        // TODO: better reporting ?
        qDebug() << "No request found for this reply hash" << reply;
        return;
    }

    switch (reply->error())
    {
    case QNetworkReply::NoError:
        // 0 = no error condition.
        // Note: When the HTTP protocol returns a redirect no error will be reported. You can check if there is a redirect with the QNetworkRequest::RedirectionTargetAttribute attribute.
        break;

    case QNetworkReply::ConnectionRefusedError:
        // 1 = the remote server refused the connection (the server is not accepting requests)
        qCritical() << "Could not contact host";
        // TODO: stop polling process ?
        break;

    case QNetworkReply::AuthenticationRequiredError:
        // 204 = the remote server requires authentication to serve the content
        // but the credentials provided were not accepted (if any)
        // corresponds to the HTTP 401 error
        if (!requires_auth)
        {
            qDebug() << "Authentication required, retrying with credentials";
            requires_auth = true;
            request(cur_request);
        }
        else
        {
            qCritical() << "Authentication failed" << reply;
            // TODO: stop polling process ?
        }
        break;

    case QNetworkReply::UnknownContentError:
        // 299 = an unknown error related to the remote content was detected
        // corresponds to the HTTP 409 Conflict
        // reply provides the X-Transmission-Session-Id header
        // libtransmission: token validity is 1 hour (not refreshed)
        qDebug() << "Current X-Transmission-Session-Id" << auth_token;
        auth_token = reply->rawHeader(QByteArray("X-Transmission-Session-Id"));
        if (auth_token.isEmpty())
        {
            qCritical() << "Expected an X-Transmission-Session-Id token, but didn't get any." << reply;
            // TODO: stop polling process ?
            break;
        }
        qDebug() << "New X-Transmission-Session-Id" << auth_token << "now redo request";
        request(cur_request);
        break;

    default:
        qDebug() << "Unidentified Error" << reply->error();
        break;

    }

    if (reply) reply->deleteLater();
}
