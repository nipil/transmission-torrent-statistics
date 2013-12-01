#ifndef TTS_H
#define TTS_H

#include <QCoreApplication>
#include <QSettings>
#include <QTimer>

#include "rpc.h"
#include "dbs.h"

class Tts : public QCoreApplication
{
    Q_OBJECT
    QSettings * settings;
    QTimer * signalTimer;
    QTimer * pollingTimer;
    Rpc * rpc;
    Dbs * dbs;

public:
    static bool reloadRequested;
    static bool exitRequested;

    explicit Tts(int &argc, char **argv);
    virtual ~Tts();

    void loadSettings();

signals:
    
public slots:
    void unixSignalCheck();

};

#endif // TTS_H
