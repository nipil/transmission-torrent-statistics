#ifndef WEB_H
#define WEB_H

#include <QTcpServer>
#include <QSettings>

/* based on http://doc.qt.digia.com/solutions/4/qtservice/qtservice-example-server.html */

class Web : public QTcpServer
{
    Q_OBJECT

    QSettings * settings;

    void serve(QTcpSocket * socket, QString & localPath);
    void serveFile(QTcpSocket * socket, QString & path);
    void reply(QTcpSocket * socket, QString http_result, QString message);

public:
    explicit Web(QObject *parent, QSettings * s);
    virtual ~Web();

signals:
    void jsonList(QByteArray & out);
    void jsonStats(QByteArray & out, QString & hashString, uint time_min, uint time_max);

public slots:
     void newConnection();
     void readyRead();
     void disconnected();
};

#endif // WEB_H
