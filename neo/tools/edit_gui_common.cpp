#include "edit_gui_common.h"

//https://docs.microsoft.com/en-us/cpp/porting/modifying-winver-and-win32-winnt

float GetWindowScalingFactor(HWND window)
{
    float scaling_factor = 1.0f;

#if (WINVER == 0x0A00) // Windows 10 
    UINT dpi = GetDpiForWindow(window);
    scaling_factor = static_cast<float>(dpi) / 96.0f;
#else
    HDC hdc = GetDC(window);
    int LogicalScreenHeight = GetDeviceCaps(hdc, VERTRES);
    int PhysicalScreenHeight = GetDeviceCaps(hdc, DESKTOPVERTRES);
    scaling_factor = (float)PhysicalScreenHeight / (float)LogicalScreenHeight;
#endif

    return scaling_factor;// 1.25 = 125%
}


