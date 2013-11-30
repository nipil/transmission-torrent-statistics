#include <QtGlobal>
#include <QDebug>
#include <QStringList>
#include <qjson/parser.h>
#include <qjson/serializer.h>
#include "common.h"
#include "rpc.h"

Rpc::Rpc(QObject * p, QSettings * s) :
    QObject(p),
    tag(1),
    requires_auth(false),
    settings(s)
{
    nam = new QNetworkAccessManager(this);
    connect(nam, SIGNAL(finished(QNetworkReply*)), this, SLOT(http_finished(QNetworkReply*)));
}

void Rpc::poll()
{
    qDebug() << "Rpc::poll";
    tbt_everstats_request();
}

void Rpc::http_request(uint json_tag)
{
    qDebug() << "Rpc::http_request" << json_tag;
    qDebug() << "\n" << json_tracking[json_tag] << "\n";

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

    QNetworkReply* reply = nam->post(*req,json_tracking[json_tag]);
    Q_ASSERT(reply != NULL);
    qDebug() << "reply" << reply;

    http_tracking.insert(reply,json_tag);

    if (req) delete req;
}

void Rpc::http_finished( QNetworkReply * reply )
{
    qDebug() << "Rpc::http_finished" << reply;

    uint cur_request = http_tracking.take(reply);
    Q_ASSERT(cur_request > 0);
    qDebug() << "cur_request" << cur_request;

    int httpcode = reply->attribute(QNetworkRequest::HttpStatusCodeAttribute).toInt();
    qDebug() << "HTTP code" << httpcode;

    switch (reply->error())
    {
    case QNetworkReply::NoError:
        // 0 = no error condition.
        // Note: When the HTTP protocol returns a redirect no error will be reported. You can check if there is a redirect with the QNetworkRequest::RedirectionTargetAttribute attribute.
        if (httpcode == 200)
        {
            QByteArray cur_resp = reply->readAll();
            http_response(cur_resp);
        }
        break;

    case QNetworkReply::ConnectionRefusedError:
        // 1 = the remote server refused the connection (the server is not accepting requests)
        qCritical() << "Could not contact host";
        // TODO: stop polling process ?
        break;

    case QNetworkReply::AuthenticationRequiredError:
        // 204 = the remote server requires authentication to serve the content
        // but the credentials provided were not accepted (if any)
        if (httpcode == 401)
        {
            if (!requires_auth)
            {
                qDebug() << "Authentication required, retrying with credentials";
                requires_auth = true;
                http_request(cur_request);
            }
            else
            {
                qCritical() << "Authentication failed" << reply;
                // TODO: stop polling process ?
            }
        }
        break;
    case QNetworkReply::UnknownContentError:
        // 299 = an unknown error related to the remote content was detected
        if (httpcode == 409)
        {
            // reply provides the X-Transmission-Session-Id header
	        // libtransmission: token validity is 1 hour (not refreshable)
	        qDebug() << "Current X-Transmission-Session-Id" << auth_token;
            auth_token = reply->rawHeader(QByteArray("X-Transmission-Session-Id"));
            if (auth_token.isEmpty())
            {
		        qCritical() << "Expected an X-Transmission-Session-Id token, but didn't get any.";
		        // TODO: stop polling process ?
		        break;
            }
	        qDebug() << "New X-Transmission-Session-Id" << auth_token << "now redo request";
            http_request(cur_request);
        }
        break;
    default:
        qDebug() << "Unidentified Error" << reply->error();
        break;

    }

    if (reply) reply->deleteLater();
}

void Rpc::http_response(QByteArray & response)
{
    qDebug() << "Rpc::http_response" << response;

    QJson::Parser parser;
    bool ok;

    QVariant data = parser.parse(response, &ok);
    if (!ok)
    {
        qDebug() << "QJson parsing error" << parser.errorString() << "at" << parser.errorLine();
        // TODO better handling ?
        return;
    }

    json_response(data);
}

void Rpc::json_request(QString method, QVariantMap & arguments)
{
    qDebug() << "Rpc::json_request";

    QVariantMap req;
    req.insert("method",method);
    req.insert("arguments",arguments);
    req.insert("tag",tag);

    QJson::Serializer s;
    QByteArray json = s.serialize(req);
    qDebug() << json;

    json_tracking.insert(tag,json);

    http_request(tag);
    tag++;
}

void Rpc::json_response(QVariant & response)
{
    qDebug() << "Rpc::json_response";

    Q_ASSERT(response.type() == QVariant::Map);
    QVariantMap vm = response.toMap();

    qDebug() << "tag" << vm["tag"].toString();
    bool ok;
    uint cur_tag = vm["tag"].toUInt(&ok);
    if (!ok)
    {
        qDebug() << "Cannot convert tag to uint";
        // TODO better handling ?
        return;
    }

    QByteArray json_cur = json_tracking.take(cur_tag);

    QString result = vm["result"].toString();
    qDebug() << "result" << result;
    if (result != "success")
    {
        qDebug() << "json request failed : " << json_cur;
        // TODO better handling ?
        return;
    }

    tbt_everstats_result(vm["arguments"]);
}

void Rpc::tbt_everstats_request()
{
    qDebug() << "Rpc::tbt_everstats_request";

    QVariantList tmp;
    tmp << "hashString";
    tmp << "uploadedEver";
    tmp << "downloadedEver";
    tmp << "name";

    QVariantMap fields;
    fields.insert("fields",tmp);

    json_request("torrent-get", fields);
}

void Rpc::tbt_everstats_result(QVariant & arguments)
{
    qDebug() << "Rpc::tbt_everstats_result";

    Q_ASSERT(arguments.type() == QVariant::Map);
    QVariantMap am = arguments.toMap();

    Q_ASSERT(am["torrents"].type() == QVariant::List);
    QVariantList torrents = am["torrents"].toList();

    foreach (QVariant torrent, torrents) {
        bool ok;
        QVariantMap mt = torrent.toMap();

        qlonglong downloadedEver = mt["downloadedEver"].toLongLong(&ok);
        if (!ok)
            qDebug() << "cannot convert downloadedEver to longlong";

        qlonglong uploadedEver = mt["uploadedEver"].toLongLong(&ok);
        if (!ok)
            qDebug() << "cannot convert uploadedEver to longlong";

        QString hashString = mt["hashString"].toString();
        QString name = mt["name"].toString();

        qDebug() << hashString << downloadedEver << uploadedEver << name;
    }
}
