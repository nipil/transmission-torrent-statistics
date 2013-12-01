#ifndef DBS_H
#define DBS_H

#include <QSettings>
#include <QSqlDatabase>

class Dbs : public QObject
{
    Q_OBJECT

    QSettings * settings;
    QSqlDatabase db;

public:
    explicit Dbs(QObject * p, QSettings * s);
    virtual ~Dbs();

signals:

public slots:

};

#endif // DBS_H
