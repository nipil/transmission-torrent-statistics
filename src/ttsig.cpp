#include <QtGlobal>
#include <QDebug>
#include <csignal>
#include "common.h"
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
    qDebug() << "Initializing signal handling";
    if (signal(SIGINT,signal_handler) == SIG_ERR)
    {
        qCritical() << "Cannot register SIGINT";
        throw EXIT_SIGNAL_INITERROR;
    }
    if (signal(SIGTERM,signal_handler) == SIG_ERR)
    {
        qCritical() << "Cannot register SIGTERM";
        throw EXIT_SIGNAL_INITERROR;
    }
    if (signal(SIGHUP,signal_handler) == SIG_ERR)
    {
        qCritical() << "Cannot register SIGHUP";
        throw EXIT_SIGNAL_INITERROR;
    }
}
