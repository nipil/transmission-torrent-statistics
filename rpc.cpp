#include <QDebug>
#include "common.h"
#include "rpc.h"

Rpc::Rpc(QObject * p, QSettings * s) :
    QObject(p),
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
    url.setPath("/transmission/rpc");
    qDebug() << "url" << url;

    QNetworkRequest * req = new QNetworkRequest(url);

    QString username = settings->value(TTS_SETTINGS_RPC_USER).toString();
    QString password = settings->value(TTS_SETTINGS_RPC_PASSWORD).toString();
    if (!username.isEmpty() && !password.isEmpty())
    {
        QByteArray basic_auth = QByteArray(QString(username + ":" + password).toUtf8()).toBase64();
        qDebug() << "username" << username << "password" << password << "basic_auth" << basic_auth;
        req->setRawHeader(QByteArray("Authorization"),basic_auth);
    }

    if (!auth_token.isEmpty())
    {
        qDebug() << "auth_token" << auth_token;
        req->setRawHeader(QByteArray("X-Transmission-Session-Id"),auth_token);
    }

    QNetworkReply* reply = nam->post(*req,request.toUtf8());

    if (req) delete req;
}

void Rpc::finished( QNetworkReply * reply )
{
    qDebug() << "RPC finished";

    if (reply->error() != QNetworkReply::NoError)
        qDebug() << "Error" << reply->error();

    if (reply) reply->deleteLater();
}
