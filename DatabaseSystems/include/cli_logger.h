#ifndef __CLI_LOGGER_H
#define __CLI_LOGGER_H

#define LOG_BUF 4096
#define LOG_MSG_BUF 256
#define TRANSACTION_MAX 200

#define LOG_FILE_NAME "logfile_"
#define V "VERBOSE ->"
#define E "  ERROR ->"

void createLogger();
void updateLog(char *key, char *str);
void shutDownLogger();

#endif
