#include "ScriptManager.h"
#include "SrcMgr.h"
#include "framework.h"

bool g_addgrop = true;
bool g_systray = true;
bool g_startup = false;
bool g_osmenu = true;

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
    wcex.hIcon = LoadIcon(hInstance, MAKEINTRESOURCE(IDI_ICON1));
    wcex.hCursor = LoadCursor(nullptr, IDC_ARROW);
    wcex.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
    wcex.lpszMenuName = MAKEINTRESOURCEW(IDC_RUNPYTHON);
    wcex.lpszClassName = szWindowClass;
    wcex.hIconSm = LoadIcon(wcex.hInstance, MAKEINTRESOURCE(IDI_ICON1));
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
    if (RegOpenKey(HKEY_CURRENT_USER,
            TEXT("Software\\Microsoft\\Windows\\CurrentVersion\\Run"),
            &newValue)
        != ERROR_SUCCESS) {
        OutputDebugString(L"Error2");
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
void toggle_check_menu(HWND hWnd, int menuid)
{
    bool chkflg;
    HMENU hmenu = GetMenu(hWnd);
    UINT uState = GetMenuState(hmenu, menuid, MF_BYCOMMAND);
    if (uState & MFS_CHECKED) {
        chkflg = false;
        CheckMenuItem(hmenu, menuid, MF_BYCOMMAND | MFS_UNCHECKED);
        main_window_height = 215;
    } else {
        chkflg = true;
        CheckMenuItem(hmenu, menuid, MF_BYCOMMAND | MFS_CHECKED);
        main_window_height = 544;
    }

    if (g_script_manager) {
        g_script_manager->resize_window(hWnd, chkflg, add_group_height);
    }
}
void toggle_sys_tray(HWND hWnd, int menuid)
{
    HMENU hmenu = GetMenu(hWnd);
    g_systray = !g_systray;
    if (g_systray) {
        CheckMenuItem(hmenu, menuid, MF_BYCOMMAND | MFS_CHECKED);
        Shell_NotifyIcon(NIM_ADD, &g_nid);
    } else {
        CheckMenuItem(hmenu, menuid, MF_BYCOMMAND | MFS_UNCHECKED);
        Shell_NotifyIcon(NIM_DELETE, &g_nid);
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
    if (!g_systray)
        return;
    g_nid.cbSize = (DWORD)sizeof(NOTIFYICONDATA);
    g_nid.hWnd = hWnd;
    g_nid.uID = IDR_MAINFRAME;
    g_nid.uFlags = NIF_ICON | NIF_MESSAGE | NIF_TIP;
    g_nid.uCallbackMessage = WM_TO_TRAY;
    g_nid.hIcon = LoadIcon(hInst, MAKEINTRESOURCE(IDI_ICON1));
    wcscpy_s(g_nid.szTip, _T("Script Manager"));
    Shell_NotifyIcon(NIM_ADD, &g_nid);
    ShowWindow(hWnd, SW_HIDE);
}
void show_main_window(HWND hWnd)
{
    if (!g_systray)
        return;
    POINT p;
    GetCursorPos(&p);
    SetForegroundWindow(hWnd);
    ShowWindow(hWnd, SW_SHOWNORMAL);
    SetWindowPos(hWnd, NULL, p.x - main_window_width, p.y - main_window_height - 20, 0, 0, SWP_NOSIZE);
}
void createContextMenu()
{
    hPopMenu = CreatePopupMenu();
    AppendMenu(hPopMenu, MF_BYPOSITION | MF_STRING, IDC_RMENU_TERMINAL, L"Terminal");
    AppendMenu(hPopMenu, MF_BYPOSITION | MF_STRING, IDC_RMENU_TERMINAL_AS, L"Terminal (admin)");
    AppendMenu(hPopMenu, MF_SEPARATOR, NULL, _T("SEP"));
    AppendMenu(hPopMenu, MF_BYPOSITION | MF_STRING, IDM_EXIT, L"Exit");
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
void HandleCopyDataEvent(HWND main_window_handle, LPARAM lparam)
{
    UINT uMessage = RegisterWindowMessage(L"script_mgr_kxkx5150__japan_kyoto");
    COPYDATASTRUCT* copy_data_structure = { 0 };
    copy_data_structure = (COPYDATASTRUCT*)lparam;
    LPCWSTR arguments = (LPCWSTR)copy_data_structure->lpData;

    if (copy_data_structure->dwData == uMessage) {
        OutputDebugString(arguments);
        OutputDebugString(L"\n");
    }
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
            toggle_check_menu(hWnd, ID_SCRIPT_ADD);
            break;

        case ID_MENU_SYSTEMTRAY: {
            toggle_sys_tray(hWnd, ID_MENU_SYSTEMTRAY);
        } break;

        case ID_MENU_STARTUP: {
            setStartUp(hWnd);
        } break;

        default:
            return DefWindowProc(hWnd, message, wParam, lParam);
        }
    } break;

    case WM_COPYDATA: {
        HandleCopyDataEvent(hWnd, lParam);

    } break;

    case WM_PAINT: {
        PAINTSTRUCT ps;
        HDC hdc = BeginPaint(hWnd, &ps);
        EndPaint(hWnd, &ps);
    } break;

    case WM_SIZE: {
        if (wParam == SIZE_MINIMIZED) {
            if (!g_systray)
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
        g_script_manager->receive_args();

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
