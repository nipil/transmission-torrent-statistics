#include <QtGlobal>
#include <QDebug>
#include "common.h"
#include "options.h"

Options::Options(int &argc, char **argv) :
    no_rpc_polling(false),
    rpc_polling_interval(60)
{
    qDebug() << "Options::Options";
    for (int i = 0; i < argc; i++)
    {
        if (QString(argv[i]) == "--no-rpc-polling")
        {
            no_rpc_polling = true;
            qDebug() << "Disabling RPC polling";
        }
        if (QString(argv[i]) == "--rpc-polling-fast")
        {
            rpc_polling_interval = 5;
            qDebug() << "Fast RPC polling";
        }
    }
}
