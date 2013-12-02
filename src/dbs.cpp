#include <QtGlobal>
#include <QDebug>
#include <QDir>
#include <QSqlQuery>
#include <QSqlError>
#include "common.h"
#include "dbs.h"

#include <QSqlDriver>

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

    QSqlDatabase db = QSqlDatabase::addDatabase(TTS_DB_DRIVER,TTS_DB_CONNECTION_NAME);
    if (!db.isValid())
    {
        qCritical() << "Could not setup database environment using driver" << TTS_DB_DRIVER << db.lastError().text();
        throw EXIT_DB_SETUP_FAILED;
    }

    QDir path(settings->value(TTS_SETTINGS_DB_PATH).toString());
    QString filename = settings->value(TTS_SETTINGS_DB_NAME).toString();
    QString absfilepath = path.absoluteFilePath(filename);
    qDebug() << "Database location" << absfilepath;

    db.setDatabaseName(absfilepath);

    bool ok = db.open();
    qDebug() << "DB open" << ok;
    if (!ok)
    {
        qCritical() << "Database open error" << db.lastError().text();
        throw EXIT_DB_OPEN;
    }

    if (!db.driver()->hasFeature(QSqlDriver::Transactions))
    {
        qCritical() << "Database doesn't support transaction";
        throw EXIT_DB_TRANSACTION_ERROR;
    }

    tables = db.tables();
    qDebug() << "Tables are :" << tables.join("\n");

    if (!tables.contains(TTS_DB_HASHTABLE_NAME))
        createMasterTable();

    db.close();
}

void Dbs::close()
{
    qDebug() << "Dbs::close";

    QSqlDatabase::removeDatabase(TTS_DB_CONNECTION_NAME);
}

void Dbs::reload()
{
    qDebug() << "Dbs::reload";

    close();
    open();
}

Dbs::~Dbs()
{
    qDebug() << "Dbs::~Dbs";

    close();
}

void Dbs::createMasterTable()
{
    qDebug() << "Dbs::createMasterTable" << QSqlDatabase::connectionNames().join("/");

    QSqlDatabase db = QSqlDatabase::database(TTS_DB_CONNECTION_NAME,true);

    if (!db.isValid())
    {
        qCritical() << "Could not find database connection" << db.lastError().text();
        throw EXIT_DB_CONNECTION_NOT_FOUND;
    }

    if (!db.isOpen())
    {
        qCritical() << "Database open error" << db.lastError().text();
        throw EXIT_DB_OPEN;
    }

    // When using transactions, you must start the transaction before you create your query.
    if (!db.transaction())
    {
        qCritical() << "Could not start transaction" << db.lastError().text();
        throw EXIT_DB_TRANSACTION_ERROR;
    }

    // do some stuff
    QSqlQuery query("",db);
    bool ok = query.exec(
                "CREATE TABLE "
                TTS_DB_HASHTABLE_NAME
                "(hash VARCHAR(255),"
                "name VARCHAR(255),"
                "primary KEY (hash));"
    );

    if (!ok)
    {
        qCritical() << "Could not create table" << query.lastError().text();
        throw EXIT_DB_CREATE_TABLE_FAILED;
    }

    if (!db.commit())
    {
        qCritical() << "Could not commit transaction" << db.lastError().text();
        throw EXIT_DB_TRANSACTION_ERROR;
    }

    db.close();

    tables.append(TTS_DB_HASHTABLE_NAME);
}

void Dbs::createHashTable(QString & hashString)
{
    qDebug() << "Dbs::createHashTable" << hashString;




}

void Dbs::store(QString & hashString, qlonglong downloadedEver, qlonglong uploadedEver, QString & name)
{
    qDebug() << "Dbs::store";

    if (!tables.contains(hashString))
        createHashTable(hashString);

}

