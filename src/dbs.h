#ifndef DBS_H
#define DBS_H

#include <QSettings>
#include <QSqlDatabase>

class Dbs : public QObject
{
    Q_OBJECT

    QSettings * settings;
    QSqlDatabase db;

    void open();

public:
    explicit Dbs(QObject * p, QSettings * s);
    virtual ~Dbs();

signals:

public slots:
    void store(QString & hashString, qlonglong downloadedEver, qlonglong uploadedEver, QString & name);

};

#endif // DBS_H
