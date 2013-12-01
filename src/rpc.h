#ifndef RPC_H
#define RPC_H

#include <QNetworkAccessManager>
#include <QNetworkRequest>
#include <QNetworkReply>
#include <QUrl>
#include <QSettings>
#include <QTextStream>

class Rpc : public QObject
{
    Q_OBJECT

    uint tag;
    bool requires_auth;
    QSettings * settings;
    QNetworkAccessManager * nam;
    QByteArray auth_token;
    QHash<QNetworkReply*,uint> http_tracking;
    QHash<uint,QByteArray> json_tracking;

    void tbt_everstats_request();
    void tbt_everstats_result(QVariant & arguments);

    void json_request(QString method, QVariantMap & arguments);
    void json_response(QVariant & response);

    void http_request(uint json_tag);
    void http_response(QByteArray & response);

public:
    explicit Rpc(QObject * p, QSettings * s);
    virtual ~Rpc();

signals:
    void store(QString & hashString, qlonglong downloadedEver, qlonglong uploadedEver, QString & name);

public slots:
    void poll();
    void http_finished( QNetworkReply * reply );
};

#endif // RPC_H
