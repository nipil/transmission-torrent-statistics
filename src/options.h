#ifndef OPTIONS_H
#define OPTIONS_H

class Options
{
public:
    bool no_rpc_polling;

    Options(int &argc, char **argv);
};

#endif // OPTIONS_H
