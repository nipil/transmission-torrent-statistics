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
    QHash<QNetworkReply*,QByteArray> requests;

    void tbt_everstats();

    void json_request(QString method, QVariantMap & arguments);
    void json_response(QVariant & response);

    void http_request(QByteArray & request);
    void http_response(QByteArray & response);

public:
    explicit Rpc(QObject * p, QSettings * s);

signals:
    
public slots:
    void poll();
    void http_finished( QNetworkReply * reply );
};

#endif // RPC_H
