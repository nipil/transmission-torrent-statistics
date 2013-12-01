#ifndef COMMON_H
#define COMMON_H

#define TTS_APP_VERSION "1.0"
#define TTS_APP_ORGNAME "nipil"
#define TTS_APP_ORGDOMAIN "nipil.org"
#define TTS_APP_NAME "transmission-torrent-statistics"

#define TTS_SETTINGS_RPC_HOST "RPC/Host"
#define TTS_SETTINGS_RPC_PORT "RPC/Port"
#define TTS_SETTINGS_RPC_USER "RPC/User"
#define TTS_SETTINGS_RPC_PASSWORD "RPC/Password"
#define TTS_SETTINGS_RPC_SSL "RPC/SSL"

#define TTS_SETTINGS_DB_NAME "SQLite/name"
#define TTS_SETTINGS_DB_PATH "SQLite/path"

#define EXIT_UNKNOWN 255
#define EXIT_OK 0
#define EXIT_SIGNAL_INITERROR 1
#define EXIT_CONNECTION_REFUSED 2
#define EXIT_DB_OPEN 3
#define EXIT_RPC_AUTHFAILED 4
#define EXIT_RPC_NOTOKEN 5
#define EXIT_RPC_HTTPERROR 6
#define EXIT_JSON_PARSINGERROR 7
#define EXIT_JSON_CONVERTERROR 8
#define EXIT_RPC_FAILED 9

#endif // COMMON_H
