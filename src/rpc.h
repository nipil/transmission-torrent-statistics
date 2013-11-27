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
    
    void request(QString request);

signals:
    
public slots:
    void poll();
    void finished( QNetworkReply * reply );
};

#endif // RPC_H
