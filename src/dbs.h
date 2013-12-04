#ifndef DBS_H
#define DBS_H

#include <QSettings>
#include <QSqlDatabase>
#include <QStringList>

class Dbs : public QObject
{
    Q_OBJECT

    enum SqliteWriteMode {
        WriteOff = 0,
        WriteNormal = 1,
        WriteFull = 2
    };

    QSettings * settings;

    QStringList known_tables;
    QStringList known_hashes;

    void open();
    void close();

    QSqlQuery * initQuery(bool transaction);
    void execQuery(QSqlQuery * query);
    void cleanupQuery(QSqlQuery * query, bool transaction);

    void loadMasterHashes();

    QString hashToTable(QString & hashString);
    void createMasterTable();
    void insertMasterTable(QString & hashString, QString & name);
    void createHashTable(QString & hashString);
    void insertHashTable(QString & hashString, uint unixtime, qlonglong downloadedEver, qlonglong uploadedEver);

public:
    explicit Dbs(QObject * p, QSettings * s);
    virtual ~Dbs();

    void reload();

signals:

public slots:
    void store(QString & hashString, qlonglong downloadedEver, qlonglong uploadedEver, QString & name, uint unixtime);

};

#endif // DBS_H
