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
            serve(t, tokens[1]);
            t->flush();
        }
    }
}

void Web::serve(QTcpSocket * socket, QString & localPath)
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
        emit jsonList(socket);
    }
    else if (re_data.exactMatch(localPath))
    {
        bool ok;
        qDebug() << "Captured texts" << re_data.capturedTexts();
        QString hashString = re_data.cap(1);
        QString fromTime = re_data.cap(2);
        QString toTime = re_data.cap(3);
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
        if (time_from > time_to)
        {
            reply(socket,"HTTP/1.1 400 Bad Request","Error: fromTime > toTime");
            return;
        }
        emit jsonStats(socket, hashString, time_from, time_to);
    }
    else
    {
        serveFile(socket, localPath);
    }
}

void Web::serveFile(QTcpSocket * socket, QString & path)
{
    qDebug() << "Web::serveFile" << socket << path;

    QFile file( settings->value(TTS_SETTINGS_WEB_PATH).toString() + path );
    if (!file.exists())
    {
        reply(socket,"HTTP/1.1 404 Not Found","Error: not found");
        return;
    }

    if (!file.open(QIODevice::ReadOnly))
    {
        reply(socket,"HTTP/1.1 403 Forbidden",
              QString("FileError %1: %2")
              .arg(file.error())
              .arg(file.errorString()));
        return;
    }

    replyHeader(socket,"HTTP/1.1 200 OK",file.size());

    while(!file.atEnd())
    {
        QByteArray buffer = file.read(TTS_BUFFER_SIZE);
        qDebug() << "Read" << buffer.size() << "bytes from file";
        replyData(socket, buffer);
    }

    file.close();
}

void Web::reply(QTcpSocket * socket, QString http_result, QString message)
{
    qDebug() << "Web::reply" << http_result << message;

    replyHeader(socket, http_result, message.size());

    QByteArray data = message.toUtf8();
    replyData(socket, data);
}

void Web::replyHeader(QTcpSocket * socket, QString http_result, qint64 size)
{
    qDebug() << "Web::replyHeader" << http_result << size;

    QByteArray data (QString("%1\r\n"
                             "Content-Length: %2\r\n"
                             "\r\n")
                     .arg(http_result)
                     .arg(size)
                     .toUtf8());

    replyData(socket, data);
}

void Web::replyData(QTcpSocket * socket, QByteArray & buffer)
{
    qDebug() << "Web::replyData" << socket << buffer.size();

    qint64 written = 0;
    do
    {
        written = socket->write(buffer.constData() + written,
                                buffer.size() - written);
        qDebug() << "Wrote" << written << "bytes to socket";
    } while (written < buffer.size());
}
