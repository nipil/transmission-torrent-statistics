#include <QtGlobal>
#include <QDebug>
#include "common.h"
#include "logger.h"
#include "tts.h"

/*
 * QtDebuging can be really turned of at compile time via
 * DEFINES += QT_NO_DEBUG_OUTPUT in .pro
 */

int main(int argc, char *argv[])
{
    Logger::initQtLog();

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
