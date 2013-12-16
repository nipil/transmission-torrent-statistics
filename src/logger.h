#ifndef LOGGER_H
#define LOGGER_H

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
};

#endif // LOGGER_H
