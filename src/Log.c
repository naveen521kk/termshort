#include "Log.h"
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int DEBUG_MESSAGES = DEBUG_MODE;

void
log_info (char *message, ...)
{
    va_list args;
    va_start (args, message);
    fprintf (stdout, LOGGING_INFO_PREEFIX);
    vfprintf (stdout, message, args);
    fprintf (stdout, "\n");
    va_end (args);
}

void
log_error (int should_exit, char *message, ...)
{
    fprintf (stderr, "Error: ");

    va_list args;
    va_start (args, message);

    vfprintf (stderr, message, args);
    fprintf (stderr, "\n");
    va_end (args);

    if (should_exit == 1)
        {
            log_debug ("Exiting with status code %d", 1);
            exit (1);
        }
}

void
log_debug (char *message, ...)
{
    va_list args;
    va_start (args, message);

    const char *is_debug = getenv ("SCREENSHOTTER_DEBUG");
    if (is_debug && !*is_debug)
        {
            is_debug = NULL;
        }

    if (DEBUG_MESSAGES || is_debug)
        {
            fprintf (stdout, LOGGING_DEBUG_PREFIX);
            vfprintf (stdout, message, args);
            fprintf (stdout, "\n");
        }
    va_end (args);
}

void
initialise_logger (int argc, char *argv[])
{
    for (size_t i = 1; i < argc; i++)
        {
            if (0 == strcmp (argv[i], "--debug"))
                {
                    DEBUG_MESSAGES = 1;
                }
        }
    log_debug ("Initialising logger");
}