#ifndef LOGGER_H
#define LOGGER_H

#include <QTextStream>
#include <QDateTime>

class Logger
{
    // qDebug and friends
    static bool show_qt_debug;
    static bool show_qt_warning;
    static bool show_qt_critical;
    static bool show_qt_fatal;
    static void loggerQtMessageOutput(QtMsgType type, const char *msg);

public:
    static void showQtDebug(bool show);
    static void showQtWarning(bool show);
    static void showQtCritical(bool show);
    static void showQtFatal(bool show);
    static void initQtLog();

private:
    Logger();
    virtual ~Logger();
    static Logger TTSLOG;
    static QTextStream strout;

    static inline Logger & Prepend(QString header)
    {
        strout << QDateTime::currentDateTime()
                  .toString("yyyy-MM-dd hh:mm:ss")
               << " " << header;
        return TTSLOG;
    }

public:
    static inline Logger & Debug() { return Prepend("Debug"); }
    static inline Logger & Info() { return Prepend("Info"); }
    static inline Logger & Warn() { return Prepend("Warn"); }
    static inline Logger & Error() { return Prepend("Error"); }

    template <class T>
    inline Logger & operator << (T t)
    {
        strout << " " << t;
        return TTSLOG;
    }
};

#endif // LOGGER_H
