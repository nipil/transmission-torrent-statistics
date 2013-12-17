#ifndef LOGGER_H
#define LOGGER_H

#include <QTextStream>
#include <QDateTime>

class Logger
{
private:
    QTextStream out;
    typedef enum
    {
        LOG_ERROR = 0,
        LOG_WARN,
        LOG_INFO,
        LOG_DEBUG
    } LOG_LEVEL;
    LOG_LEVEL lvl;
    static uint global_max_level;

public:
    Logger(QString line_header, LOG_LEVEL level, void  * p = 0);
    Logger(const Logger & other); /* needed because QTextStream cannot be copied */
    virtual ~Logger();

    template <class T>
    Logger & operator << (T t)
    {
        if (lvl < global_max_level)
            out << " " << t;
        return *this;
    }

    static void setMaxLevel(uint level = 255);
    static Logger Debug(void * p = NULL);
    static Logger Info();
    static Logger Warn();
    static Logger Error();

private:
    static bool show_qt_debug;
    static bool show_qt_warning;
    static bool show_qt_critical;
    static bool show_qt_fatal;
    static QtMsgHandler original_handler;
    static void loggerQtMessageOutput(QtMsgType type, const char *msg);

public:
    static void showQtDebug(bool show);
    static void showQtWarning(bool show);
    static void showQtCritical(bool show);
    static void showQtFatal(bool show);

};

#endif // LOGGER_H
