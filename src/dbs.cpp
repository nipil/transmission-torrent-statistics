#include <QtGlobal>
#include <QDebug>
#include <QDir>
#include <QSqlQuery>
#include <QSqlError>
#include <QDateTime>
#include <qjson/serializer.h>
#include "common.h"
#include "dbs.h"
#include "logger.h"

#include <QSqlDriver>

Dbs::Dbs(QObject *p, QSettings * s, QString overrideFileName) :
    QObject(p),
    overridden_filename(overrideFileName),
    settings(s)
{
    qDebug() << "Dbs::Dbs";

    connection_name = TTS_DB_CONNECTION_NAME + overridden_filename;
    Logger::Debug(this) << "Database connection name :" << connection_name;

    open();
}

void Dbs::open()
{
    qDebug() << "Dbs::open";

    QSqlDatabase db = QSqlDatabase::addDatabase(TTS_DB_DRIVER,connection_name);
    if (!db.isValid())
    {
        Logger::Error() << "Could not setup database environment using driver" << TTS_DB_DRIVER << db.lastError().text();
        throw EXIT_DB_SETUP_FAILED;
    }

    QDir path(settings->value(TTS_SETTINGS_DB_PATH).toString());
    QString filename = settings->value(TTS_SETTINGS_DB_NAME).toString();
    if (!overridden_filename.isEmpty())
    {
        filename = overridden_filename;
        Logger::Info() << "DB filename overridden to" << filename;
    }
    QString absfilepath = path.absoluteFilePath(filename);
    Logger::Info() << "Database location" << absfilepath;

    db.setDatabaseName(absfilepath);

    bool ok = db.open();
    qDebug() << "DB open" << ok;
    if (!ok)
    {
        Logger::Error() << "Database open error" << db.lastError().text();
        throw EXIT_DB_OPEN;
    }

    if (!db.driver()->hasFeature(QSqlDriver::Transactions))
    {
        Logger::Error() << "Database doesn't support transaction";
        throw EXIT_DB_TRANSACTION_ERROR;
    }

    known_tables = db.tables();
    known_tables.sort();
    Logger::Debug(this) << this << "DB Tables are :" << known_tables.join(" ");

    if (!known_tables.contains(TTS_DB_HASHTABLE_NAME))
    {
        Logger::Debug(this) << "Creating master table";
        createMasterTable();
    }

    loadMasterHashes();

    db.close();
}

void Dbs::close()
{
    qDebug() << "Dbs::close";

    QSqlDatabase::removeDatabase(connection_name);
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

    QSqlDatabase db = QSqlDatabase::database(connection_name,true);

    if (!db.isValid())
    {
        Logger::Error() << "Could not find database connection" << db.lastError().text();
        throw EXIT_DB_CONNECTION_NOT_FOUND;
    }

    if (!db.isOpen())
    {
        Logger::Error() << "Database open error" << db.lastError().text();
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
            Logger::Error() << "Could not start transaction" << db.lastError().text();
            throw EXIT_DB_TRANSACTION_ERROR;
        }
    }

    QSqlQuery * query = new QSqlQuery("",db);
    Q_CHECK_PTR(query);

    return query;
}

void Dbs::execQuery(QSqlQuery * query)
{
    qDebug() << "Dbs::execQuery" << query;

    Q_CHECK_PTR(query);
    bool ok = query->exec();

    qDebug() << "sql" << query->lastQuery();

    if (!ok)
    {
        Logger::Error() << "Query failed" << query->lastError().text();
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
        QSqlDatabase db = QSqlDatabase::database(connection_name,true);
        if (!db.commit())
        {
            Logger::Error() << "Could not commit transaction" << db.lastError().text();
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

    known_hashes.insert(hashString,name);
}

void Dbs::loadMasterHashes()
{
    qDebug() << "Dbs::loadMasterHashes";

    QSqlQuery * q = initQuery(false);
    QString sql = QString("SELECT hash,name FROM %1;"
                          ).arg(TTS_DB_HASHTABLE_NAME);
    q->prepare(sql);
    execQuery(q);

    known_hashes.clear();
    while (q->next())
    {
        QString hash = q->value(0).toString();
        QString name = q->value(1).toString();
        Logger::Debug(this) << "hash" << hash << "name" << name;
        known_hashes.insert(hash,name);
    }

    cleanupQuery(q,false);
}

QString Dbs::getTorrentName(QString & hashString)
{
    return known_hashes.value(hashString);
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
                          "uploadedEver INTEGER(32),"
                          "primary KEY (unixtime));"
                          ).arg(tableName);
    q->prepare(sql);
    execQuery(q);
    cleanupQuery(q,true);

    known_tables.append(tableName);
}

uint Dbs::getCount(QString & hashString)
{
    qDebug() << "Dbs::getCount" << hashString;

    QSqlQuery * q = initQuery(false);
    QString sql = QString("SELECT COUNT(unixtime) FROM %1;").arg(hashToTable(hashString));
    q->prepare(sql);
    execQuery(q);

    uint count = 0;
    if (q->next())
    {
        bool ok;
        count = q->value(0).toUInt(&ok);
        if (!ok)
        {
            Logger::Error() << "Cannot convert count" << q->value(0).toString() << "to uint";
            throw EXIT_DB_CONVERT_ERROR;
        }
    }

    cleanupQuery(q,false);

    return count;
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
        Logger::Debug(this) << "Inserting torrent" << hashString << "in master table";
        insertMasterTable(hashString,name);
    }

    QString tableName = hashToTable(hashString);

    // if storage table for this torrent hash doesn't exist, create it
    if (!known_tables.contains(tableName))
    {
        Logger::Debug(this) << "Creating table for torrent" << hashString;
        createHashTable(tableName);
    }

    // data deduplication, don't store samples which didn't change
    QHash<QString,Sample>::iterator i = last_samples.find(hashString);
    if (i == last_samples.end())
    {
        Dbs::Sample sample = getLatest(hashString);
        i = last_samples.insert(hashString, sample);
    }

    if (i == last_samples.end() || i->downloadedEver != downloadedEver || i->uploadedEver != uploadedEver)
    {
        insertHashTable(tableName, unixtime, downloadedEver, uploadedEver);
        // replaces the old if already existant
        last_samples.insert( hashString, Sample(unixtime,downloadedEver,uploadedEver) );
    }
}

void Dbs::jsonList(QByteArray & out)
{
    qDebug() << "Dbs::jsonList";

    QVariantList torrents;
    foreach(QString hash, known_hashes.keys())
    {
        QVariantMap torrent;
        torrent.insert("name",getTorrentName(hash));
        torrent.insert("hash",hash);

        Dbs::Sample sample = getLatest(hash);
        QVariant last(sample.unixtime);
        torrent.insert("last",last.toString());
        torrents << torrent;
    }

    QJson::Serializer s;
    out = s.serialize(torrents);
}

void Dbs::jsonStats(QByteArray & out, QString & hashString, uint time_min, uint time_max)
{
    qDebug() << "Dbs::jsonStats" << hashString << time_min << time_max;

    if (!known_hashes.contains(hashString))
    {
        out.append("[]");
        return;
    }

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

Dbs::Sample Dbs::getLatest(QString & hashString)
{
    qDebug() << "Dbs::getLatest" << hashString;

    QString tableName = hashToTable(hashString);
    QSqlQuery * q = initQuery(false);
    QString sql = QString("SELECT unixtime,downloadedEver,uploadedEver FROM %1 "
                          "WHERE unixtime = (SELECT MAX(unixtime) FROM %2);"
                          ).arg(tableName).arg(tableName);
    q->prepare(sql);
    execQuery(q);

    Dbs::Sample sample;
    if (q->next())
    {
        sample.set(q->value(0),q->value(1),q->value(2));
        qDebug() << "Found" << sample.unixtime << sample.downloadedEver << sample.uploadedEver;
    }
    else
    {
        qDebug() << "No entry found";
    }

    cleanupQuery(q,false);

    return sample;
}

Dbs::Sample::Sample() :
  unixtime(0),
  downloadedEver(0),
  uploadedEver(0)
{
}

Dbs::Sample::Sample(uint t, qlonglong d, qlonglong u) :
    unixtime(t),
    downloadedEver(d),
    uploadedEver(u)
{
}

void Dbs::Sample::set(QVariant vt, QVariant vd, QVariant vu)
{
    unixtime = 0;
    downloadedEver = 0;
    uploadedEver = 0;

    bool ok;
    uint t = vt.toUInt(&ok);
    if (!ok)
    {
        Logger::Error() << "Cannot convert unixtime" << vt.toString() << "to uint";
        throw EXIT_DB_CONVERT_ERROR;
    }
    qlonglong d = vd.toLongLong(&ok);
    if (!ok)
    {
        Logger::Error() << "Cannot convert downloadedEver" << vd.toString() << "to qlonglong";
        throw EXIT_DB_CONVERT_ERROR;
    }
    qlonglong u = vu.toLongLong(&ok);
    if (!ok)
    {
        Logger::Error() << "Cannot convert uploadedEver" << vu.toString() << "to qlonglong";
        throw EXIT_DB_CONVERT_ERROR;
    }

    unixtime = t;
    downloadedEver = d;
    uploadedEver = u;
}

void Dbs::maintenance(QObject * p, QSettings * s, Options & o)
{
    qDebug() << "Dbs::maintenance";

    Logger::Info() << "Entering maintenance mode";

    // move current database out of the way to create a fresh one
    QDateTime now = QDateTime::currentDateTime();
    QDir path( s->value(TTS_SETTINGS_DB_PATH).toString() );
    QString cname = s->value(TTS_SETTINGS_DB_NAME).toString();
    QString oname = now.toString( "yyyy-MM-dd_hh-mm-ss_zzz_" ) + cname;
    QString cpath = path.absoluteFilePath( cname );
    QString opath = path.absoluteFilePath( oname );
    Logger::Info() << "DB Backup name:" << opath;
    if (!QFile::rename(cpath,opath))
    {
        Logger::Error() << "Renaming " << cpath << "to" << opath << "failed";
        throw EXIT_DB_MAINTENANCE_RENAME_ERROR;
    }

    // create new clean database and open old one
    Dbs cdb(p,s);
    Dbs odb(p,s,oname);

    QDateTime t_start = QDateTime::currentDateTime();

    // db_age parameters
    uint oldest_time = t_start.toTime_t() - o.db_age * 24 * 60 * 60;

    // import old database data to new database, cleaning on the way
    foreach(QString hashString, odb.known_hashes.keys())
    {
        uint count = odb.getCount(hashString);
        QString name = odb.getTorrentName(hashString);
        QString tableName = odb.hashToTable(hashString);

        QSqlQuery * q = odb.initQuery(false);
        QString sql = QString("SELECT unixtime,downloadedEver,uploadedEver FROM %1 "
                              "ORDER BY unixtime ASC;").arg(tableName);
        q->prepare(sql);
        odb.execQuery(q);

        Dbs::Sample sample;
        bool first = true;
        while(q->next())
        {
            sample.set(q->value(0),q->value(1),q->value(2));

            // db age cleanup task, skip sample if old
            if (o.db_age > 0)
                if (sample.unixtime < oldest_time)
                    continue;

            // data deduplication is done automatically on lower level
            cdb.store(hashString,sample.downloadedEver,sample.uploadedEver, name, sample.unixtime);

            // deactivate qDebug() output after first sample to prevent flooding
            // this way we can still check table creation and so on
            if (first)
            {
                first = false;
                Logger::showQtDebug(false);
            }
        }
        // restore qDebug() output
        Logger::showQtDebug(true);

        odb.cleanupQuery(q,false);

        uint after = 0;
        if (cdb.known_tables.contains(tableName))
            after = cdb.getCount(hashString);

        Logger::Info() << "Torrent" << hashString << "from" << count << "to" << after << "samples";
    }

    QDateTime t_end = QDateTime::currentDateTime();

    Logger::Info() << "Exiting maintenance mode, operations took" << t_start.secsTo(t_end) << "seconds";
}
