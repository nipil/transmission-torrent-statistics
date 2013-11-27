#include <QDebug>
#include <csignal>
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

int main(int argc, char *argv[])
{
    qDebug() << "Initializing signal handling";
    if (signal(SIGINT,signal_handler) == SIG_ERR)
    {
        qDebug() << "Cannot register SIGINT";
        return -1;
    }
    if (signal(SIGTERM,signal_handler) == SIG_ERR)
    {
        qDebug() << "Cannot register SIGTERM";
        return -1;
    }
    if (signal(SIGHUP,signal_handler) == SIG_ERR)
    {
        qDebug() << "Cannot register SIGHUP";
        return -1;
    }

    qDebug() << "Initializing application";
    Tts tts(argc, argv);

    qDebug() << "Starting event loop";
    int result = tts.exec();

    qDebug() << "Event loop ended with value" << result;
    return result;
}
