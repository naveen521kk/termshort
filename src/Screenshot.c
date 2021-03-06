#include "Screenshot.h"
#include <stdio.h>
#include <windows.h>

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
grab_screenshot (int all_screens, int includeLayeredWindows, WCHAR *filename)
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
    hFile = CreateFile (filename, GENERIC_WRITE, 0, NULL, CREATE_ALWAYS,
                        FILE_ATTRIBUTE_NORMAL, NULL);

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
