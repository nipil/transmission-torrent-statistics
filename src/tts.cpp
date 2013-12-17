#include <QtGlobal>
#include <QDebug>
#include <QDir>
#include "common.h"
#include "logger.h"
#include "tts.h"
#include "ttsig.h"

bool Tts::reloadRequested = false;
bool Tts::exitRequested = false;

Tts::Tts(int &argc, char **argv) :
    QCoreApplication(argc,argv),
    options(this->arguments())
{
    qDebug() << "Tts::Tts";

    this->setOrganizationName(TTS_APP_ORGNAME);
    this->setOrganizationDomain(TTS_APP_ORGDOMAIN);
    this->setApplicationName(TTS_APP_NAME);
    this->setApplicationVersion(TTS_APP_VERSION);

    signal_init();

    settings = new QSettings(this);
    Q_CHECK_PTR(settings);
    loadSettings();

    Dbs::maintenance(this,settings,options);

    dbs = new Dbs(this,settings);
    Q_CHECK_PTR(dbs);

    rpc = new Rpc(this,settings);
    Q_CHECK_PTR(rpc);

    web = new Web(this,settings);
    Q_CHECK_PTR(web);

    bool r1 = connect(rpc,SIGNAL(store(QString&,qlonglong,qlonglong,QString&,uint)),
                      dbs,  SLOT(store(QString&,qlonglong,qlonglong,QString&,uint)));
    Q_ASSERT(r1 == true);

    bool w1 = connect(web,SIGNAL(jsonList(QByteArray&)),
                      dbs,  SLOT(jsonList(QByteArray&)));
    Q_ASSERT(w1 == true);

    bool w2 = connect(web,SIGNAL(jsonStats(QByteArray&,QString&,uint,uint)),
                      dbs,  SLOT(jsonStats(QByteArray&,QString&,uint,uint)));
    Q_ASSERT(w2 == true);

    signalTimer = new QTimer(this);
    Q_CHECK_PTR(signalTimer);

    bool r2 = connect(signalTimer,SIGNAL(timeout()),this,SLOT(signalCheck()));
    Q_ASSERT(r2 == true);

    signalTimer->start(200);

    pollingTimer = new QTimer(this);
    Q_CHECK_PTR(pollingTimer);

    bool r3 = connect(pollingTimer,SIGNAL(timeout()),rpc,SLOT(poll()));
    Q_ASSERT(r3 == true);

    if (!options.no_rpc_polling)
    {
        // time based polling
        pollingTimer->start(options.rpc_polling_interval * 1000);
        // force an immediate polling
        rpc->poll();
    }
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

    if (web)
        delete web;

    if (signalTimer)
        delete signalTimer;

    if (pollingTimer)
        delete pollingTimer;
}

void Tts::loadSettings()
{
    Logger::Debug(this) << "Loading Settings";

    QMap<QString,QVariant> params;
    params.insert(TTS_SETTINGS_RPC_HOST,QString("localhost"));
    params.insert(TTS_SETTINGS_RPC_PORT, int(9091));
    params.insert(TTS_SETTINGS_RPC_USER, QString(""));
    params.insert(TTS_SETTINGS_RPC_PASSWORD, QString(""));
    params.insert(TTS_SETTINGS_RPC_SSL, bool(false));
    params.insert(TTS_SETTINGS_DB_NAME, (TTS_APP_NAME ".sqlite") );
    params.insert(TTS_SETTINGS_DB_PATH, TTS_SETTINGS_DB_PATH_DEFAULT);
    params.insert(TTS_SETTINGS_WEB_PORT, int(4646));
    params.insert(TTS_SETTINGS_WEB_PATH, TTS_SETTINGS_WEB_PATH_DEFAULT);

    QMap<QString,QVariant>::const_iterator i;
    for (i=params.begin();i!=params.end();i++)
    {
        QVariant var = settings->value( i.key(), i.value() );
        settings->setValue( i.key(), var );
        Logger::Debug(this) << "Parameter" << i.key() << "=" << var.toString();
    }
}

void Tts::signalCheck()
{
    if (reloadRequested)
    {
        reloadRequested = false;
        Logger::Info() << "Reload requested";
        settings->sync();
        dbs->reload();
    }
    if (exitRequested)
    {
        exitRequested = false;
        Logger::Info() << "Exit requested";
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
