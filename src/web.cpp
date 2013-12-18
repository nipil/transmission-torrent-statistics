#include <QtGlobal>
#include <QDebug>
#include <QTcpSocket>
#include <QRegExp>
#include <QDir>
#include <QDateTime>
#include "common.h"
#include "logger.h"
#include "web.h"

Web::Web(QObject *parent, QSettings * s) :
    QTcpServer(parent),
    settings(s)
{
    qDebug() << "Web::Web";

    if (!connect(this,SIGNAL(newConnection()),
                 this,SLOT(newConnection())))
    {
        Logger::Error() << "Could not connect QT signal in file" << __FILE__ << "on line" << __LINE__;
        throw EXIT_QTCONNECT_ERROR;
    }

    int port = settings->value(TTS_SETTINGS_WEB_PORT).toInt();
    Logger::Info() << "Web server listening on any address (IPv4+IPv6) on tcp port" << port;

    // AnyIPv6 listens on both IPv4/IPv6 addresses
    bool ok = listen(QHostAddress::AnyIPv6, port);
    if (!ok || !isListening())
    {
        Logger::Error() << "Could not create listening web server" << errorString() << serverError();
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
        Logger::Debug(this) << "Received connection" << t << "from" << t->peerAddress().toString() << " using source port" << t->peerPort();

        if (!connect(t, SIGNAL(readyRead()),
                     this,SLOT(readyRead())))
        {
            Logger::Error() << "Could not connect QT signal in file" << __FILE__ << "on line" << __LINE__;
            throw EXIT_QTCONNECT_ERROR;
        }

        if (!connect(t, SIGNAL(disconnected()),
                     this,SLOT(disconnected())))
        {
            Logger::Error() << "Could not connect QT signal in file" << __FILE__ << "on line" << __LINE__;
            throw EXIT_QTCONNECT_ERROR;
        }
    }
}

void Web::disconnected()
{
    qDebug() << "Web::disconnected";

    QTcpSocket * t = dynamic_cast<QTcpSocket *>(sender());
    Q_CHECK_PTR(t);

    if (t) t->deleteLater();
}

void Web::readyRead()
{
    qDebug() << "Web::readyRead";

    QTcpSocket * t = dynamic_cast<QTcpSocket *>(sender());
    Q_CHECK_PTR(t);

    /* based on http://doc.qt.digia.com/solutions/4/qtservice/qtservice-example-server.html */

    while (t->canReadLine())
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
    QRegExp re_list("/json/list[/]?");
    Q_ASSERT(re_list.isValid() == true);

    // RPC: request torrent stats (up/down total)
    // url /hash/fromTime/toTime
    QRegExp re_data("/json/([0-9a-fA-F]+)/([0-9]+)/([0-9]+)[/]?");
    Q_ASSERT(re_data.isValid() == true);

    Logger::Debug(this) << "Web request from" << socket << "asking for" << localPath;

    if (re_list.exactMatch(localPath))
    {
        QByteArray result;
        emit jsonList(result);
        qDebug() << "jsonList" << result;
        Logger::Debug(this) << "Serving JSON LIST to" << socket;
        replyHeader(socket,"HTTP/1.1 200 OK",result.size());
        replyData(socket, result);
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
            Logger::Error() << "Cannot convert fromTime" << fromTime << "to uint";
            throw EXIT_WEB_CONVERT_ERROR;
        }
        uint time_to = toTime.toUInt(&ok);
        if (!ok)
        {
            Logger::Error() << "Cannot convert toTime" << toTime << "to uint";
            throw EXIT_WEB_CONVERT_ERROR;
        }
        if (time_from > time_to)
        {
            reply(socket,"HTTP/1.1 400 Bad Request","Error: fromTime > toTime");
            return;
        }
        QByteArray result;
        emit jsonStats(result, hashString, time_from, time_to);
        qDebug() << "jsonStats" << result;
        Logger::Debug(this) << "Serving JSON STATS to" << socket << "for torrent" << hashString << "from time" << time_from << "to time" << time_to;
        replyHeader(socket,"HTTP/1.1 200 OK",result.size());
        replyData(socket, result);
    }
    else
    {
        serveFile(socket, localPath);
    }
}

void Web::serveFile(QTcpSocket * socket, QString & path)
{
    qDebug() << "Web::serveFile" << socket << path;

    if (path == "/") path = "/index.html";

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

    Logger::Debug(this) << "Serving local file to" << socket << "totaling" << file.size() << "bytes";

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
