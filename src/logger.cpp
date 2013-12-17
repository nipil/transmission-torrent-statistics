#include <QtGlobal>
#include <QDebug>

#include "logger.h"

// ensures custom QtDebug message handler is initialized on startup
QtMsgHandler Logger::original_handler = qInstallMsgHandler(loggerQtMessageOutput);

bool Logger::show_qt_debug = true;
bool Logger::show_qt_warning = true;
bool Logger::show_qt_critical = true;
bool Logger::show_qt_fatal = true;

uint Logger::global_max_level = Logger::LOG_INFO;

void Logger::showQtDebug(bool show)
{
    qDebug() << "Logger::showQtDebug" << show;

    show_qt_debug = show;
}

void Logger::showQtWarning(bool show)
{
    qDebug() << "Logger::showQtWarning" << show;

    show_qt_warning = show;
}

void Logger::showQtCritical(bool show)
{
    qDebug() << "Logger::showQtCritical" << show;

    show_qt_critical = show;
}

void Logger::showQtFatal(bool show)
{
    qDebug() << "Logger::showQtFatal" << show;

    show_qt_fatal = show;
}

void Logger::loggerQtMessageOutput(QtMsgType type, const char *msg)
{
    switch (type) {
    case QtDebugMsg:
        if (show_qt_debug)
            fprintf(stderr, "QtDebug: %s\n", msg);
        break;
    case QtWarningMsg:
        if (show_qt_warning)
            fprintf(stderr, "QtWarning: %s\n", msg);
        break;
    case QtCriticalMsg:
        if (show_qt_critical)
            fprintf(stderr, "QtCritical: %s\n", msg);
        break;
    case QtFatalMsg:
        if (show_qt_fatal)
            fprintf(stderr, "QtFatal: %s\n", msg);
        abort();
    }
}

Logger::Logger(QString header, LOG_LEVEL level, void * p) :
    out(stdout,QIODevice::WriteOnly),
    lvl(level)
{
    if (lvl <= global_max_level)
    {
        out << QDateTime::currentDateTime().toString(Qt::ISODate) << " " << header;
        if (p)
            out << " " << p;
    }
}

Logger::Logger(const Logger & other) :
    out(stdout,QIODevice::WriteOnly),
    lvl( other.lvl )
{
}

Logger::~Logger()
{
    if (lvl <= global_max_level)
        out << endl;
}

Logger Logger::Debug(void * p)
{
    if (p)
        return Logger("Debug",LOG_DEBUG,p);
    else
        return Logger("Debug",LOG_DEBUG);
}

Logger Logger::Info()
{
    return Logger("Info",LOG_INFO);
}

Logger Logger::Warn()
{
    return Logger("Warn",LOG_WARN);
}

Logger Logger::Error()
{
    return Logger("Error",LOG_ERROR);
}

Logger Logger::Verbose()
{
    return Logger("Verbose", LOG_VERBOSE);
}

void Logger::setMaxLevel(uint level)
{
    qDebug() << "Logger::setMaxLevel" << level;

    global_max_level = level;
}
