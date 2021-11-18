#include "Log.h"
#include <errno.h>
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
    const long i = strtol (str, &end, 10);

    const int range_error = errno == ERANGE;
    if (range_error)
        log_error (1, "Range error occurred while converting to int. Check your "
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
