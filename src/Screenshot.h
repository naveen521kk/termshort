#pragma once
#include <windows.h>

// should be second
#include <wchar.h>

int grab_screenshot (int all_screens, int includeLayeredWindows,
                     WCHAR *filename);
