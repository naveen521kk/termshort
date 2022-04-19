#include "Log.h"
#include "Utils.h"
#include <Screenshot.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <windows.h>

// should be second
#include <wchar.h>

int
main (int argc, char *argv[])
{
    initialise_logger (argc, argv);
    log_debug ("Received %d arguments", argc);
    WCHAR *filename;
    filename = calloc (MAX_PATH, sizeof (WCHAR));
    if (filename == NULL)
        {
            log_error (1, "Memory error");
        }
    if (argc == 1)
        {
            log_info ("Received no arguments. Choosing interactive mode.");
            filename = L"default.bmp";
        }
    else
        {
            log_debug ("Received the following arguments: ");
            for (int i = 1; i < argc; i++)
                {
                    log_debug ("%d : %s", i, argv[i]);
                }

            int width, height;
            int optind;
            size_t positional_args_no = 0;
            char **positional_args;

            positional_args = calloc (argc, sizeof (char *));
            if (positional_args == NULL)
                {
                    log_error (1, "Memory Error. Can't allocate memory.");
                }

            for (optind = 1; optind < argc; optind++)
                {
                    if (starts_with (argv[optind], "--"))
                        {
                            if (starts_with (argv[optind], "--help"))
                                {
                                    print_help (argv[0]);
                                    return 129;
                                }
                            if (starts_with (argv[optind], "--width"))
                                {
                                    width = convert_char_to_int (
                                        split_args (argv[optind], "="));
                                    log_debug ("Setting Width: %d", width);
                                    continue;
                                }
                            if (starts_with (argv[optind], "--height"))
                                {
                                    height = convert_char_to_int (
                                        split_args (argv[optind], "="));
                                    log_debug ("Setting Height: %d", height);
                                    continue;
                                }
                            if (starts_with (argv[optind], "--debug"))
                                {
                                    continue; // handled else where.
                                }
                            if (starts_with (argv[optind], "--version"))
                                {
                                    print_version (argv[0]);
                                    return 0;
                                }
                            print_help (argv[0]);
                            log_error (1, "Invalid Argument: %s", argv[optind]);
                        }
                    else if (starts_with (argv[optind], "-"))
                        {
                            if (starts_with (argv[optind], "-h"))
                                {
                                    print_help (argv[0]);
                                    return 129;
                                }
                            if (starts_with (argv[optind], "-v"))
                                {
                                    print_version (argv[0]);
                                    return 0;
                                }
                        }
                    else
                        {
                            positional_args[positional_args_no] = calloc (
                                (strlen (argv[optind]) + 1), sizeof (char *));

                            if (positional_args[positional_args_no] == NULL)
                                {
                                    log_error (1, "Memory Error.");
                                }

                            positional_args[positional_args_no] = argv[optind];
                            positional_args_no += 1;
                        }
                }
            if (positional_args_no == 0 || positional_args_no > 1)
                {
                    for (size_t i = 0; i < positional_args_no; i++)
                        {
                            free (positional_args[positional_args_no]);
                        }
                    free (positional_args);
                    if (positional_args_no == 0)
                        log_error (1, "Positional argument `file` is missing.");
                    else
                        log_error (
                            1,
                            "Multiple positional arguments found for `file`.");
                    return 1;
                }
            /*
            positional_args_no contains number of positional args
            which means, if positional_args_no is 1 then we must access
            the 0th (n - 1)th element
            */
            log_debug ("Got filename as: %s",
                       positional_args[positional_args_no - 1]);

            int nChars = MultiByteToWideChar (
                CP_UTF8, 0, positional_args[positional_args_no - 1], -1, NULL,
                0);
            if (MultiByteToWideChar (CP_UTF8, 0,
                                     positional_args[positional_args_no - 1],
                                     -1, filename, nChars)
                != nChars)
                {
                    log_error (1, "Cannot convert string");
                }

            for (size_t i = 0; i < positional_args_no; i++)
                {
                    free (positional_args[positional_args_no]);
                }
            free (positional_args);
        }
    int output = grab_screenshot (1, 0, filename);
    log_info ("Screenshot saved at '%ls`", filename);
    free (filename);
    return output;
}
