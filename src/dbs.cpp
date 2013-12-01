#include <QtGlobal>
#include <QDebug>
#include "common.h"
#include "dbs.h"

Dbs::Dbs(QObject *p, QSettings * s) :
    QObject(p),
    settings(s)
{
    qDebug() << "Dbs::Dbs";

    db = QSqlDatabase::addDatabase("QSQLITE");
    db.setDatabaseName(TTS_SETTINGS_DB_NAME);

    bool ok = db.open();
    qDebug() << "db open result" << ok;
    if (!ok)
    {
        qCritical() << "Could not open database";
        // TODO: stop polling process ?
        return;
    }
}

Dbs::~Dbs()
{
    qDebug() << "Dbs::~Dbs";

}
