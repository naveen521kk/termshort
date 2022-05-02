#include "Utils.h"
#include "Log.h"
#include "Version.h"
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int
starts_with (const char *pre, const char *str)
{
    return strncmp (pre, str, strlen (str)) == 0;
}

int
convert_char_to_int (const char *str)
{
    errno = 0;
    char *end;
    const int i = strtol (str, &end, 10);

    const int range_error = errno == ERANGE;
    if (range_error)
        log_error (1,
                   "Range error occurred while converting to int. Check your "
                   "parameters.");

    return i;
}

char *
split_args (char *inp, const char *sep)
{
    char *tmp;
    tmp = strtok (inp, sep);
    if (!*tmp)
        {
            log_error (1, "Invalid argument: %s", inp);
        }
    tmp = strtok (NULL, sep);
    if (!tmp)
        {
            log_error (1, "Invalid argument: %s", inp);
        }
    return tmp;
}

void
print_help (char *program_name)
{
    printf ("usage: %s [--version] [--help] [--debug] [<filename>]\n",
            program_name);
    printf ("\n\n");
    printf ("    --version, -v    Print version of program and exit\n");
    printf ("    --help, -h       Print this help text and exit\n");
    printf ("    --debug      Enable Debug mode\n");
}

void
print_version (char *program_name)
{
    printf ("%s version %s\n", program_name, VERSION);
}
