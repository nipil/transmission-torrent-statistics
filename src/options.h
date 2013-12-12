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
    QVariant requiredValue(QString & current, QStringList & remaining);

public:
    bool no_rpc_polling;
    uint rpc_polling_interval;
    bool db_deduplication;
    Options(QStringList args);
};

#endif // OPTIONS_H
