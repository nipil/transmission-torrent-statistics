#ifndef TTS_H
#define TTS_H

#include <QCoreApplication>
#include <QSettings>
#include <QTimer>

#include "rpc.h"

class Tts : public QCoreApplication
{
    Q_OBJECT
    QSettings * settings;
    QTimer * signalTimer;
    QTimer * pollingTimer;
    Rpc * rpc;

public:
    static bool reloadRequested;
    static bool exitRequested;

    explicit Tts(int &argc, char **argv);
    void loadSettings();

signals:
    
public slots:
    void unixSignalCheck();

};

#endif // TTS_H
