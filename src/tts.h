#ifndef TTS_H
#define TTS_H

#include <QCoreApplication>
#include <QSettings>
#include <QTimer>

#include "options.h"
#include "rpc.h"
#include "dbs.h"
#include "web.h"

class Tts : public QCoreApplication
{
    Q_OBJECT
    QSettings * settings;
    QTimer * signalTimer;
    QTimer * pollingTimer;
    Rpc * rpc;
    Dbs * dbs;
    Web * web;

    Options options;

public:
    static bool reloadRequested;
    static bool exitRequested;

    explicit Tts(int &argc, char **argv);
    virtual ~Tts();

    void loadSettings();

    virtual bool notify ( QObject * receiver, QEvent * event );

signals:
    
public slots:
    void signalCheck();

};

#endif // TTS_H
