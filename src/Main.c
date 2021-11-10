#include "Log.h"
#include "Utils.h"
#include <stdio.h>
#include <string.h>
#include <windows.h>

// should be second
#include <wchar.h>
typedef HANDLE (__stdcall *Func_SetThreadDpiAwarenessContext) (HANDLE);

Func_SetThreadDpiAwarenessContext
get_thread_dpi_api ()
{
    Func_SetThreadDpiAwarenessContext api_function;
    HMODULE user32 = LoadLibraryA ("User32.dll");

    api_function = (Func_SetThreadDpiAwarenessContext)GetProcAddress (
        user32, "SetThreadDpiAwarenessContext");
    FreeLibrary (user32);
    return api_function;
}

int
grab_screenshot (int all_screens, int includeLayeredWindows)
{
    int x = 0, y = 0, width, height;
    HBITMAP bitmap;
    HDC screen, screen_copy;
    BITMAP bmpScreen;
    DWORD dwBmpSize = 0;
    DWORD rop;
    HANDLE dpiAwareness = NULL;
    HANDLE hDIB = NULL;
    char *lpbitmap = NULL;
    HANDLE hFile = NULL;
    DWORD dwSizeofDIB = 0;
    DWORD dwBytesWritten = 0;
    Func_SetThreadDpiAwarenessContext SetThreadDpiAwarenessContext_function;

    /* step 1: create a memory DC large enough to hold the
       entire screen */

    screen = CreateDC (L"DISPLAY", NULL, NULL, NULL);
    screen_copy = CreateCompatibleDC (screen);

    // added in Windows 10 (1607)
    // loaded dynamically to avoid link errors
    SetThreadDpiAwarenessContext_function = get_thread_dpi_api ();
    if (SetThreadDpiAwarenessContext_function != NULL)
        {
            // DPI_AWARENESS_CONTEXT_PER_MONITOR_AWARE =
            // ((DPI_CONTEXT_HANDLE)-3)
            dpiAwareness = SetThreadDpiAwarenessContext_function ((HANDLE)-3);
        }

    if (all_screens)
        {
            x = GetSystemMetrics (SM_XVIRTUALSCREEN);
            y = GetSystemMetrics (SM_YVIRTUALSCREEN);
            width = GetSystemMetrics (SM_CXVIRTUALSCREEN);
            height = GetSystemMetrics (SM_CYVIRTUALSCREEN);
        }
    else
        {
            width = GetDeviceCaps (screen, HORZRES);
            height = GetDeviceCaps (screen, VERTRES);
        }

    if (SetThreadDpiAwarenessContext_function != NULL)
        {
            SetThreadDpiAwarenessContext_function (dpiAwareness);
        }

    bitmap = CreateCompatibleBitmap (screen, width, height);
    if (!bitmap)
        {
            goto error;
        }

    if (!SelectObject (screen_copy, bitmap))
        {
            goto error;
        }

    /* step 2: copy bits into memory DC bitmap */

    rop = SRCCOPY;
    if (includeLayeredWindows)
        {
            rop |= CAPTUREBLT;
        }
    if (!BitBlt (screen_copy, 0, 0, width, height, screen, x, y, rop))
        {
            goto error;
        }

    // Get the BITMAP from the HBITMAP.
    GetObject (bitmap, sizeof (BITMAP), &bmpScreen);

    BITMAPFILEHEADER bmfHeader;
    BITMAPINFOHEADER bi;

    bi.biSize = sizeof (BITMAPINFOHEADER);
    bi.biWidth = bmpScreen.bmWidth;
    bi.biHeight = bmpScreen.bmHeight;
    bi.biPlanes = 1;
    bi.biBitCount = 32;
    bi.biCompression = BI_RGB;
    bi.biSizeImage = 0;
    bi.biXPelsPerMeter = 0;
    bi.biYPelsPerMeter = 0;
    bi.biClrUsed = 0;
    bi.biClrImportant = 0;

    dwBmpSize = ((bmpScreen.bmWidth * bi.biBitCount + 31) / 32) * 4
                * bmpScreen.bmHeight;

    // Starting with 32-bit Windows, GlobalAlloc and LocalAlloc are implemented
    // as wrapper functions that call HeapAlloc using a handle to the process's
    // default heap. Therefore, GlobalAlloc and LocalAlloc have greater
    // overhead than HeapAlloc.
    hDIB = GlobalAlloc (GHND, dwBmpSize);
    lpbitmap = (char *)GlobalLock (hDIB);

    // Gets the "bits" from the bitmap, and copies them into a buffer
    // that's pointed to by lpbitmap.
    GetDIBits (screen, bitmap, 0, (UINT)bmpScreen.bmHeight, lpbitmap,
               (BITMAPINFO *)&bi, DIB_RGB_COLORS);

    // A file is created, this is where we will save the screen capture.
    hFile = CreateFile (L"captureqwsx.bmp", GENERIC_WRITE, 0, NULL,
                        CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);

    // Add the size of the headers to the size of the bitmap to get the total
    // file size.
    dwSizeofDIB
        = dwBmpSize + sizeof (BITMAPFILEHEADER) + sizeof (BITMAPINFOHEADER);

    // Offset to where the actual bitmap bits start.
    bmfHeader.bfOffBits
        = (DWORD)sizeof (BITMAPFILEHEADER) + (DWORD)sizeof (BITMAPINFOHEADER);

    // Size of the file.
    bmfHeader.bfSize = dwSizeofDIB;

    // bfType must always be BM for Bitmaps.
    bmfHeader.bfType = 0x4D42; // BM.

    WriteFile (hFile, (LPSTR)&bmfHeader, sizeof (BITMAPFILEHEADER),
               &dwBytesWritten, NULL);
    WriteFile (hFile, (LPSTR)&bi, sizeof (BITMAPINFOHEADER), &dwBytesWritten,
               NULL);
    WriteFile (hFile, (LPSTR)lpbitmap, dwBmpSize, &dwBytesWritten, NULL);

    // Unlock and Free the DIB from the heap.
    GlobalUnlock (hDIB);
    GlobalFree (hDIB);

    // Close the handle for the file that was created.
    CloseHandle (hFile);
    goto done;

done:

    DeleteDC (screen_copy);
    DeleteDC (screen);

    return 0;
error:
    printf ("screen grab failed");

    DeleteDC (screen_copy);
    DeleteDC (screen);

    return 0;
}

int
main (int argc, char *argv[])
{
    initialise_logger ();
    log_debug ("Received %d arguments", argc);
    WCHAR *filename;
    filename = calloc (MAX_PATH, sizeof (WCHAR *));
    if (filename == NULL){
        log_error("Memory error", 1);
    }
    if (argc == 1)
        {
            log_info ("Received no arguments. Choosing interactive mode.");
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
                    log_error ("Memory Error. Can't allocate memory.", 1);
                }

            for (optind = 1; optind < argc; optind++)
                {
                    if (starts_with (argv[optind], "--"))
                        {
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
                            log_error ("Invalid Argument: %s", 1, argv[optind]);
                        }
                    else
                        {
                            positional_args[positional_args_no] = malloc (
                                (strlen (argv[optind]) + 1) * sizeof (char *));

                            if (positional_args == NULL)
                                {
                                    log_error ("Memory Error.", 1);
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
                        log_error ("Positional argument `file` is missing.", 1);
                    else
                        log_error ("Multiple positional arguments found for `file`.", 1);
                    return 1;
                }


            int nChars = MultiByteToWideChar (CP_UTF8, 0, positional_args[positional_args_no], -1, NULL, 0);
            if (MultiByteToWideChar(CP_UTF8, 0, positional_args[positional_args_no], -1, (WCHAR*)filename, nChars) != nChars){
                log_error("Cannot convert string", 1);
            }

            for (size_t i = 0; i < positional_args_no; i++)
                {
                    free (positional_args[positional_args_no]);
                }
            free (positional_args);
        }
    int output = grab_screenshot (1, 1);
    free (filename);
    return output;
}
