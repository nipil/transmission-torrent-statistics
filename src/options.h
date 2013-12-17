#ifndef OPTIONS_H
#define OPTIONS_H

#include <QStringList>
#include <QVariant>

class Options
{
    QString lastValue;

    void errorUnknown(QString arg);
    void errorMissing(QString arg);
    void errorConvert(QString arg, QString type);
    void errorRange(QString arg, QString v_min, QString v_max);

    QVariant requiredValue(QString & current, QStringList & remaining);

public:
    bool no_rpc_polling;
    uint rpc_polling_interval;
    bool db_deduplication;
    uint db_age;
    Options(QStringList args);
};

#endif // OPTIONS_H
