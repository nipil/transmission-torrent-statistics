#ifndef DBS_H
#define DBS_H

#include <QSettings>
#include <QSqlDatabase>
#include <QStringList>

#include "options.h"

class Dbs : public QObject
{
    Q_OBJECT

    enum SqliteWriteMode {
        WriteOff = 0,
        WriteNormal = 1,
        WriteFull = 2
    };

    QString overridden_filename;
    QString connection_name;
    QSettings * settings;

    QStringList known_tables;
    QHash<QString,QString> known_hashes;

public:
    class Sample
    {
    public:
        uint unixtime;
        qlonglong downloadedEver;
        qlonglong uploadedEver;
        Sample();
        Sample(uint t, qlonglong d, qlonglong u);
    };

private:
    QHash<QString,Sample> last_samples;

    void open();
    void close();

    QSqlQuery * initQuery(bool transaction);
    void execQuery(QSqlQuery * query);
    void cleanupQuery(QSqlQuery * query, bool transaction);

    void loadMasterHashes();
    QString getTorrentName(QString & hashString);

    QString hashToTable(QString & hashString);
    void createMasterTable();
    void insertMasterTable(QString & hashString, QString & name);
    void createHashTable(QString & hashString);
    void insertHashTable(QString & hashString, uint unixtime, qlonglong downloadedEver, qlonglong uploadedEver);

    Dbs::Sample getLatest(QString & hashString);
    uint getCount(QString & hashString);

public:
    explicit Dbs(QObject * p, QSettings * s, QString overrideFileName = QString());
    virtual ~Dbs();

    void reload();

    static void maintenance(QObject *p, QSettings * s, Options & o);

signals:

public slots:
    void store(QString & hashString, qlonglong downloadedEver, qlonglong uploadedEver, QString & name, uint unixtime);

    void jsonList(QByteArray & out);
    void jsonStats(QByteArray & out, QString & hashString, uint time_min, uint time_max);
};

#endif // DBS_H
