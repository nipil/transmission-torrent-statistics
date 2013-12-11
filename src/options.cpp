#include <QtGlobal>
#include <QDebug>
#include "common.h"
#include "options.h"

Options::Options(int &argc, char **argv) :
    no_rpc_polling(false)
{
    qDebug() << "Options::Options";
    for (int i = 0; i < argc; i++)
    {
        if (QString(argv[i]) == "--no-rpc-polling")
        {
            no_rpc_polling = true;
            qDebug() << "Disabling RPC polling";
        }
    }
}
