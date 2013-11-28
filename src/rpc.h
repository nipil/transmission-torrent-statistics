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

    bool requires_auth;
    QSettings * settings;
    QNetworkAccessManager * nam;
    QByteArray auth_token;
    QHash<QNetworkReply*,QString> requests;

    void tbt_everstats();

    void json_request(QString method, QStringList & arguments, ulong tag);
    void json_response(QString & result, QStringList & arguments, ulong * tag);

    void json_append(QTextStream & text, QString key, QString value, bool quoted_value);
    void json_array(QString & out, QString key, QStringList items, bool quoted_items);

    void http_request(QString & request);
    void http_response(QString & response);

public:
    explicit Rpc(QObject * p, QSettings * s);

signals:
    
public slots:
    void poll();
    void http_finished( QNetworkReply * reply );
};

#endif // RPC_H
