/*------------------------------------------------------------------
/*------------------------------------------------------------------
   DEVCAPS2.C -- Displays Device Capability Information (Version 2)
                 (c) Charles Petzold, 1998
------------------------------------------------------------------*/
#include <windows.h>
#include "resource.h"
void ClientResize1(HWND hWnd, int nWidth, int nHeight)
{
    RECT rcClient, rcWind;
    POINT ptDiff;
    GetClientRect(hWnd, &rcClient);
    GetWindowRect(hWnd, &rcWind);
    ptDiff.x = (rcWind.right - rcWind.left) - rcClient.right;
    ptDiff.y = (rcWind.bottom - rcWind.top) - rcClient.bottom;
    MoveWindow(hWnd, rcWind.left, rcWind.top, nWidth + ptDiff.x, nHeight + ptDiff.y, TRUE);
}
LRESULT CALLBACK WndProc(HWND, UINT, WPARAM, LPARAM);
void DoBasicInfo(HDC, HDC, int, int);
void DoOtherInfo(HDC, HDC, int, int);
void DoBitCodedCaps(HDC, HDC, int, int, int);
typedef struct
{
    int     iMask;
    TCHAR* szDesc;
}
BITS;
#define IDM_DEVMODE      1000
int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance,
    PSTR szCmdLine, int iCmdShow)
{
    static TCHAR szAppName[] = TEXT("DevCaps2");
    HWND         hwnd;
    MSG          msg;
    WNDCLASS     wndclass;
    wndclass.style = CS_HREDRAW | CS_VREDRAW;
    wndclass.lpfnWndProc = WndProc;
    wndclass.cbClsExtra = 0;
    wndclass.cbWndExtra = 0;
    wndclass.hInstance = hInstance;
    wndclass.hIcon = LoadIcon(NULL, IDI_APPLICATION);
    wndclass.hCursor = LoadCursor(NULL, IDC_ARROW);
    wndclass.hbrBackground = (HBRUSH)GetStockObject(WHITE_BRUSH);
    wndclass.lpszMenuName = szAppName;
    wndclass.lpszClassName = szAppName;
    if (!RegisterClass(&wndclass))
    {
        MessageBox(NULL, TEXT("This program requires Windows NT!"),
            szAppName, MB_ICONERROR);
        return 0;
    }
    hwnd = CreateWindow(szAppName, NULL,
        WS_OVERLAPPEDWINDOW,
        CW_USEDEFAULT, CW_USEDEFAULT,
        CW_USEDEFAULT, CW_USEDEFAULT,
        NULL, NULL, hInstance, NULL);
    ShowWindow(hwnd, iCmdShow);
    UpdateWindow(hwnd);
    while (GetMessage(&msg, NULL, 0, 0))
    {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }
    return msg.wParam;
}
LRESULT CALLBACK WndProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam)
{
    static TCHAR            szDevice[32], szWindowText[64];
    static int              cxChar, cyChar, nCurrentDevice = IDM_SCREEN,
        nCurrentInfo = IDM_BASIC;
    static DWORD            dwNeeded, dwReturned;
    static PRINTER_INFO_4* pinfo4;
    static PRINTER_INFO_5* pinfo5;
    DWORD                   i;
    HDC                     hdc, hdcInfo;
    HMENU                   hMenu;
    HANDLE                  hPrint;
    PAINTSTRUCT             ps;
    TEXTMETRIC              tm;
    switch (message)
    {
    case WM_CREATE:
        ClientResize1(hwnd, 500, 200);
        hdc = GetDC(hwnd);
        SelectObject(hdc, GetStockObject(SYSTEM_FIXED_FONT));
        GetTextMetrics(hdc, &tm);
        cxChar = tm.tmAveCharWidth;
        cyChar = tm.tmHeight + tm.tmExternalLeading;
        ReleaseDC(hwnd, hdc);
        // fall through
    case WM_SETTINGCHANGE:
        hMenu = GetSubMenu(GetMenu(hwnd), 0);
        while (GetMenuItemCount(hMenu) > 1)
            DeleteMenu(hMenu, 1, MF_BYPOSITION);
        // Get a list of all local and remote printers
        // 
        // First, find out how large an array we need; this
        //   call will fail, leaving the required size in dwNeeded
        //
        // Next, allocate space for the info array and fill it
        // 
        // Put the printer names on the menu
        //if (GetVersion() & 0x80000000)         // Windows 98
        if (-1)         // Windows 98
        {
            EnumPrinters(PRINTER_ENUM_LOCAL, NULL, 5, NULL,
                0, &dwNeeded, &dwReturned);
            pinfo5 = malloc(dwNeeded);
            EnumPrinters(PRINTER_ENUM_LOCAL, NULL, 5, (PBYTE)pinfo5,
                dwNeeded, &dwNeeded, &dwReturned);
            for (i = 0; i < dwReturned; i++)
            {
                AppendMenu(hMenu, (i + 1) % 16 ? 0 : MF_MENUBARBREAK, i + 1,
                    pinfo5[i].pPrinterName);
            }
            free(pinfo5);
        }
        else                                    // Windows NT
        {
            EnumPrinters(PRINTER_ENUM_LOCAL, NULL, 4, NULL,
                0, &dwNeeded, &dwReturned);
            pinfo4 = malloc(dwNeeded);
            EnumPrinters(PRINTER_ENUM_LOCAL, NULL, 4, (PBYTE)pinfo4,
                dwNeeded, &dwNeeded, &dwReturned);
            for (i = 0; i < dwReturned; i++)
            {
                AppendMenu(hMenu, MF_STRING, i + 1,
                    pinfo4[i].pPrinterName);
            }
            free(pinfo4);
        }
        AppendMenu(hMenu, MF_SEPARATOR, 0, NULL);
        AppendMenu(hMenu, 0, IDM_DEVMODE, TEXT("Properties"));
        wParam = IDM_SCREEN;
        // fall through
    case WM_COMMAND:
        hMenu = GetMenu(hwnd);
        if (LOWORD(wParam) == IDM_SCREEN ||         // IDM_SCREEN & Printers
            LOWORD(wParam) < IDM_DEVMODE)
        {
            CheckMenuItem(hMenu, nCurrentDevice, MF_UNCHECKED);
            nCurrentDevice = LOWORD(wParam);
            CheckMenuItem(hMenu, nCurrentDevice, MF_CHECKED);
        }
        else if (LOWORD(wParam) == IDM_DEVMODE)     // Properties selection
        {
            GetMenuString(hMenu, nCurrentDevice, szDevice,
                sizeof(szDevice) / sizeof(TCHAR), MF_BYCOMMAND);
            if (OpenPrinter(szDevice, &hPrint, NULL))
            {
                PrinterProperties(hwnd, hPrint);
                ClosePrinter(hPrint);
            }
        }
        else                               // info menu items
        {
            CheckMenuItem(hMenu, nCurrentInfo, MF_UNCHECKED);
            nCurrentInfo = LOWORD(wParam);
            CheckMenuItem(hMenu, nCurrentInfo, MF_CHECKED);
        }
        InvalidateRect(hwnd, NULL, TRUE);
        return 0;
    case WM_INITMENUPOPUP:
        if (lParam == 0)
            EnableMenuItem(GetMenu(hwnd), IDM_DEVMODE,
                nCurrentDevice == IDM_SCREEN ? MF_GRAYED : MF_ENABLED);
        return 0;
    case WM_PAINT:
        lstrcpy(szWindowText, TEXT("Device Capabilities: "));
        if (nCurrentDevice == IDM_SCREEN)
        {
            lstrcpy(szDevice, TEXT("DISPLAY"));
            hdcInfo = CreateIC(szDevice, NULL, NULL, NULL);
        }
        else
        {
            hMenu = GetMenu(hwnd);
            GetMenuString(hMenu, nCurrentDevice, szDevice,
                sizeof(szDevice), MF_BYCOMMAND);
            hdcInfo = CreateIC(NULL, szDevice, NULL, NULL);
        }
        lstrcat(szWindowText, szDevice);
        SetWindowText(hwnd, szWindowText);
        hdc = BeginPaint(hwnd, &ps);
        SelectObject(hdc, GetStockObject(SYSTEM_FIXED_FONT));
        if (hdcInfo)
        {
            switch (nCurrentInfo)
            {
            case IDM_BASIC:
                DoBasicInfo(hdc, hdcInfo, cxChar, cyChar);
                break;
            case IDM_OTHER:
            case IDM_CURVE:
            case IDM_LINE:
            case IDM_POLY:
            case IDM_TEXT:
                break;
            }
            DeleteDC(hdcInfo);
        }
        EndPaint(hwnd, &ps);
        return 0;
    case WM_DESTROY:
        PostQuitMessage(0);
        return 0;
    }
    return DefWindowProc(hwnd, message, wParam, lParam);
}
void DoBasicInfo(HDC hdc, HDC hdcInfo, int cxChar, int cyChar)
{
    static struct
    {
        int     nIndex;
        TCHAR* szDesc;
    }
    info[] =
    {
         HORZSIZE,        TEXT("HORZSIZE        Width in millimeters:"),
         VERTSIZE,        TEXT("VERTSIZE        Height in millimeters:"),
         HORZRES,         TEXT("HORZRES         Width in pixels:"),
         VERTRES,         TEXT("VERTRES         Height in raster lines:"),
         //  BITSPIXEL,       TEXT("BITSPIXEL       Color bits per pixel:"),
         //  PLANES,          TEXT("PLANES          Number of color planes:"),
         //  NUMBRUSHES,      TEXT("NUMBRUSHES      Number of device brushes:"),
         //  NUMPENS,         TEXT("NUMPENS         Number of device pens:"),
         //  NUMMARKERS,      TEXT("NUMMARKERS      Number of device markers:"),
         //  NUMFONTS,        TEXT("NUMFONTS        Number of device fonts:"),
         //  NUMCOLORS,       TEXT("NUMCOLORS       Number of device colors:"),
         //  PDEVICESIZE,     TEXT("PDEVICESIZE     Size of device structure:"),
         //  ASPECTX,         TEXT("ASPECTX         Relative width of pixel:"),
         //  ASPECTY,         TEXT("ASPECTY         Relative height of pixel:"),
         //  ASPECTXY,        TEXT("ASPECTXY        Relative diagonal of pixel:"),
          LOGPIXELSX,      TEXT("LOGPIXELSX      Horizontal dots per inch:"),
          LOGPIXELSY,      TEXT("LOGPIXELSY      Vertical dots per inch:"),
          //  SIZEPALETTE,     TEXT("SIZEPALETTE     Number of palette entries:"),
          //  NUMRESERVED,     TEXT("NUMRESERVED     Reserved palette entries:"),
          //  COLORRES,        TEXT("COLORRES        Actual color resolution:"),
           PHYSICALWIDTH,   TEXT("PHYSICALWIDTH   Printer page pixel width:"),
           PHYSICALHEIGHT,  TEXT("PHYSICALHEIGHT  Printer page pixel height:"),
           PHYSICALOFFSETX, TEXT("PHYSICALOFFSETX Printer page x offset:"),
           PHYSICALOFFSETY, TEXT("PHYSICALOFFSETY Printer page y offset:")
    };
    int   i;
    TCHAR szBuffer[80];
    for (i = 0; i < sizeof(info) / sizeof(info[0]); i++)
        TextOut(hdc, cxChar, (i + 1) * cyChar, szBuffer,
            wsprintf(szBuffer, TEXT("%-45s%8d"), info[i].szDesc,
                GetDeviceCaps(hdcInfo, info[i].nIndex)));
}
