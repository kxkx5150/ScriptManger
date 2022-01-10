#include "ScriptManager.h"
#include "SrcMgr.h"
#include "framework.h"
#include <strsafe.h>

NOTIFYICONDATA g_nid;
HMENU hPopMenu;
int main_window_width = 340;
int main_window_height = 544;
int add_group_height = 176;
struct KeyObj {
    bool alt;
    bool ctrl;
    bool shift;
    TCHAR keycode;
};
KeyObj keyobj = {
    false,
    false,
    false,
    0
};

UINT uMessage = RegisterWindowMessage(L"script_mgr_kxkx5150__japan_kyoto");
#define MAX_LOADSTRING 100
HINSTANCE hInst;
WCHAR szTitle[MAX_LOADSTRING];
WCHAR szWindowClass[MAX_LOADSTRING];
SrcMgr* g_script_manager = nullptr;
HHOOK hHook { NULL };
HWND g_mainhwnd = nullptr;

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
void delete_regkey(const TCHAR* key)
{
    HKEY newValue;
    if (RegOpenKey(HKEY_CURRENT_USER, L"SOFTWARE\\Script_Manager_kxkx5150", &newValue) != ERROR_SUCCESS)
        return;
    RegDeleteValue(newValue, key);
}
BOOL create_shellreg(const TCHAR* parentkey, const TCHAR* key,
    const TCHAR* valname, const TCHAR* val, const TCHAR* optstr, bool createsub = true)
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
    if (RegOpenKey(HKEY_CURRENT_USER, keystr.c_str(), &hKey) != ERROR_SUCCESS || !createsub) {
        Ret = RegCreateKeyEx(HKEY_CURRENT_USER, keystr.c_str(), 0, NULL, REG_OPTION_NON_VOLATILE,
            KEY_ALL_ACCESS, NULL, &hKey, &dwDisposition);
        if (Ret != ERROR_SUCCESS)
            return FALSE;

        std::wstring regstr = val;
        DWORD regvallen = regstr.size() * sizeof(wchar_t);
        if (RegSetValueEx(hKey, valname, 0, REG_SZ,
                (LPBYTE)regstr.c_str(),
                regvallen)
            != ERROR_SUCCESS) {

            RegCloseKey(hKey);
            RegCloseKey(hpkey);
            return FALSE;
        }

        if (!createsub) {
            RegCloseKey(hKey);
            RegCloseKey(hpkey);
            return TRUE;
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

    lResult = RegDeleteKey(hKeyRoot, lpSubKey);
    if (lResult == ERROR_SUCCESS)
        return TRUE;

    lResult = RegOpenKeyEx(hKeyRoot, lpSubKey, 0, KEY_READ, &hKey);
    if (lResult != ERROR_SUCCESS) {
        if (lResult == ERROR_FILE_NOT_FOUND) {
            return TRUE;
        } else {
            return FALSE;
        }
    }

    lpEnd = lpSubKey + lstrlen(lpSubKey);
    if (*(lpEnd - 1) != TEXT('\\')) {
        *lpEnd = TEXT('\\');
        lpEnd++;
        *lpEnd = TEXT('\0');
    }

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
        delete_shellreg(L"SOFTWARE\\Classes\\Directory\\Background\\shell", L"Script_Manager_kxkx5150");

    } else {
        CheckMenuItem(hmenu, ID_MENU_EXPLORERMENU, MF_BYCOMMAND | MFS_CHECKED);
        create_shellreg(L"SOFTWARE\\Classes\\*\\shell", L"Script_Manager_kxkx5150",
            L"", L"Open with Script Manager",
            L"\" \"%1\"");
        create_shellreg(L"SOFTWARE\\Classes\\Directory\\shell", L"Script_Manager_kxkx5150",
            L"", L"Open with Script Manager",
            L"\" \"%V\"");
        create_shellreg(L"SOFTWARE\\Classes\\Directory\\Background\\shell", L"Script_Manager_kxkx5150",
            L"", L"Open with Script Manager",
            L"\"");
    }
}
void toggle_mode(HWND hWnd, int menuid)
{
    HMENU hmenu = GetMenu(hWnd);
    UINT uState = GetMenuState(hmenu, menuid, MF_BYCOMMAND);
    if (uState & MFS_CHECKED) {
        g_script_manager->resize_window(hWnd, false, add_group_height);
        CheckMenuItem(hmenu, menuid, MF_BYCOMMAND | MFS_UNCHECKED);
        create_shellreg(L"SOFTWARE", L"Script_Manager_kxkx5150",
            L"mode", L"search",
            L"", false);

        main_window_height = 358;
    } else {
        g_script_manager->resize_window(hWnd, true, add_group_height);
        CheckMenuItem(hmenu, menuid, MF_BYCOMMAND | MFS_CHECKED);
        create_shellreg(L"SOFTWARE", L"Script_Manager_kxkx5150",
            L"mode", L"add",
            L"", false);

        main_window_height = 544;
    }
}
void toggle_sys_tray(HWND hWnd, int menuid)
{
    HMENU hmenu = GetMenu(hWnd);
    UINT uState = GetMenuState(hmenu, menuid, MF_BYCOMMAND);
    if (!uState) {
        CheckMenuItem(hmenu, menuid, MF_BYCOMMAND | MFS_CHECKED);
        create_shellreg(L"SOFTWARE", L"Script_Manager_kxkx5150",
            L"systray", L"enable",
            L"", false);
        Shell_NotifyIcon(NIM_ADD, &g_nid);
    } else {
        CheckMenuItem(hmenu, menuid, MF_BYCOMMAND | MFS_UNCHECKED);
        create_shellreg(L"SOFTWARE", L"Script_Manager_kxkx5150",
            L"systray", L"disable",
            L"", false);
        Shell_NotifyIcon(NIM_DELETE, &g_nid);
    }
}
void toggle_run_minimize(HWND hWnd, int menuid)
{
    HMENU hmenu = GetMenu(hWnd);
    UINT uState = GetMenuState(hmenu, menuid, MF_BYCOMMAND);
    if (!uState) {
        CheckMenuItem(hmenu, menuid, MF_BYCOMMAND | MFS_CHECKED);
        create_shellreg(L"SOFTWARE", L"Script_Manager_kxkx5150",
            L"run_minimize", L"enable",
            L"", false);
    } else {
        CheckMenuItem(hmenu, menuid, MF_BYCOMMAND | MFS_UNCHECKED);
        create_shellreg(L"SOFTWARE", L"Script_Manager_kxkx5150",
            L"run_minimize", L"disable",
            L"", false);
    }
}
void SetAbsoluteForegroundWindow(HWND hWnd)
{
    int nTargetID, nForegroundID;
    DWORD sp_time;
    nForegroundID = GetWindowThreadProcessId(GetForegroundWindow(), NULL);
    nTargetID = GetWindowThreadProcessId(hWnd, NULL);
    AttachThreadInput(nTargetID, nForegroundID, TRUE);
    SystemParametersInfo(SPI_GETFOREGROUNDLOCKTIMEOUT, 0, &sp_time, 0);
    SystemParametersInfo(SPI_SETFOREGROUNDLOCKTIMEOUT, 0, (LPVOID)0, 0);
    BringWindowToTop(hWnd);
    SystemParametersInfo(SPI_SETFOREGROUNDLOCKTIMEOUT, 0, &sp_time, 0);
    AttachThreadInput(nTargetID, nForegroundID, FALSE);
}
void show_main_window(HWND hWnd, bool tray = false)
{
    HMENU hmenu = GetMenu(hWnd);
    POINT p;
    GetCursorPos(&p);
    ShowWindow(hWnd, SW_SHOWNORMAL);
    int offx = 0, offy = 0;
    if (tray) {
        offx = main_window_width;
        offy = main_window_height + 30;
    } else {
        offy = 60;
    }
    SetWindowPos(hWnd, HWND_TOPMOST, p.x - offx, p.y - offy, 0, 0, SWP_NOSIZE);
    SetAbsoluteForegroundWindow(hWnd);
    UINT uState2 = GetMenuState(hmenu, ID_SCRIPT_ADD, MF_BYCOMMAND);
    if (MFS_CHECKED != uState2) {
        g_script_manager->set_focus_search_editor();
    }
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
    delete[] buffer;
}
void receive_args(HWND main_window_handle, LPARAM lparam)
{
    UINT uMessage = RegisterWindowMessage(L"script_mgr_kxkx5150__japan_kyoto");
    COPYDATASTRUCT* copy_data_structure = { 0 };
    copy_data_structure = (COPYDATASTRUCT*)lparam;
    LPCWSTR arguments = (LPCWSTR)copy_data_structure->lpData;

    if (copy_data_structure->dwData == uMessage) {
        show_main_window(main_window_handle);
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
std::wstring get_regval(HWND hWnd, std::wstring keystr, std::wstring subkeystr)
{
    HKEY hKey;
    if (RegOpenKey(HKEY_CURRENT_USER, keystr.c_str(), &hKey) == ERROR_SUCCESS) {
        std::wstring strValueOfBinDir;
        GetStringRegKey(hKey, subkeystr.c_str(), strValueOfBinDir, L"empty");
        return strValueOfBinDir;
    }
    return L"empty";
}
void get_reg_sttings(HWND hWnd)
{
    HMENU hmenu = GetMenu(hWnd);
    std::wstring str = get_regval(hWnd, L"Software\\Microsoft\\Windows\\CurrentVersion\\Run", L"Script_Manager_kxkx5150");
    if (str != L"empty")
        CheckMenuItem(hmenu, ID_MENU_STARTUP, MF_BYCOMMAND | MFS_CHECKED);

    str = get_regval(hWnd, L"SOFTWARE\\Classes\\*\\shell\\Script_Manager_kxkx5150", L"");
    if (str != L"empty")
        CheckMenuItem(hmenu, ID_MENU_EXPLORERMENU, MF_BYCOMMAND | MFS_CHECKED);

    str = get_regval(hWnd, L"SOFTWARE\\Script_Manager_kxkx5150", L"mode");
    if (str == L"search") {
        toggle_mode(hWnd, ID_SCRIPT_ADD);
    }

    str = get_regval(hWnd, L"SOFTWARE\\Script_Manager_kxkx5150", L"systray");
    if (str == L"disable") {
        toggle_sys_tray(hWnd, ID_MENU_SYSTEMTRAY);
    }

    str = get_regval(hWnd, L"SOFTWARE\\Script_Manager_kxkx5150", L"run_minimize");
    if (str == L"disable") {
        toggle_run_minimize(hWnd, ID_MENU_RUNMINIMIZED);
    }

    str = get_regval(hWnd, L"SOFTWARE\\Script_Manager_kxkx5150", L"shortcut");
    if (str != L"empty") {
        int sval = std::stoi(str);
        if (sval >= 1000) {
            UINT keycode = sval / 1000;
            sval -= keycode * 1000;
            keyobj.keycode = keycode;
            if (sval >= 100) {
                keyobj.ctrl = true;
                sval -= 100;
            }
            if (sval >= 10) {
                keyobj.alt = true;
                sval -= 10;
            }
            if (sval >= 1) {
                keyobj.shift = true;
            }
            OutputDebugString(L"");
        }
    }
}
HWND create_combobox(HWND hParent, int nX, int nY, int nWidth, int nHeight, int id)
{
    return CreateWindow(
        L"COMBOBOX", L"",
        WS_CHILD | WS_VISIBLE | CBS_DROPDOWNLIST | CBS_AUTOHSCROLL | WS_CLIPSIBLINGS | WS_VSCROLL,
        nX, nY, nWidth, nHeight,
        hParent, (HMENU)id, hInst, NULL);
}
HWND create_button(HWND hParent, int nX, int nY, int nWidth, int nHeight, int id, TCHAR* txt)
{
    return CreateWindow(
        L"BUTTON", txt,
        WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON,
        nX, nY, nWidth, nHeight,
        hParent, (HMENU)id, hInst, NULL);
}
void create_shotcut_control(HWND hDlg)
{
    HWND keylisthwnd = CreateWindow(
        L"COMBOBOX", NULL,
        WS_CHILD | WS_VISIBLE | CBS_DROPDOWNLIST | WS_CLIPSIBLINGS | WS_VSCROLL,
        32, 10, 150, 20,
        hDlg, (HMENU)IDD_SHORTCUT_KEY_LISTBOX, hInst, NULL);
    TCHAR i;
    int selidx = 0;
    for (i = L'z'; L'a' <= i; i--) {
        std::wstring aaa = &i;
        aaa[1] = L'\0';
        SendMessage(keylisthwnd, CB_INSERTSTRING, 0, (LPARAM)aaa.c_str());
        if (i == keyobj.keycode) {
            selidx = i - 96;
        }
    }
    SendMessage(keylisthwnd, CB_INSERTSTRING, 0, (LPARAM)L"");
    SendMessage(keylisthwnd, CB_SETCURSEL, selidx, 0);

    HWND ctrlhwnd = create_combobox(hDlg, 32, 45, 150, 20, IDD_SHORTCUT_CTRL_LISTBOX);
    SendMessage(ctrlhwnd, CB_INSERTSTRING, 0, (LPARAM)L"CTRL");
    SendMessage(ctrlhwnd, CB_INSERTSTRING, 0, (LPARAM)L"");
    if (keyobj.ctrl) {
        SendMessage(ctrlhwnd, CB_SETCURSEL, 1, 0);
    } else {
        SendMessage(ctrlhwnd, CB_SETCURSEL, 0, 0);
    }

    HWND althwnd = create_combobox(hDlg, 32, 80, 150, 20, IDD_SHORTCUT_ALT_LISTBOX);
    SendMessage(althwnd, CB_INSERTSTRING, 0, (LPARAM)L"ALT");
    SendMessage(althwnd, CB_INSERTSTRING, 0, (LPARAM)L"");
    SendMessage(althwnd, CB_SETCURSEL, 0, 0);
    if (keyobj.alt) {
        SendMessage(althwnd, CB_SETCURSEL, 1, 0);
    } else {
        SendMessage(althwnd, CB_SETCURSEL, 0, 0);
    }

    HWND shifthwnd = create_combobox(hDlg, 32, 115, 150, 20, IDD_SHORTCUT_SHIFT_LISTBOX);
    SendMessage(shifthwnd, CB_INSERTSTRING, 0, (LPARAM)L"SHIFT");
    SendMessage(shifthwnd, CB_INSERTSTRING, 0, (LPARAM)L"");
    SendMessage(shifthwnd, CB_SETCURSEL, 0, 0);
    if (keyobj.shift) {
        SendMessage(shifthwnd, CB_SETCURSEL, 1, 0);
    } else {
        SendMessage(shifthwnd, CB_SETCURSEL, 0, 0);
    }

    create_button(hDlg, 18, 170, 90, 26, IDD_SHORTCUT_OKBUTTON, (TCHAR*)L"OK");
    create_button(hDlg, 110, 170, 90, 26, IDD_SHORTCUT_CANCELBUTTON, (TCHAR*)L"Cancel");
}
void toggle_shortcut()
{
    bool miniwin = IsIconic(g_mainhwnd);
    if (miniwin) {
        SetTimer(g_mainhwnd, 10, 100, NULL);

    } else {
        PostMessage(g_mainhwnd, WM_SYSCOMMAND, SC_MINIMIZE, 0);
    }
}
void ok_shotcut(HWND hDlg)
{
    HWND keylisthwnd = GetDlgItem(hDlg, IDD_SHORTCUT_KEY_LISTBOX);
    HWND ctrlhwnd = GetDlgItem(hDlg, IDD_SHORTCUT_CTRL_LISTBOX);
    HWND althwnd = GetDlgItem(hDlg, IDD_SHORTCUT_ALT_LISTBOX);
    HWND shifthwnd = GetDlgItem(hDlg, IDD_SHORTCUT_SHIFT_LISTBOX);

    TCHAR keytxt[10];
    GetWindowText(keylisthwnd, keytxt, 10);
    TCHAR ctrltxt[10];
    GetWindowText(ctrlhwnd, ctrltxt, 10);
    TCHAR alttxt[10];
    GetWindowText(althwnd, alttxt, 10);
    TCHAR shifttxt[10];
    GetWindowText(shifthwnd, shifttxt, 10);

    int regval = 0;
    if (keytxt == L"") {
        keyobj.keycode = 0;
    } else {
        keyobj.keycode = *keytxt;
        UINT keycode = keyobj.keycode;
        regval = 1000 * keycode;
    }

    if (_tcslen(ctrltxt) == 0) {
        keyobj.ctrl = false;
    } else {
        keyobj.ctrl = true;
        regval += 100;
    }

    if (_tcslen(alttxt) == 0) {
        keyobj.alt = false;
    } else {
        keyobj.alt = true;
        regval += 10;
    }

    if (_tcslen(shifttxt) == 0) {
        keyobj.shift = false;
    } else {
        keyobj.shift = true;
        regval += 1;
    }

    std::wstring regstr = std::to_wstring(regval);
    create_shellreg(L"SOFTWARE", L"Script_Manager_kxkx5150",
        L"shortcut", regstr.c_str(),
        L"", false);
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
INT_PTR CALLBACK shortcut_dialog_proc(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
    UNREFERENCED_PARAMETER(lParam);
    switch (message) {
    case WM_INITDIALOG: {
        create_shotcut_control(hDlg);
        return (INT_PTR)TRUE;
    }

    case WM_COMMAND:
        if (LOWORD(wParam) == IDD_SHORTCUT_OKBUTTON) {
            ok_shotcut(hDlg);
            EndDialog(hDlg, LOWORD(wParam));
            return (INT_PTR)TRUE;

        } else if (LOWORD(wParam) == IDD_SHORTCUT_CANCELBUTTON) {
            EndDialog(hDlg, LOWORD(wParam));
            return (INT_PTR)TRUE;
        } else if (LOWORD(wParam) == IDD_SHORTCUT_ALT_LISTBOX) {
            int wmEvent = HIWORD(wParam);
            if (wmEvent == LBN_SELCHANGE) {
                HWND ctrlhwnd = GetDlgItem(hDlg, IDD_SHORTCUT_CTRL_LISTBOX);
                HWND althwnd = GetDlgItem(hDlg, IDD_SHORTCUT_ALT_LISTBOX);

                int altidx = SendMessage(althwnd, CB_GETCURSEL, 0, 0);
                if (altidx == 0)
                    break;

                int idx = SendMessage(ctrlhwnd, CB_GETCURSEL, 0, 0);
                if (idx == 0) {
                    SendMessage(ctrlhwnd, CB_SETCURSEL, 1, 0);
                }
            }
        }
        break;
    }
    return (INT_PTR)FALSE;
}

LRESULT CALLBACK LowLevelKeyboardProc(int nCode, WPARAM wParam, LPARAM lParam)
{
    if (wParam == WM_KEYDOWN) {
        KBDLLHOOKSTRUCT* pK = (KBDLLHOOKSTRUCT*)lParam;
        if (pK->vkCode == keyobj.keycode - 32) {
            bool ctrl = GetKeyState(VK_CONTROL) < 0 ? true : false;
            if (keyobj.ctrl == ctrl) {
                bool alt = GetKeyState(VK_MENU) < 0 ? true : false;
                if (keyobj.alt == alt) {
                    bool shift = GetKeyState(VK_SHIFT) < 0 ? true : false;
                    if (keyobj.shift == shift) {
                        toggle_shortcut();
                        return TRUE;
                    }
                }
            }
        }
    }

    return CallNextHookEx(hHook, nCode, wParam, lParam);
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

        case ID_MENU_RUNMINIMIZED: {
            toggle_run_minimize(hWnd, ID_MENU_RUNMINIMIZED);
        } break;

        case ID_MENU_STARTUP: {
            setStartUp(hWnd);
        } break;

        case ID_MENU_EXPLORERMENU: {
            setContextMenu(hWnd);
        } break;

        case ID_MENU_SHORTCUTKEY: {
            DialogBox(hInst, MAKEINTRESOURCE(IDD_DIALOG1), hWnd, shortcut_dialog_proc);

        } break;

        default:
            return DefWindowProc(hWnd, message, wParam, lParam);
        }
    } break;

    case WM_TIMER:
        if (wParam == 1) {
            PostMessage(hWnd, WM_SYSCOMMAND, SC_MINIMIZE, 0);
        } else if (wParam == 10) {
            show_main_window(hWnd);
        }
        KillTimer(hWnd, wParam);

        break;

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
            g_script_manager->clear_search_editor();
            HMENU hmenu = GetMenu(hWnd);
            UINT uState = GetMenuState(hmenu, ID_MENU_SYSTEMTRAY, MF_BYCOMMAND);
            if (!uState)
                return 0;
            ShowWindow(hWnd, SW_HIDE);
        } else {
        }
    } break;

    case WM_TO_TRAY: {
        if (lParam == WM_LBUTTONDOWN) {
            show_main_window(hWnd, true);

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

    case WM_CHAR:
        if (VK_ESCAPE == (TCHAR)wParam) {
            PostMessage(g_mainhwnd, WM_SYSCOMMAND, SC_MINIMIZE, 0);
        }
        break;

    case WM_CREATE: {
        g_mainhwnd = hWnd;
        g_script_manager = new SrcMgr(hWnd, hInst);
        g_script_manager->init();
        g_script_manager->read_setting_csv();
        TCHAR* cmdline = GetCommandLineW();
        g_script_manager->receive_args(1, cmdline);
        createContextMenu();
        create_trayicon(hWnd);
        get_reg_sttings(hWnd);
        hHook = SetWindowsHookEx(WH_KEYBOARD_LL, LowLevelKeyboardProc, 0, 0);

    } break;

    case WM_CLOSE: {
        HMENU hmenu = GetMenu(hWnd);
        UINT uState = GetMenuState(hmenu, ID_MENU_SYSTEMTRAY, MF_BYCOMMAND);
        if (uState) {
            PostMessage(hWnd, WM_SYSCOMMAND, SC_MINIMIZE, 0);
        } else {
            DestroyWindow(hWnd);
        }
    } break;

    case WM_DESTROY: {
        Shell_NotifyIcon(NIM_DELETE, &g_nid);
        delete g_script_manager;
        UnhookWindowsHookEx(hHook);
        PostQuitMessage(0);
    } break;

    default:
        return DefWindowProc(hWnd, message, wParam, lParam);
    }
    return 0;
}
