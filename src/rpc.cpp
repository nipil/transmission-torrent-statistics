#include <QtGlobal>
#include <QDebug>
#include <QStringList>
#include "common.h"
#include "rpc.h"

Rpc::Rpc(QObject * p, QSettings * s) :
    QObject(p),
    requires_auth(false),
    settings(s)
{
    nam = new QNetworkAccessManager(this);
    connect(nam, SIGNAL(finished(QNetworkReply*)), this, SLOT(http_finished(QNetworkReply*)));
}

void Rpc::poll()
{
    qDebug() << "RPC poll";
    tbt_everstats();
}

void Rpc::http_request(QString & query)
{
    qDebug() << "RPC request";
    qDebug() << "\n" << query << "\n";

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

    QNetworkReply* reply = nam->post(*req,query.toUtf8());
    requests.insert(reply,query);

    if (req) delete req;
}

void Rpc::http_finished( QNetworkReply * reply )
{
    qDebug() << "RPC finished";

    QString cur_request = requests.take(reply);
    if (cur_request.isEmpty())
    {
        // TODO: better reporting ?
        qDebug() << "No request found for this reply hash" << reply;
        return;
    }

    int httpcode = reply->attribute(QNetworkRequest::HttpStatusCodeAttribute).toInt();
    qDebug() << "HTTP code" << httpcode;

    switch (reply->error())
    {
    case QNetworkReply::NoError:
        // 0 = no error condition.
        // Note: When the HTTP protocol returns a redirect no error will be reported. You can check if there is a redirect with the QNetworkRequest::RedirectionTargetAttribute attribute.
        if (httpcode == 200)
        {
            QByteArray data = reply->readAll();
            QString resp = data;
            http_response(resp);
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

void Rpc::http_response(QString & response)
{
    qDebug() << "RPC response";
    qDebug() << "\n" << response << "\n";

    QString result;
    QStringList arguments;
    ulong tag;
    json_response(result, arguments, &tag);
}

void Rpc::json_array(QString & out, QString key, QStringList items, bool quoted_items)
{
    QTextStream text(&out);
    QString sep = (quoted_items) ? "\",\"" : ",";
    text << "\"" << key << "\":[";
    if (!items.isEmpty())
    {
        if (quoted_items)
            text << "\"";
        text << items.join(sep);
        if (quoted_items)
            text << "\"";
        text << "]";
    }
}

void Rpc::json_append(QTextStream & text, QString key, QString value, bool quoted_value)
{
    text << "\"" << key << "\":";
    if (quoted_value)
        text << "\"";
    text << value;
    if (quoted_value)
        text << "\"";
}

void Rpc::json_request(QString method, QStringList & arguments, ulong tag)
{
    QString json;
    QTextStream ts(&json);
    ts << "{";
    json_append(ts,"arguments", "{" + arguments.join(",") + "}",false);
    ts << ",";
    json_append(ts,"method","torrent-get",true);
    if (tag > 0)
    {
        ts << ",";
        json_append(ts,"tag",QString::number(tag),false);
    }
    ts << "}";
    http_request(json);
}

void Rpc::json_response(QString & result, QStringList & arguments, ulong * tag)
{
    /*
    {
       "arguments":{
          "torrents":[
             {
                "downloadedEver":0,
                "hashString":"26942386b3e4e69fbe8bbd462d14076d17dc0123",
                "name":"tagada",
                "uploadedEver":1229818760
             },{}
          ]
       },
       "result":"success",
       "tag":1
    }
    */
}

void Rpc::tbt_everstats()
{
    static uint tag = 1;
    QString method = "torrent-get";
    QStringList tmp;
    tmp << "hashString";
    tmp << "uploadedEver";
    tmp << "downloadedEver";
    tmp << "name";
    QString json_fields;
    json_array(json_fields, "fields", tmp, true);
    tmp.clear();
    tmp.append(json_fields);
    json_request(method, tmp, tag++);
}
