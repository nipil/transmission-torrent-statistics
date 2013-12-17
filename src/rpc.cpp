#include <QtGlobal>
#include <QDebug>
#include <QStringList>
#include <QDateTime>
#include <qjson/parser.h>
#include <qjson/serializer.h>
#include "common.h"
#include "logger.h"
#include "rpc.h"

Rpc::Rpc(QObject * p, QSettings * s) :
    QObject(p),
    tag(1),
    requires_auth(false),
    settings(s)
{
    qDebug() << "Rpc::Rpc";

    nam = new QNetworkAccessManager(this);
    Q_CHECK_PTR(nam);

    bool r = connect(nam, SIGNAL(finished(QNetworkReply*)), this, SLOT(http_finished(QNetworkReply*)));
    Q_ASSERT(r == true);
}

Rpc::~Rpc()
{
    qDebug() << "Rpc::~Rpc";

    if (nam)
        delete nam;
}

void Rpc::poll()
{
    qDebug() << "Rpc::poll" << http_tracking.size() << json_tracking.size();

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

    QNetworkRequest req(url);

    req.setHeader(QNetworkRequest::ContentTypeHeader, "application/x-www-form-urlencoded");
    QString ua = TTS_APP_NAME + QString("/") + TTS_APP_VERSION;
    req.setRawHeader(QByteArray("User-Agent"), QByteArray(ua.toUtf8()));

    if (requires_auth)
    {
        QString username = settings->value(TTS_SETTINGS_RPC_USER).toString();
        QString password = settings->value(TTS_SETTINGS_RPC_PASSWORD).toString();
        if (!username.isEmpty() && !password.isEmpty())
        {
            QByteArray basic_auth = "Basic " + QByteArray(QString(username + ":" + password).toUtf8()).toBase64();
            qDebug() << "With authentication: username" << username << "password" << password << "basic_auth" << basic_auth;
            req.setRawHeader(QByteArray("Authorization"),basic_auth);
        }
    }
    else
        qDebug() << "Without authentication";

    if (!auth_token.isEmpty())
    {
        qDebug() << "auth_token" << auth_token;
        req.setRawHeader(QByteArray("X-Transmission-Session-Id"),auth_token);
    }

    QNetworkReply* reply = nam->post(req,json_tracking[json_tag]);
    Q_CHECK_PTR(reply);
    qDebug() << "reply" << reply;

    http_tracking.insert(reply,json_tag);
}

void Rpc::http_finished( QNetworkReply * reply )
{
    qDebug() << "Rpc::http_finished" << reply;

    Q_ASSERT(http_tracking.contains(reply));
    uint cur_request = http_tracking.take(reply);
    qDebug() << "cur_request" << cur_request;

    int httpcode = reply->attribute(QNetworkRequest::HttpStatusCodeAttribute).toInt();
    qDebug() << "HTTP code" << httpcode;

    switch (reply->error())
    {
    case QNetworkReply::NoError:
        if (httpcode == 200)
        {
            QByteArray cur_resp = reply->readAll();
            http_response(cur_resp);
            break;
        }
        Logger::Warn() << "RPC didn't fail but returned unhandled HTTP code" << httpcode;
        break;

    case QNetworkReply::ConnectionRefusedError:
        Logger::Warn() << "Could not contact transmission via HTTP RPC";
        break;

    case QNetworkReply::AuthenticationRequiredError:
        if (httpcode == 401)
        {
            if (!requires_auth)
            {
                Logger::Info() << "Authentication required, retrying with credentials";
                requires_auth = true;
                http_request(cur_request);
            }
            else
            {
                Logger::Error() << "Authentication failed" << reply;
                throw EXIT_RPC_AUTH_FAILED;
            }
        }
        break;
    case QNetworkReply::UnknownContentError:
        if (httpcode == 409)
        {
            // reply provides the X-Transmission-Session-Id header
	        // libtransmission: token validity is 1 hour (not refreshable)
	        qDebug() << "Current X-Transmission-Session-Id" << auth_token;
            auth_token = reply->rawHeader(QByteArray("X-Transmission-Session-Id"));
            if (auth_token.isEmpty())
            {
                Logger::Error() << "Expected an X-Transmission-Session-Id token, but didn't get any.";
                throw EXIT_RPC_NO_TOKEN;
            }
            qDebug() << "New X-Transmission-Session-Id" << auth_token << "now redo request";
            Logger::Debug(this) << "Received X-Transmission-Session-Id" << auth_token;
            http_request(cur_request);
        }
        break;
    default:
        Logger::Error() << "Unidentified HTTP Error" << reply->error();
        throw EXIT_RPC_HTTP_ERROR;
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
        Logger::Error() << "QJson parsing error" << parser.errorString() << "at" << parser.errorLine();
        throw EXIT_JSON_PARSING_ERROR;
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
        Logger::Error() << "Cannot convert tag" << vm["tag"].toString() << "to uint";
        throw EXIT_JSON_CONVERT_ERROR;
    }

    Q_ASSERT(json_tracking.contains(cur_tag));
    QByteArray json_cur = json_tracking.take(cur_tag);

    QString result = vm["result"].toString();
    qDebug() << "result" << result;
    if (result != "success")
    {
        Logger::Error() << "Json request failed : " << json_cur;
        throw EXIT_RPC_FAILED;
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

    QDateTime t_start = QDateTime::currentDateTime();

    Q_ASSERT(arguments.type() == QVariant::Map);
    QVariantMap am = arguments.toMap();

    Q_ASSERT(am["torrents"].type() == QVariant::List);
    QVariantList torrents = am["torrents"].toList();

    foreach (QVariant torrent, torrents) {
        bool ok;
        QVariantMap mt = torrent.toMap();

        qlonglong downloadedEver = mt["downloadedEver"].toLongLong(&ok);
        if (!ok)
        {
            Logger::Error() << "Cannot convert downloadedEver" << mt["downloadedEver"].toString() << "to longlong";
            throw EXIT_JSON_CONVERT_ERROR;
        }

        qlonglong uploadedEver = mt["uploadedEver"].toLongLong(&ok);
        if (!ok)
        {
            Logger::Error() << "Cannot convert uploadedEver" << mt["uploadedEver"].toString() << "to longlong";
            throw EXIT_JSON_CONVERT_ERROR;
        }

        QString hashString = mt["hashString"].toString();
        QString name = mt["name"].toString();

        qDebug() << hashString << downloadedEver << uploadedEver << name;
        emit store(hashString, downloadedEver, uploadedEver, name, t_start.toTime_t());
    }

    QDateTime t_end = QDateTime::currentDateTime();
    Logger::Debug(this) << "DB Storage duration" << t_start.msecsTo(t_end) << "ms";
}
