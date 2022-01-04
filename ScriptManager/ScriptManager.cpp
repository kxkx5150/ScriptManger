#include "ScriptManager.h"
#include "SrcMgr.h"
#include "framework.h"
#include <strsafe.h>

NOTIFYICONDATA g_nid;
HMENU hPopMenu;
int main_window_width = 340;
int main_window_height = 544;
int add_group_height = 176;

UINT uMessage = RegisterWindowMessage(L"script_mgr_kxkx5150__japan_kyoto");
#define MAX_LOADSTRING 100
HINSTANCE hInst;
WCHAR szTitle[MAX_LOADSTRING];
WCHAR szWindowClass[MAX_LOADSTRING];
SrcMgr* g_script_manager = nullptr;
LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);

ATOM MyRegisterClass(HINSTANCE hInstance)
{
    WNDCLASSEXW wcex;
    wcex.cbSize = sizeof(WNDCLASSEX);
    wcex.style = CS_HREDRAW | CS_VREDRAW;
    wcex.lpfnWndProc = WndProc;
    wcex.cbClsExtra = 0;
    wcex.cbWndExtra = 0;
    wcex.hInstance = hInstance;
    wcex.hIcon = LoadIcon(hInstance, MAKEINTRESOURCE(IDI_ICON2));
    wcex.hCursor = LoadCursor(nullptr, IDC_ARROW);
    wcex.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
    wcex.lpszMenuName = MAKEINTRESOURCEW(IDC_RUNPYTHON);
    wcex.lpszClassName = szWindowClass;
    wcex.hIconSm = LoadIcon(wcex.hInstance, MAKEINTRESOURCE(IDI_ICON2));
    return RegisterClassExW(&wcex);
}
BOOL InitInstance(HINSTANCE hInstance, int nCmdShow)
{
    RECT rect;
    HWND hDeskWnd = GetDesktopWindow();
    GetWindowRect(hDeskWnd, &rect);
    int left = (rect.right - rect.left) / 2 - main_window_width / 2;
    int top = (rect.bottom - rect.top) / 2 - main_window_height / 2;

    hInst = hInstance;
    HWND hWnd = CreateWindow(szWindowClass, szTitle,
        WS_OVERLAPPED | WS_SYSMENU | WS_MINIMIZEBOX,
        left, top, main_window_width, main_window_height, nullptr, nullptr, hInstance, nullptr);
    if (!hWnd) {
        return FALSE;
    }

    //nCmdShow = SW_MINIMIZE;
    ShowWindow(hWnd, nCmdShow);
    UpdateWindow(hWnd);
    return TRUE;
}
int setStartUp(HWND hWnd)
{
    TCHAR szPath[MAX_PATH];
    DWORD pathLen = GetModuleFileName(NULL, szPath, MAX_PATH);
    if (pathLen == 0) {
        return -1;
    }
    HKEY newValue;
    std::wstring keystr = L"Software\\Microsoft\\Windows\\CurrentVersion\\Run";
    if (RegOpenKey(HKEY_CURRENT_USER, keystr.c_str(), &newValue) != ERROR_SUCCESS) {
        return -1;
    }

    HMENU hmenu = GetMenu(hWnd);
    UINT uState = GetMenuState(hmenu, ID_MENU_STARTUP, MF_BYCOMMAND);
    if (uState & MFS_CHECKED) {
        RegDeleteValue(newValue, TEXT("Script_Manager_kxkx5150"));
        CheckMenuItem(hmenu, ID_MENU_STARTUP, MF_BYCOMMAND | MFS_UNCHECKED);

    } else {
        DWORD pathLenInBytes = pathLen * sizeof(*szPath);
        if (RegSetValueEx(newValue,
                TEXT("Script_Manager_kxkx5150"),
                0,
                REG_SZ,
                (LPBYTE)szPath,
                pathLenInBytes)
            != ERROR_SUCCESS) {
            RegCloseKey(newValue);
            return -1;
        }
        CheckMenuItem(hmenu, ID_MENU_STARTUP, MF_BYCOMMAND | MFS_CHECKED);
    }

    RegCloseKey(newValue);
    return 0;
}
BOOL create_shellreg(const TCHAR* parentkey, const TCHAR* key, const TCHAR* optstr)
{
    TCHAR szPath[MAX_PATH];
    DWORD pathLen = GetModuleFileName(NULL, szPath, MAX_PATH);
    if (pathLen == 0) {
        return -1;
    }
    HKEY hpkey;
    if (RegOpenKey(HKEY_CURRENT_USER, parentkey, &hpkey) != ERROR_SUCCESS) {
        return -1;
    }

    HKEY hKey;
    DWORD Ret;
    DWORD dwDisposition;
    std::wstring keystr = parentkey;
    keystr += L"\\";
    keystr += key;
    if (RegOpenKey(HKEY_CURRENT_USER, keystr.c_str(), &hKey) != ERROR_SUCCESS) {
        Ret = RegCreateKeyEx(HKEY_CURRENT_USER, keystr.c_str(), 0, NULL, REG_OPTION_NON_VOLATILE,
            KEY_ALL_ACCESS, NULL, &hKey, &dwDisposition);
        if (Ret != ERROR_SUCCESS)
            return FALSE;

        std::wstring regstr = L"Open with Script Manager";
        DWORD regvallen = regstr.size() * sizeof(wchar_t);
        if (RegSetValueEx(hKey, NULL, 0, REG_SZ,
                (LPBYTE)regstr.c_str(),
                regvallen)
            != ERROR_SUCCESS) {

            RegCloseKey(hKey);
            RegCloseKey(hpkey);
            return -1;
        }

        HKEY hcKey;
        std::wstring cmdstr = keystr;
        cmdstr += L"\\";
        cmdstr += L"command";
        Ret = RegCreateKeyEx(HKEY_CURRENT_USER, cmdstr.c_str(), 0, NULL, REG_OPTION_NON_VOLATILE,
            KEY_ALL_ACCESS, NULL, &hcKey, &dwDisposition);
        if (Ret != ERROR_SUCCESS)
            return FALSE;

        std::wstring exepath = L"\"";
        exepath += szPath;
        exepath += optstr;
        DWORD exepathlen = exepath.size() * sizeof(wchar_t);
        RegSetValueEx(hcKey, NULL, 0, REG_EXPAND_SZ,
            (LPBYTE)exepath.c_str(),
            exepathlen);

        RegCloseKey(hcKey);
        RegCloseKey(hKey);
        RegCloseKey(hpkey);
        return TRUE;
    }

    RegCloseKey(hpkey);
    return TRUE;
}
BOOL RegDelnodeRecurse(HKEY hKeyRoot, LPTSTR lpSubKey)
{
    LPTSTR lpEnd;
    LONG lResult;
    DWORD dwSize;
    TCHAR szName[MAX_PATH];
    HKEY hKey;
    FILETIME ftWrite;

    // First, see if we can delete the key without having
    // to recurse.

    lResult = RegDeleteKey(hKeyRoot, lpSubKey);

    if (lResult == ERROR_SUCCESS)
        return TRUE;

    lResult = RegOpenKeyEx(hKeyRoot, lpSubKey, 0, KEY_READ, &hKey);

    if (lResult != ERROR_SUCCESS) {
        if (lResult == ERROR_FILE_NOT_FOUND) {
            printf("Key not found.\n");
            return TRUE;
        } else {
            printf("Error opening key.\n");
            return FALSE;
        }
    }

    // Check for an ending slash and add one if it is missing.

    lpEnd = lpSubKey + lstrlen(lpSubKey);

    if (*(lpEnd - 1) != TEXT('\\')) {
        *lpEnd = TEXT('\\');
        lpEnd++;
        *lpEnd = TEXT('\0');
    }

    // Enumerate the keys

    dwSize = MAX_PATH;
    lResult = RegEnumKeyEx(hKey, 0, szName, &dwSize, NULL,
        NULL, NULL, &ftWrite);

    if (lResult == ERROR_SUCCESS) {
        do {

            StringCchCopy(lpEnd, MAX_PATH * 2, szName);

            if (!RegDelnodeRecurse(hKeyRoot, lpSubKey)) {
                break;
            }

            dwSize = MAX_PATH;

            lResult = RegEnumKeyEx(hKey, 0, szName, &dwSize, NULL,
                NULL, NULL, &ftWrite);

        } while (lResult == ERROR_SUCCESS);
    }

    lpEnd--;
    *lpEnd = TEXT('\0');

    RegCloseKey(hKey);

    // Try again to delete the key.

    lResult = RegDeleteKey(hKeyRoot, lpSubKey);

    if (lResult == ERROR_SUCCESS)
        return TRUE;

    return FALSE;
}
void delete_shellreg(const TCHAR* parentkey, const TCHAR* key)
{
    std::wstring s = parentkey;
    s += L"\\";
    s += key;
    TCHAR szDelKey[MAX_PATH * 2];
    wcscpy_s(szDelKey, MAX_PATH, s.c_str());
    RegDelnodeRecurse(HKEY_CURRENT_USER, szDelKey);
}
void setContextMenu(HWND hWnd)
{
    HMENU hmenu = GetMenu(hWnd);
    UINT uState = GetMenuState(hmenu, ID_MENU_EXPLORERMENU, MF_BYCOMMAND);
    if (uState & MFS_CHECKED) {
        CheckMenuItem(hmenu, ID_MENU_EXPLORERMENU, MF_BYCOMMAND | MFS_UNCHECKED);
        delete_shellreg(L"SOFTWARE\\Classes\\*\\shell", L"Script_Manager_kxkx5150");
        delete_shellreg(L"SOFTWARE\\Classes\\Directory\\shell", L"Script_Manager_kxkx5150");
    } else {
        CheckMenuItem(hmenu, ID_MENU_EXPLORERMENU, MF_BYCOMMAND | MFS_CHECKED);
        create_shellreg(L"SOFTWARE\\Classes\\*\\shell", L"Script_Manager_kxkx5150", L"\" \"%1\"");
        create_shellreg(L"SOFTWARE\\Classes\\Directory\\shell", L"Script_Manager_kxkx5150", L"\" \"%V\"");
    }
}
void toggle_mode(HWND hWnd, int menuid)
{
    HMENU hmenu = GetMenu(hWnd);
    UINT uState = GetMenuState(hmenu, menuid, MF_BYCOMMAND);
    if (uState & MFS_CHECKED) {
        g_script_manager->resize_window(hWnd, false, add_group_height);
        CheckMenuItem(hmenu, menuid, MF_BYCOMMAND | MFS_UNCHECKED);
        main_window_height = 215;
    } else {
        g_script_manager->resize_window(hWnd, true, add_group_height);
        CheckMenuItem(hmenu, menuid, MF_BYCOMMAND | MFS_CHECKED);
        main_window_height = 544;
    }
}
void toggle_sys_tray(HWND hWnd, int menuid)
{
    HMENU hmenu = GetMenu(hWnd);
    UINT uState = GetMenuState(hmenu, menuid, MF_BYCOMMAND);
    if (!uState) {
        CheckMenuItem(hmenu, menuid, MF_BYCOMMAND | MFS_CHECKED);
        Shell_NotifyIcon(NIM_ADD, &g_nid);
    } else {
        CheckMenuItem(hmenu, menuid, MF_BYCOMMAND | MFS_UNCHECKED);
        Shell_NotifyIcon(NIM_DELETE, &g_nid);
    }
}
void show_main_window(HWND hWnd, int ofx = 0, int ofy = 0)
{
    HMENU hmenu = GetMenu(hWnd);
    UINT uState = GetMenuState(hmenu, ID_MENU_SYSTEMTRAY, MF_BYCOMMAND);
    if (!uState)
        return;
    POINT p;
    GetCursorPos(&p);
    SetForegroundWindow(hWnd);
    ShowWindow(hWnd, SW_SHOWNORMAL);
    SetWindowPos(hWnd, NULL, p.x - main_window_width + ofx, p.y - main_window_height - 20 + ofy, 0, 0, SWP_NOSIZE);
}
void send_args(HWND mainhwnd)
{
    std::wstring s = ::GetCommandLine();
    TCHAR* buffer = new TCHAR[s.length() + 1];
    wcscpy_s(buffer, s.length() + 1, s.c_str());
    COPYDATASTRUCT data_to_send = { 0 };
    data_to_send.dwData = uMessage;
    data_to_send.cbData = (DWORD)8191;
    data_to_send.lpData = buffer;
    SendMessage(mainhwnd, WM_COPYDATA, 0, (LPARAM)&data_to_send);
}
void receive_args(HWND main_window_handle, LPARAM lparam)
{
    UINT uMessage = RegisterWindowMessage(L"script_mgr_kxkx5150__japan_kyoto");
    COPYDATASTRUCT* copy_data_structure = { 0 };
    copy_data_structure = (COPYDATASTRUCT*)lparam;
    LPCWSTR arguments = (LPCWSTR)copy_data_structure->lpData;

    if (copy_data_structure->dwData == uMessage) {
        show_main_window(main_window_handle, 180, 480);
        g_script_manager->receive_args(1, arguments);
    }
}
int APIENTRY wWinMain(_In_ HINSTANCE hInstance, _In_opt_ HINSTANCE hPrevInstance,
    _In_ LPWSTR lpCmdLine, _In_ int nCmdShow)
{
    UNREFERENCED_PARAMETER(hPrevInstance);
    UNREFERENCED_PARAMETER(lpCmdLine);
    LoadStringW(hInstance, IDS_APP_TITLE, szTitle, MAX_LOADSTRING);
    LoadStringW(hInstance, IDC_RUNPYTHON, szWindowClass, MAX_LOADSTRING);

    HWND mainhwnd = FindWindow(szWindowClass, szTitle);
    if (mainhwnd != NULL) {
        SetForegroundWindow(mainhwnd);
        ShowWindow(mainhwnd, SW_SHOWNORMAL);
        send_args(mainhwnd);
        return FALSE;
    }

    MyRegisterClass(hInstance);
    if (!InitInstance(hInstance, nCmdShow)) {
        return FALSE;
    }

    HACCEL hAccelTable = LoadAccelerators(hInstance, MAKEINTRESOURCE(IDC_RUNPYTHON));
    MSG msg;

    while (GetMessage(&msg, nullptr, 0, 0)) {
        if (!TranslateAccelerator(msg.hwnd, hAccelTable, &msg)) {
            TranslateMessage(&msg);
            DispatchMessage(&msg);
        }
    }

    return (int)msg.wParam;
}
void create_trayicon(HWND hWnd)
{
    HMENU hmenu = GetMenu(hWnd);
    UINT uState = GetMenuState(hmenu, ID_MENU_SYSTEMTRAY, MF_BYCOMMAND);
    if (!uState)
        return;
    g_nid.cbSize = (DWORD)sizeof(NOTIFYICONDATA);
    g_nid.hWnd = hWnd;
    g_nid.uID = IDR_MAINFRAME;
    g_nid.uFlags = NIF_ICON | NIF_MESSAGE | NIF_TIP;
    g_nid.uCallbackMessage = WM_TO_TRAY;
    g_nid.hIcon = LoadIcon(hInst, MAKEINTRESOURCE(IDI_ICON2));
    wcscpy_s(g_nid.szTip, _T("Script Manager"));
    Shell_NotifyIcon(NIM_ADD, &g_nid);
    ShowWindow(hWnd, SW_HIDE);
}
void createContextMenu()
{
    hPopMenu = CreatePopupMenu();
    AppendMenu(hPopMenu, MF_BYPOSITION | MF_STRING, IDC_RMENU_TERMINAL, L"Terminal");
    AppendMenu(hPopMenu, MF_BYPOSITION | MF_STRING, IDC_RMENU_TERMINAL_AS, L"Terminal (admin)");
    AppendMenu(hPopMenu, MF_SEPARATOR, NULL, _T("SEP"));
    AppendMenu(hPopMenu, MF_BYPOSITION | MF_STRING, IDM_EXIT, L"Exit");
}

LONG GetStringRegKey(HKEY hKey, const std::wstring& strValueName, std::wstring& strValue, const std::wstring& strDefaultValue)
{
    strValue = strDefaultValue;
    WCHAR szBuffer[512];
    DWORD dwBufferSize = sizeof(szBuffer);
    ULONG nError;
    nError = RegQueryValueExW(hKey, strValueName.c_str(), 0, NULL, (LPBYTE)szBuffer, &dwBufferSize);
    if (ERROR_SUCCESS == nError) {
        strValue = szBuffer;
    }
    return nError;
}
bool get_regval(HWND hWnd, std::wstring keystr, std::wstring subkeystr)
{
    HKEY hKey;
    if (RegOpenKey(HKEY_CURRENT_USER, keystr.c_str(), &hKey) == ERROR_SUCCESS) {
        std::wstring strValueOfBinDir;
        GetStringRegKey(hKey, subkeystr.c_str(), strValueOfBinDir, L"empty");
        if (strValueOfBinDir != L"empty") {
            return true;
        }
    }
    return false;
}
void get_reg_sttings(HWND hWnd)
{
    HMENU hmenu = GetMenu(hWnd);
    bool flg = get_regval(hWnd, L"Software\\Microsoft\\Windows\\CurrentVersion\\Run", L"Script_Manager_kxkx5150");
    if (flg)
        CheckMenuItem(hmenu, ID_MENU_STARTUP, MF_BYCOMMAND | MFS_CHECKED);

    flg = get_regval(hWnd, L"SOFTWARE\\Classes\\*\\shell\\Script_Manager_kxkx5150", L"");
    if (flg)
        CheckMenuItem(hmenu, ID_MENU_EXPLORERMENU, MF_BYCOMMAND | MFS_CHECKED);

}
INT_PTR CALLBACK About(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
    UNREFERENCED_PARAMETER(lParam);
    switch (message) {
    case WM_INITDIALOG:
        return (INT_PTR)TRUE;

    case WM_COMMAND:
        if (LOWORD(wParam) == IDOK || LOWORD(wParam) == IDCANCEL) {
            EndDialog(hDlg, LOWORD(wParam));
            return (INT_PTR)TRUE;
        }
        break;
    }
    return (INT_PTR)FALSE;
}
LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
    switch (message) {

    case WM_COMMAND: {
        switch (LOWORD(wParam)) {

        case IDM_ABOUT:
            DialogBox(hInst, MAKEINTRESOURCE(IDD_ABOUTBOX), hWnd, About);
            break;

        case IDC_RMENU_TERMINAL:
            ShellExecute(NULL, L"", L"wt.exe", L"", L"", SW_SHOWNORMAL);
            break;

        case IDC_RMENU_TERMINAL_AS:
            ShellExecute(NULL, L"runas", L"wt.exe", L"", L"", SW_SHOWNORMAL);
            break;

        case IDM_EXIT:
            DestroyWindow(hWnd);
            break;

        case ID_SCRIPT_ADD:
            toggle_mode(hWnd, ID_SCRIPT_ADD);
            break;

        case ID_MENU_SYSTEMTRAY: {
            toggle_sys_tray(hWnd, ID_MENU_SYSTEMTRAY);
        } break;

        case ID_MENU_STARTUP: {
            setStartUp(hWnd);
        } break;

        case ID_MENU_EXPLORERMENU: {
            setContextMenu(hWnd);
        } break;

        default:
            return DefWindowProc(hWnd, message, wParam, lParam);
        }
    } break;

    case WM_COPYDATA: {
        receive_args(hWnd, lParam);

    } break;

    case WM_PAINT: {
        PAINTSTRUCT ps;
        HDC hdc = BeginPaint(hWnd, &ps);
        EndPaint(hWnd, &ps);
    } break;

    case WM_SIZE: {
        if (wParam == SIZE_MINIMIZED) {
            HMENU hmenu = GetMenu(hWnd);
            UINT uState = GetMenuState(hmenu, ID_MENU_SYSTEMTRAY, MF_BYCOMMAND);
            if (!uState)
                return 0;
            ShowWindow(hWnd, SW_HIDE);
        }
    } break;

    case WM_TO_TRAY: {
        if (lParam == WM_LBUTTONDOWN) {
            show_main_window(hWnd);

        } else if (lParam == WM_RBUTTONUP) {
            POINT p;
            GetCursorPos(&p);
            SetForegroundWindow(hWnd);
            TrackPopupMenu(hPopMenu,
                TPM_RIGHTALIGN | TPM_LEFTBUTTON,
                p.x, p.y, 0, hWnd, NULL);
        }
    } break;

    case WM_CTLCOLORSTATIC: {
        static HBRUSH hBrushColor;
        auto rgb = RGB(0, 0xbb, 0xFF);
        if (GetDlgItem(hWnd, IDC_ADDGROUP) == (HWND)lParam) {
            rgb = RGB(0xFF, 0xFF, 0);
        } else if (GetDlgItem(hWnd, IDC_SEARCHGROUP) == (HWND)lParam) {
            rgb = RGB(0, 0xCF, 0xBB);
        }
        hBrushColor = CreateSolidBrush(rgb);
        return (LRESULT)hBrushColor;
    } break;

    case WM_CREATE: {
        g_script_manager = new SrcMgr(hWnd, hInst);
        g_script_manager->init();
        g_script_manager->read_setting_csv();
        TCHAR* cmdline = GetCommandLineW();
        g_script_manager->receive_args(1, cmdline);
        get_reg_sttings(hWnd);
        createContextMenu();
        create_trayicon(hWnd);

    } break;

        //case WM_CLOSE: {
        //    ShowWindow(hWnd, SW_HIDE);
        //} break;

    case WM_DESTROY: {
        Shell_NotifyIcon(NIM_DELETE, &g_nid);
        PostQuitMessage(0);
    } break;

    default:
        return DefWindowProc(hWnd, message, wParam, lParam);
    }
    return 0;
}
