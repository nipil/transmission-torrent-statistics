#include <QtGlobal>
#include <QDebug>
#include "common.h"
#include "tts.h"

/*
 * Logging information :
 * debugging is done to stderr via qDebug()
 * normal logging is done via qWarning() which can be recovered
 * error is done via qCritical() and should stop the process
 *
 * Debuging can be disabled at compile time via
 * DEFINES += QT_NO_DEBUG_OUTPUT in .pro
 */

int main(int argc, char *argv[])
{
    try
    {
        int result;

        qDebug() << "Initializing application";
        Tts tts(argc, argv);

        qDebug() << "Starting event loop";
        result = tts.exec();

        qDebug() << "Event loop ended with value" << result;
        return result;
    }
    catch( int err )
    {
        qCritical() << "Main" << "Exception" << err;
        return err;
    }
    catch( ... )
    {
        qCritical() << "Main" << "Untracked  exception occured";
        return EXIT_UNKNOWN;
    }
}
