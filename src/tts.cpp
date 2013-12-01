#include <QtGlobal>
#include <QDebug>
#include <QDir>
#include "common.h"
#include "tts.h"
#include "ttsig.h"

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

    signal_init();

    settings = new QSettings(this);
    Q_ASSERT(settings != NULL);
    loadSettings();

    dbs = new Dbs(this,settings);
    Q_ASSERT(dbs != NULL);

    rpc = new Rpc(this,settings);
    Q_ASSERT(rpc != NULL);

    bool r1 = connect(rpc,SIGNAL(store(QString&,qlonglong,qlonglong,QString&)), dbs, SLOT(store(QString&,qlonglong,qlonglong,QString&)));
    Q_ASSERT(r1 == true);

    signalTimer = new QTimer(this);
    Q_ASSERT(signalTimer != NULL);

    bool r2 = connect(signalTimer,SIGNAL(timeout()),this,SLOT(signalCheck()));
    Q_ASSERT(r2 == true);

    signalTimer->start(200);

    pollingTimer = new QTimer(this);
    Q_ASSERT(pollingTimer != NULL);

    bool r3 = connect(pollingTimer,SIGNAL(timeout()),rpc,SLOT(poll()));
    Q_ASSERT(r3 == true);

    pollingTimer->start(60000);

    rpc->poll();
}

Tts::~Tts()
{
    qDebug() << "Tts::~Tts";

    if (dbs)
        delete dbs;

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
    params.insert(TTS_SETTINGS_DB_NAME, (TTS_APP_NAME ".sqlite") );
    params.insert(TTS_SETTINGS_DB_PATH, QDir::homePath ());

    QMap<QString,QVariant>::const_iterator i;
    for (i=params.begin();i!=params.end();i++)
    {
        QVariant var = settings->value( i.key(), i.value() );
        settings->setValue( i.key(), var );
    }
}

void Tts::signalCheck()
{
    if (reloadRequested)
    {
        reloadRequested = false;
        qDebug() << "Tts::signalCheck reloadRequested";
        settings->sync();
        // TODO: reopen database
    }
    if (exitRequested)
    {
        exitRequested = false;
        qDebug() << "Tts::signalCheck exitRequested";
        quit();
    }
}

bool Tts::notify ( QObject * receiver, QEvent * event )
{
    try
    {
        return QCoreApplication::notify(receiver,event);
    }
    catch( int err )
    {
        qCritical() << "Tts::notify" << "Exception" << err;
        exit(err);
    }
    catch( ... )
    {
        qCritical() << "Tts::notify" << "Untracked  exception occured";
        exit();
    }
    return false;
}
