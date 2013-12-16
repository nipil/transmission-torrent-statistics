#include <QtGlobal>
#include <QDebug>

#include "logger.h"

Logger Logger::TTSLOG;
QTextStream Logger::strout(stdout,QIODevice::WriteOnly);

bool Logger::show_qt_debug = true;
bool Logger::show_qt_warning = true;
bool Logger::show_qt_critical = true;
bool Logger::show_qt_fatal = true;

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
            fprintf(stderr, "Debug: %s\n", msg);
        break;
    case QtWarningMsg:
        if (show_qt_warning)
            fprintf(stderr, "Warning: %s\n", msg);
        break;
    case QtCriticalMsg:
        if (show_qt_critical)
            fprintf(stderr, "Critical: %s\n", msg);
        break;
    case QtFatalMsg:
        if (show_qt_fatal)
            fprintf(stderr, "Fatal: %s\n", msg);
        abort();
    }
}

Logger::Logger()
{
    qDebug() << "Logger::Logger";

    qInstallMsgHandler(loggerQtMessageOutput);
}

Logger::~Logger()
{
    qDebug() << "Logger::~Logger";
}
