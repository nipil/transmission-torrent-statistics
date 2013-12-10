#include <QtGlobal>
#include <QDebug>
#include <QDir>
#include <QSqlQuery>
#include <QSqlError>
#include <QDateTime>
#include <qjson/serializer.h>
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

    known_tables = db.tables();
    qDebug() << "Tables are :" << known_tables.join("\n");

    if (!known_tables.contains(TTS_DB_HASHTABLE_NAME))
        createMasterTable();

    loadMasterHashes();

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

QSqlQuery * Dbs::initQuery(bool transaction)
{
    qDebug() << "Dbs::initQuery" << transaction;

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

    /*
     *turn off sqlite write sync for each query to speed things up 100x
     *see https://www.sqlite.org/pragma.html#pragma_synchronous
     */
    QSqlQuery q(QString("PRAGMA synchronous = %1;").arg(WriteOff),db);

    // When using transactions, you must start the transaction before you create your query.
    if (transaction)
    {
        if (!db.transaction())
        {
            qCritical() << "Could not start transaction" << db.lastError().text();
            throw EXIT_DB_TRANSACTION_ERROR;
        }
    }

    QSqlQuery * query = new QSqlQuery("",db);
    Q_ASSERT(query != NULL);

    return query;
}

void Dbs::execQuery(QSqlQuery * query)
{
    qDebug() << "Dbs::execQuery" << query;

    Q_ASSERT(query != NULL);
    bool ok = query->exec();

    qDebug() << "sql" << query->lastQuery();

    if (!ok)
    {
        qCritical() << "Query failed" << query->lastError().text();
        if (query) delete query;
        throw EXIT_DB_QUERY_FAILED;
    }
}

void Dbs::cleanupQuery(QSqlQuery * query, bool transaction)
{
    qDebug() << "Dbs::cleanupQuery" << query << transaction;

    if (query) delete query;

    if (transaction)
    {
        QSqlDatabase db = QSqlDatabase::database(TTS_DB_CONNECTION_NAME,true);
        if (!db.commit())
        {
            qCritical() << "Could not commit transaction" << db.lastError().text();
            throw EXIT_DB_TRANSACTION_ERROR;
        }
    }
}

void Dbs::createMasterTable()
{
    qDebug() << "Dbs::createMasterTable";

    QSqlQuery * q = initQuery(true);
    QString sql = QString("CREATE TABLE %1"
                          "(hash VARCHAR(255),"
                          "name VARCHAR(255),"
                          "primary KEY (hash));"
                          ).arg(TTS_DB_HASHTABLE_NAME);
    q->prepare(sql);
    execQuery(q);
    cleanupQuery(q,true);

    known_tables.append(TTS_DB_HASHTABLE_NAME);
}

void Dbs::insertMasterTable(QString & hashString, QString & name)
{
    qDebug() << "Dbs::insertMasterTable" << hashString << name;

    QSqlQuery * q = initQuery(true);
    QString sql = QString("INSERT OR REPLACE INTO %1"
                          "(hash,name) "
                          "VALUES( :hash, :name );"
                          ).arg(TTS_DB_HASHTABLE_NAME);
    q->prepare(sql);
    q->bindValue(":hash", hashString);
    q->bindValue(":name", name);
    execQuery(q);
    cleanupQuery(q,true);

    known_hashes.append(hashString);
}

void Dbs::loadMasterHashes()
{
    qDebug() << "Dbs::loadMasterHashes";

    QSqlQuery * q = initQuery(false);
    QString sql = QString("SELECT hash FROM %1;"
                          ).arg(TTS_DB_HASHTABLE_NAME);
    q->prepare(sql);
    execQuery(q);

    known_hashes.clear();
    while (q->next())
    {
        QString hash = q->value(0).toString();
        qDebug() << "hash" << hash;
        known_hashes.append(hash);
    }

    cleanupQuery(q,false);
}

QString Dbs::hashToTable(QString & hashString)
{
    return QString("%1_%2").arg(TTS_DB_HASH_PREFIX).arg(hashString);
}

void Dbs::createHashTable(QString & tableName)
{
    qDebug() << "Dbs::createHashTable" << tableName;

    QSqlQuery * q = initQuery(true);
    QString sql = QString("CREATE TABLE %1"
                          "(unixtime INTEGER(32),"
                          "downloadedEver INTEGER(32),"
                          "uploadedEver INTEGER(32));"
                          ).arg(tableName);
    q->prepare(sql);
    execQuery(q);
    cleanupQuery(q,true);

    known_tables.append(tableName);
}

void Dbs::insertHashTable(QString & tableName, uint unixtime, qlonglong downloadedEver, qlonglong uploadedEver)
{
    qDebug() << "Dbs::insertHashTable" << tableName << "T" << unixtime << "D" << downloadedEver << "U" << uploadedEver;

    QSqlQuery * q = initQuery(true);
    QString sql = QString("INSERT OR REPLACE INTO %1"
                          "(unixtime,downloadedEver,uploadedEver) "
                          "VALUES( :unixtime, :downloadedEver, :uploadedEver);"
                          ).arg(tableName);
    q->prepare(sql);
    q->bindValue(":unixtime", unixtime);
    q->bindValue(":downloadedEver", downloadedEver);
    q->bindValue(":uploadedEver", uploadedEver);
    execQuery(q);
    cleanupQuery(q,true);
}

void Dbs::store(QString & hashString, qlonglong downloadedEver, qlonglong uploadedEver, QString & name, uint unixtime)
{
    qDebug() << "Dbs::store" << hashString << downloadedEver << uploadedEver << name;

    // if not yet known, insert this torrent hash in master hash list
    if (!known_hashes.contains(hashString))
    {
        insertMasterTable(hashString,name);
    }

    QString tableName = hashToTable(hashString);

    // if storage table for this torrent hash doesn't exist, create it
    if (!known_tables.contains(tableName))
    {
        createHashTable(tableName);
    }

    insertHashTable(tableName, unixtime, downloadedEver, uploadedEver);
}

void Dbs::jsonList(QByteArray & out)
{
    qDebug() << "Dbs::jsonList";

    QSqlQuery * q = initQuery(false);
    QString sql = QString("SELECT hash,name FROM %1;").arg(TTS_DB_HASHTABLE_NAME);
    q->prepare(sql);
    execQuery(q);

    QVariantList torrents;
    while (q->next())
    {
        QVariantMap torrent;
        torrent.insert("name",q->value(1).toString());
        QString hash = q->value(0).toString();
        torrent.insert("hash",hash);
        QVariant last = lastActive(hash);
        torrent.insert("last",last.toString());
        torrents << torrent;
    }

    cleanupQuery(q,false);

    QJson::Serializer s;
    out = s.serialize(torrents);
}

void Dbs::jsonStats(QByteArray & out, QString & hashString, uint time_min, uint time_max)
{
    qDebug() << "Dbs::jsonStats" << hashString << time_min << time_max;

    QSqlQuery * q = initQuery(false);
    QString sql = QString("SELECT unixtime,downloadedEver,uploadedEver FROM %1 "
                          "WHERE :time_min < unixtime AND unixtime < :time_max "
                          "ORDER BY unixtime ASC;").arg(hashToTable(hashString));
    q->prepare(sql);
    q->bindValue(":time_min",time_min);
    q->bindValue(":time_max",time_max);
    execQuery(q);

    QVariantList torrents;
    while (q->next())
    {
        QVariantMap torrent;
        torrent.insert("t",q->value(0).toString());
        torrent.insert("d",q->value(1).toString());
        torrent.insert("u",q->value(2).toString());
        torrents << torrent;
    }

    cleanupQuery(q,false);

    QJson::Serializer s;
    out = s.serialize(torrents);
}

QVariant Dbs::lastActive(QString & hashString)
{
    qDebug() << "Dbs::lastActive" << hashString;

    QSqlQuery * q = initQuery(false);
    QString sql = QString("SELECT MAX(unixtime) FROM %1;").arg(hashToTable(hashString));
    q->prepare(sql);
    execQuery(q);

    QVariant when(0);
    if (q->next())
        when = q->value(0);
    qDebug() << "when" << when;

    cleanupQuery(q,false);

    return when;
}
