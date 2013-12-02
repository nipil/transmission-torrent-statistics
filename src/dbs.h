#ifndef DBS_H
#define DBS_H

#include <QSettings>
#include <QSqlDatabase>
#include <QStringList>

class Dbs : public QObject
{
    Q_OBJECT

    QSettings * settings;

    QStringList tables;

    void open();
    void close();

    void createMasterTable();
    void createHashTable(QString & hashString);

public:
    explicit Dbs(QObject * p, QSettings * s);
    virtual ~Dbs();

    void reload();

signals:

public slots:
    void store(QString & hashString, qlonglong downloadedEver, qlonglong uploadedEver, QString & name);

};

#endif // DBS_H
