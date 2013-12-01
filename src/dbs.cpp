#include <QtGlobal>
#include <QDebug>
#include <QDir>
#include <QSqlError>
#include "common.h"
#include "dbs.h"

Dbs::Dbs(QObject *p, QSettings * s) :
    QObject(p),
    settings(s)
{
    qDebug() << "Dbs::Dbs";

    open();
}

void Dbs::open()
{
    qDebug() << "Dbs::open";

    db = QSqlDatabase::addDatabase("QSQLITE");

    QDir path(settings->value(TTS_SETTINGS_DB_PATH).toString());
    QString filename = settings->value(TTS_SETTINGS_DB_NAME).toString();
    QString absfilepath = path.absoluteFilePath(filename);
    qDebug() << "Database location" << absfilepath;

    db.setDatabaseName(absfilepath);

    bool ok = db.open();
    qDebug() << "db open result" << ok;
    if (!ok)
    {
        qCritical() << "Database open error" << db.lastError().text();
        throw EXIT_DB_OPEN;
    }
}

Dbs::~Dbs()
{
    qDebug() << "Dbs::~Dbs";

}

void Dbs::store(QString & hashString, qlonglong downloadedEver, qlonglong uploadedEver, QString & name)
{
    qDebug() << "Dbs::store";

}
