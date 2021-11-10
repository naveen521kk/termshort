#pragma once
#define LOGGING_DEBUG_PREFIX "DEBUG: "
#define LOGGING_INFO_PREEFIX "INFO: "
#define DEBUG_MODE 0;
void initialise_logger ();
void log_info (char *message, ...);
void log_error (char *message, int exit, ...);
void log_debug(char *message, ...);
