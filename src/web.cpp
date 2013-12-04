#include <QtGlobal>
#include <QDebug>
#include <QTcpSocket>
#include <QRegExp>
#include <QDir>
#include "common.h"

#include <QDateTime>

#include "web.h"

Web::Web(QObject *parent, QSettings * s) :
    QTcpServer(parent),
    settings(s)
{
    qDebug() << "Web::Web";

    bool c1 = connect(this,SIGNAL(newConnection()),this,SLOT(newConnection()));
    Q_ASSERT(c1 == true);

    int port = settings->value(TTS_SETTINGS_WEB_PORT).toInt();
    qDebug() << "Web server listening on any adress, on tcp port" << port;

    // AnyIPv6 listens on both IPv4/IPv6 addresses
    bool ok = listen(QHostAddress::AnyIPv6, port);
    if (!ok || !isListening())
    {
        qCritical() << "Could not create listening web server" << errorString() << serverError();
        throw EXIT_WEB_LISTEN_ERROR;
    }
}

Web::~Web()
{
    qDebug() << "Web::~Web";

    close(); // stops listening connection
}

void Web::newConnection()
{
    QTcpSocket * t = QTcpServer::nextPendingConnection();

    qDebug() << "Web::newConnection" << t;

    if (t && t->isValid() && t->isOpen())
    {
        qDebug() << "Received connection" << t << "from" << t->peerAddress() << "port" << t->peerPort();

        bool c1 = connect(t,SIGNAL(readyRead()),this,SLOT(readyRead()));
        Q_ASSERT(c1 == true);

        bool c2 = connect(t,SIGNAL(disconnected()),this,SLOT(disconnected()));
        Q_ASSERT(c2 == true);
    }
}

void Web::disconnected()
{
    qDebug() << "Web::disconnected";

    QTcpSocket * t = dynamic_cast<QTcpSocket *>(sender());
    Q_ASSERT(t != NULL);

    if (t) t->deleteLater();
}

void Web::readyRead()
{
    qDebug() << "Web::readyRead";

    QTcpSocket * t = dynamic_cast<QTcpSocket *>(sender());
    Q_ASSERT(t != NULL);

    /* based on http://doc.qt.digia.com/solutions/4/qtservice/qtservice-example-server.html */

    if (t->canReadLine())
    {
        QString buff = t->readLine();
        qDebug() << "Line" << buff;

        QStringList tokens = buff.split(QRegExp("[ \r\n][ \r\n]*"));
        qDebug() << "Tokens" << tokens;

        if (tokens[0].toLower() == "get")
        {
            QTextStream output(t);
            output.setAutoDetectUnicode(true); // this is the default
            serve(output, tokens[1]);
        }
    }
}

void Web::serve(QTextStream & output, QString & localPath)
{
    qDebug() << "Web::serve" << localPath;

    // RPC: request list of known torrents
    // url /list
    QRegExp re_list("/list[/]?");
    Q_ASSERT(re_list.isValid() == true);

    // RPC: request torrent stats (up/down total)
    // url /hash/fromTime/toTime
    QRegExp re_data("/([0-9a-fA-F]+)/([0-9]+)/([0-9]+)[/]?");
    Q_ASSERT(re_data.isValid() == true);

    if (re_list.exactMatch(localPath))
    {
        serveList(output);
    }
    else if (re_data.exactMatch(localPath))
    {
        bool ok;
        QString hashString = re_data.cap(0);
        QString fromTime = re_data.cap(1);
        QString toTime = re_data.cap(2);
        uint time_from = fromTime.toUInt(&ok);
        if (!ok)
        {
            qCritical() << "Cannot convert fromTime" << fromTime << "to uint";
            throw EXIT_WEB_CONVERT_ERROR;
        }
        uint time_to = toTime.toUInt(&ok);
        if (!ok)
        {
            qCritical() << "Cannot convert toTime" << toTime << "to uint";
            throw EXIT_WEB_CONVERT_ERROR;
        }
        if (fromTime > toTime)
        {
            // TODO: return html error with message
            return;
        }
        serveStats(output, hashString, time_from, time_to);
    }
    else
    {
        serveFile(output, localPath);
    }
}

void Web::serveFile(QTextStream & output, QString & path)
{
    qDebug() << "Web::serveFile" << path;

    QDir base(settings->value(TTS_SETTINGS_WEB_PATH).toString());
    qDebug() << "Serving static page from" << base;

}

void Web::serveList(QTextStream & output)
{
    qDebug() << "Web::serveList";

}

void Web::serveStats(QTextStream & output, QString & torrent, uint time_min, uint time_max)
{
    qDebug() << "Web::serveStats" << torrent << time_min << time_max;

}
