#include <QtGlobal>
#include <QDebug>
#include "common.h"
#include "tts.h"

bool Tts::reloadRequested = false;
bool Tts::exitRequested = false;

Tts::Tts(int &argc, char **argv) :
    QCoreApplication(argc,argv)
{
    this->setOrganizationName(TTS_APP_ORGNAME);
    this->setOrganizationDomain(TTS_APP_ORGDOMAIN);
    this->setApplicationName(TTS_APP_NAME);
    this->setApplicationVersion(TTS_APP_VERSION);

    settings = new QSettings(this);
    loadSettings();

    rpc = new Rpc(this,settings);

    signalTimer = new QTimer(this);
    connect(signalTimer,SIGNAL(timeout()),this,SLOT(unixSignalCheck()));
    signalTimer->start(200);

    pollingTimer = new QTimer(this);
    pollingTimer->setSingleShot(true); // debug
    connect(pollingTimer,SIGNAL(timeout()),rpc,SLOT(poll()));
    pollingTimer->start(1000);
}

void Tts::loadSettings()
{
    qDebug() << "Loading Settings";

    QMap<QString,QVariant> params;
    params.insert(TTS_SETTINGS_RPC_HOST,QString("localhost"));
    params.insert(TTS_SETTINGS_RPC_PORT, int(9091));
    params.insert(TTS_SETTINGS_RPC_USER, QString(""));
    params.insert(TTS_SETTINGS_RPC_PASSWORD, QString(""));
    params.insert(TTS_SETTINGS_RPC_SSL, bool(false));

    QMap<QString,QVariant>::const_iterator i;
    for (i=params.begin();i!=params.end();i++)
    {
        QVariant var = settings->value( i.key(), i.value() );
        settings->setValue( i.key(), var );
    }
}

void Tts::unixSignalCheck()
{
    if (reloadRequested)
    {
        reloadRequested = false;
        qDebug() << "unixSignalCheck: reloadRequested";
        settings->sync();
    }
    if (exitRequested)
    {
        exitRequested = false;
        qDebug() << "unixSignalCheck: exitRequested";
        quit();
    }
}
