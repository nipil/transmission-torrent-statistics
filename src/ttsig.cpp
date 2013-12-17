#include <QtGlobal>
#include <QDebug>
#include <csignal>
#include "common.h"
#include "logger.h"
#include "tts.h"

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

void signal_init()
{
    Logger::Debug() << "Initializing signal handling";
    if (signal(SIGINT,signal_handler) == SIG_ERR)
    {
        Logger::Error() << "Cannot register SIGINT";
        throw EXIT_SIGNAL_INIT_ERROR;
    }
    if (signal(SIGTERM,signal_handler) == SIG_ERR)
    {
        Logger::Error() << "Cannot register SIGTERM";
        throw EXIT_SIGNAL_INIT_ERROR;
    }
    if (signal(SIGHUP,signal_handler) == SIG_ERR)
    {
        Logger::Error() << "Cannot register SIGHUP";
        throw EXIT_SIGNAL_INIT_ERROR;
    }
}
