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

        Logger::Verbose() << "Initializing application";
        Tts tts(argc, argv);

        Logger::Verbose() << "Starting application event loop";
        result = tts.exec();

        Logger::Verbose() << "Application event loop ended with return value" << result;
        return result;
    }
    catch( EXIT_CODES err )
    {
        Logger::Verbose() << "Exception caught, terminating with return code" << err;
        return err;
    }
    catch( ... )
    {
        Logger::Verbose() << "Untracked exception occured";
        return EXIT_UNKNOWN;
    }
}
