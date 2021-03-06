#include <QtGlobal>
#include <QDebug>
#include "common.h"
#include "logger.h"
#include "options.h"

Options::Options(QStringList args) :
    // default values
    no_rpc_polling(false),
    rpc_polling_interval(60),
    db_deduplication(false),
    db_age(0)
{
    qDebug() << "Options::Options";

    // remove program name
    if (!args.empty())
        args.pop_front();

    bool ok;
    while (! args.isEmpty())
    {
        QString arg = args.takeFirst();
        qDebug() << arg;

        if (arg.startsWith("--no-rpc-polling"))
        {
            no_rpc_polling = true;
            Logger::Verbose() << "Disabling RPC polling";
        }

        else if (arg.startsWith("--rpc-polling-interval"))
        {
            uint t = requiredValue(arg,args).toUInt(&ok);
            if (!ok) errorConvert(arg,"uint");
            rpc_polling_interval = t;
            Logger::Verbose() << "Custom RPC polling interval set to" << rpc_polling_interval;
        }

        else if (arg.startsWith("--log-level"))
        {
            uint t = requiredValue(arg,args).toUInt(&ok);
            if (!ok) errorConvert(arg,"uint");
            if (Logger::LOG_MIN <= t && t < Logger::LOG_MAX)
                Logger::setMaxLevel(t);
            else
                errorRange(arg, QString::number(Logger::LOG_MIN), QString::number(Logger::LOG_MAX - 1));
            Logger::Verbose() << "Setting log level to" << t;
        }

        else if (arg.startsWith("--db-age-cleanup"))
        {
            uint t = requiredValue(arg,args).toUInt(&ok);
            if (!ok) errorConvert(arg,"uint");
            if (t == 0)
                errorInvalid(arg);
            db_age = t;
            Logger::Verbose() << "Databade age cleanup requested, all samples older than" << db_age << "days will be removed";
        }

        else if (arg.startsWith("--db-deduplication"))
        {
            db_deduplication = true;
            Logger::Verbose() << "Database deduplication requested";
        }

        else
            errorUnknown(arg);
    }
}

QVariant Options::requiredValue(QString & current, QStringList & remaining)
{
    qDebug() << "Options::requiredValue";

    int i = current.indexOf("=");

    lastValue = "";
    if (i != -1)
        lastValue = current.mid( i + 1 );
    else
        if (!remaining.empty())
            lastValue = remaining.takeFirst().trimmed();

    qDebug() << "Value" << lastValue;
    if (lastValue.isEmpty())
        errorMissing(current);

    return QVariant(lastValue);
}

void Options::errorMissing(QString arg)
{
    Logger::Error() << "Missing value for argument" << arg;
    throw EXIT_ARGUMENT_MISSING_VALUE_ERROR;
}

void Options::errorConvert(QString arg, QString type)
{
    Logger::Error() << "Cannot convert value" << lastValue << "of argument" << arg << "to type" << type;
    throw EXIT_ARGUMENT_CONVERT_VALUE_ERROR;
}

void Options::errorUnknown(QString arg)
{
    Logger::Error() << "Unknown argument" << arg;
    throw EXIT_ARGUMENT_UNKNOWN_ERROR;
}

void Options::errorRange(QString arg, QString v_min, QString v_max)
{
    Logger::Error() << "Argument" << arg << "out of range" << v_min << "-" << v_max;
    throw EXIT_ARGUMENT_RANGE_ERROR;
}

void Options::errorInvalid(QString arg)
{
    Logger::Error() << "Argument" << arg << "has an invalid value";
    throw EXIT_ARGUMENT_INVALID_ERROR;
}
