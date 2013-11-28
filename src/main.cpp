#include <QtGlobal>
#include <QDebug>
#include <csignal>
#include "common.h"
#include "tts.h"

/*
 * Logging information :
 * debugging is done to stderr via qDebug()
 * normal logging is done via qWarning() which can be recovered
 * error is done via qCritical() and should stop the process
 *
 * Debuging can be disabled at compile time via
 * DEFINES += QT_NO_DEBUG_OUTPUT in .pro
 */

void signal_handler(int signo)
{
    if (signo == SIGINT)
    {
        qDebug() << "SIGINT received";
        Tts::exitRequested = true;
    }
    if (signo == SIGTERM)
    {
        qDebug() << "SIGTERM received";
        Tts::exitRequested = true;
    }
    if (signo == SIGHUP)
    {
        qDebug() << "SIGHUP received";
        Tts::reloadRequested = true;
    }
}

bool signal_init()
{
    qDebug() << "Initializing signal handling";
    if (signal(SIGINT,signal_handler) == SIG_ERR)
    {
        qCritical() << "Cannot register SIGINT";
        return false;
    }
    if (signal(SIGTERM,signal_handler) == SIG_ERR)
    {
        qCritical() << "Cannot register SIGTERM";
        return false;
    }
    if (signal(SIGHUP,signal_handler) == SIG_ERR)
    {
        qCritical() << "Cannot register SIGHUP";
        return false;
    }
    return true;
}

int main(int argc, char *argv[])
{
    if (!signal_init())
        return EXIT_SIGNAL_INITERROR;

    qDebug() << "Initializing application";
    Tts tts(argc, argv);

    qDebug() << "Starting event loop";
    int result = tts.exec();

    qDebug() << "Event loop ended with value" << result;
    return result;
}
