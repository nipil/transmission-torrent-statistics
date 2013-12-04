#ifndef WEB_H
#define WEB_H

#include <QTcpServer>
#include <QSettings>

/* based on http://doc.qt.digia.com/solutions/4/qtservice/qtservice-example-server.html */

class Web : public QTcpServer
{
    Q_OBJECT

    QSettings * settings;

    void serve(QTextStream & output, QString & localPath);
    void serveFile(QTextStream & output, QString & path);
    void serveList(QTextStream & output);
    void serveStats(QTextStream & output, QString & torrent, uint time_min, uint time_max);

public:
    explicit Web(QObject *parent, QSettings * s);
    virtual ~Web();

signals:
    
public slots:
     void newConnection();
     void readyRead();
     void disconnected();
};

#endif // WEB_H
