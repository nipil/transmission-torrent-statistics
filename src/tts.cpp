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
    loadSettings();

    if (options.db_age > 0 ||
            options.db_deduplication)
        Dbs::maintenance(this,settings,options);

    dbs = new Dbs(this,settings);

    rpc = new Rpc(this,settings);

    web = new Web(this,settings);

    if (!connect(rpc,SIGNAL(store(QString&,qlonglong,QString&,uint)),
                 dbs,  SLOT(store(QString&,qlonglong,QString&,uint))))
    {
        Logger::Error() << "Could not connect QT signal in file" << __FILE__ << "on line" << __LINE__;
        throw EXIT_QTCONNECT_ERROR;
    }

    if (!connect(web,SIGNAL(jsonList(QByteArray&)),
                 dbs,  SLOT(jsonList(QByteArray&))))
    {
        Logger::Error() << "Could not connect QT signal in file" << __FILE__ << "on line" << __LINE__;
        throw EXIT_QTCONNECT_ERROR;
    }

    if (!connect(web,SIGNAL(jsonStats(QByteArray&,QString&,uint,uint)),
                 dbs,  SLOT(jsonStats(QByteArray&,QString&,uint,uint))))
    {
        Logger::Error() << "Could not connect QT signal in file" << __FILE__ << "on line" << __LINE__;
        throw EXIT_QTCONNECT_ERROR;
    }

    signalTimer = new QTimer(this);

    if (!connect(signalTimer,SIGNAL(timeout()),
                 this,         SLOT(signalCheck())))
    {
        Logger::Error() << "Could not connect QT signal in file" << __FILE__ << "on line" << __LINE__;
        throw EXIT_QTCONNECT_ERROR;
    }

    signalTimer->start(200);

    pollingTimer = new QTimer(this);

    if (!connect(pollingTimer,SIGNAL(timeout()),rpc,SLOT(poll())))
    {
        Logger::Error() << "Could not connect QT signal in file" << __FILE__ << "on line" << __LINE__;
        throw EXIT_QTCONNECT_ERROR;
    }

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
    params.insert(TTS_SETTINGS_WEB_URL_REMOVE_PREFIX, QString(""));

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
    catch( EXIT_CODES err )
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
