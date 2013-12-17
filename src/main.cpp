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

        Logger::Info() << "Initializing application";
        Tts tts(argc, argv);

        Logger::Info() << "Starting application event loop";
        result = tts.exec();

        Logger::Info() << "Application event loop ended with return value" << result;
        return result;
    }
    catch( int err )
    {
        Logger::Error() << "Exception" << err;
        return err;
    }
    catch( ... )
    {
        Logger::Error() << "Untracked  exception occured";
        return EXIT_UNKNOWN;
    }
}
