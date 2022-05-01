#pragma once
#define LOGGING_ERROR_PREFIX "Error: "
#define LOGGING_DEBUG_PREFIX "DEBUG: "
#define LOGGING_INFO_PREEFIX "INFO: "
#define DEBUG_MODE 0;
void initialise_logger ();
void log_info (char *message, ...);
void log_error (int should_exit, char *message, ...);
void log_debug (char *message, ...);
