#ifndef COMMON_H
#define COMMON_H

#define TTS_APP_VERSION "1.3"
#define TTS_APP_ORGNAME "nipil"
#define TTS_APP_ORGDOMAIN "nipil.org"
#define TTS_APP_NAME "transmission-torrent-statistics"

#define TTS_DB_CONNECTION_NAME TTS_APP_NAME
#define TTS_DB_HASHTABLE_NAME "master"
#define TTS_DB_HASH_PREFIX "torrent"
#define TTS_DB_DRIVER "QSQLITE"

#define TTS_SETTINGS_RPC_HOST "RPC/Host"
#define TTS_SETTINGS_RPC_PORT "RPC/Port"
#define TTS_SETTINGS_RPC_USER "RPC/User"
#define TTS_SETTINGS_RPC_PASSWORD "RPC/Password"
#define TTS_SETTINGS_RPC_SSL "RPC/SSL"

#define TTS_SETTINGS_DB_NAME "SQLite/name"
#define TTS_SETTINGS_DB_PATH "SQLite/path"

#define TTS_SETTINGS_WEB_PORT "WebServer/Port"
#define TTS_SETTINGS_WEB_PATH "WebServer/Path"

#define TTS_SETTINGS_DB_PATH_DEFAULT QDir::homePath()
#define TTS_SETTINGS_WEB_PATH_DEFAULT QDir::homePath()

#define TTS_BUFFER_SIZE (1 << 15)

enum
{
    EXIT_OK = 0,
    EXIT_SIGNAL_INIT_ERROR,
    EXIT_WEB_LISTEN_ERROR,
    EXIT_DB_OPEN,
    EXIT_RPC_AUTH_FAILED,
    EXIT_RPC_NO_TOKEN,
    EXIT_RPC_HTTP_ERROR,
    EXIT_JSON_PARSING_ERROR,
    EXIT_JSON_CONVERT_ERROR,
    EXIT_RPC_FAILED,
    EXIT_DB_TRANSACTION_ERROR,
    EXIT_DB_CONNECTION_NOT_FOUND,
    EXIT_DB_SETUP_FAILED,
    EXIT_DB_QUERY_FAILED,
    EXIT_WEB_CONVERT_ERROR,
    EXIT_ARGUMENT_MISSING_VALUE_ERROR,
    EXIT_ARGUMENT_CONVERT_VALUE_ERROR,
    EXIT_ARGUMENT_UNKNOWN_ERROR,
    EXIT_DB_CONVERT_ERROR,
    EXIT_DB_MAINTENANCE_RENAME_ERROR,

    EXIT_UNKNOWN = 255
};

#endif // COMMON_H
