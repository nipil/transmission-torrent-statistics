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
    try
    {
        int result;

        Logger::Debug() << "Initializing application";
        Tts tts(argc, argv);

        Logger::Debug() << "Starting event loop";
        result = tts.exec();

        Logger::Debug() << "Event loop ended with value" << result;
        return result;
    }
    catch( int err )
    {
        Logger::Error() << "Main" << "Exception" << err;
        return err;
    }
    catch( ... )
    {
        Logger::Error() << "Main" << "Untracked  exception occured";
        return EXIT_UNKNOWN;
    }
}
