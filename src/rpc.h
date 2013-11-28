#ifndef RPC_H
#define RPC_H

#include <QNetworkAccessManager>
#include <QNetworkRequest>
#include <QNetworkReply>
#include <QUrl>
#include <QSettings>

class Rpc : public QObject
{
    Q_OBJECT

    bool requires_auth;
    QSettings * settings;
    QNetworkAccessManager * nam;
    QByteArray auth_token;
    QHash<QNetworkReply*,QString> requests;

public:
    explicit Rpc(QObject * p, QSettings * s);
    
    void http_request(QString & request);
    void http_response(QString & response);

    void json_request(QString method, QStringList & arguments, unsigned int tag = 0);
    void json_response(QString & result, QStringList & arguments, unsigned int * tag = NULL);

signals:
    
public slots:
    void poll();
    void http_finished( QNetworkReply * reply );
};

#endif // RPC_H
