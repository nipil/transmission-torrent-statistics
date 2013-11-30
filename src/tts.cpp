#include <QtGlobal>
#include <QDebug>
#include "common.h"
#include "tts.h"

bool Tts::reloadRequested = false;
bool Tts::exitRequested = false;

Tts::Tts(int &argc, char **argv) :
    QCoreApplication(argc,argv)
{
    qDebug() << "Tts::Tts";

    this->setOrganizationName(TTS_APP_ORGNAME);
    this->setOrganizationDomain(TTS_APP_ORGDOMAIN);
    this->setApplicationName(TTS_APP_NAME);
    this->setApplicationVersion(TTS_APP_VERSION);

    settings = new QSettings(this);
    Q_ASSERT(settings != NULL);
    loadSettings();

    rpc = new Rpc(this,settings);
    Q_ASSERT(rpc != NULL);

    signalTimer = new QTimer(this);
    Q_ASSERT(signalTimer != NULL);
    connect(signalTimer,SIGNAL(timeout()),this,SLOT(unixSignalCheck()));
    signalTimer->start(200);

    pollingTimer = new QTimer(this);
    Q_ASSERT(pollingTimer != NULL);

    connect(pollingTimer,SIGNAL(timeout()),rpc,SLOT(poll()));
    pollingTimer->start(60000);

    rpc->poll();
}

Tts::~Tts()
{
    qDebug() << "Tts::~Tts";

    if (settings)
        delete settings;

    if (rpc)
        delete rpc;

    if (signalTimer)
        delete signalTimer;

    if (pollingTimer)
        delete pollingTimer;
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
