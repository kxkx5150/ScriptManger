#include "ScriptManager.h"
#include "SrcMgr.h"
#include "framework.h"

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
    hInst = hInstance;
    HWND hWnd = CreateWindow(szWindowClass, szTitle, WS_OVERLAPPED | WS_SYSMENU,
        CW_USEDEFAULT, 0, 340, 544, nullptr, nullptr, hInstance, nullptr);
    if (!hWnd) {
        return FALSE;
    }
    ShowWindow(hWnd, nCmdShow);
    UpdateWindow(hWnd);
    return TRUE;
}
void toggle_check_menu(HWND hWnd, int menuid)
{
    bool chkflg;
    HMENU hmenu = GetMenu(hWnd);
    UINT uState = GetMenuState(hmenu, menuid, MF_BYCOMMAND);
    if (uState & MFS_CHECKED) {
        chkflg = false;
        CheckMenuItem(hmenu, menuid, MF_BYCOMMAND | MFS_UNCHECKED);
    } else {
        chkflg = true;
        CheckMenuItem(hmenu, menuid, MF_BYCOMMAND | MFS_CHECKED);
    }

    if (g_script_manager) {
        g_script_manager->resize_window(hWnd, chkflg);
    }
}
int APIENTRY wWinMain(_In_ HINSTANCE hInstance, _In_opt_ HINSTANCE hPrevInstance,
    _In_ LPWSTR lpCmdLine, _In_ int nCmdShow)
{
    UNREFERENCED_PARAMETER(hPrevInstance);
    UNREFERENCED_PARAMETER(lpCmdLine);

    LoadStringW(hInstance, IDS_APP_TITLE, szTitle, MAX_LOADSTRING);
    LoadStringW(hInstance, IDC_RUNPYTHON, szWindowClass, MAX_LOADSTRING);
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

        case IDM_EXIT:
            DestroyWindow(hWnd);
            break;


        case IDC_COMBO:
		    if (HIWORD(wParam) == CBN_SELCHANGE) {
                g_script_manager->change_select_combobox();
            }
            break;

        case ID_SCRIPT_ADD:
            toggle_check_menu(hWnd, ID_SCRIPT_ADD);
            break;

        case ID_EXE:
            g_script_manager->exe_script();
            break;

        case ID_COMB_DELETE:
            g_script_manager->delete_script();
            break;


            

        default:
            return DefWindowProc(hWnd, message, wParam, lParam);
        }
    } break;

    case WM_PAINT: {
        PAINTSTRUCT ps;
        HDC hdc = BeginPaint(hWnd, &ps);
        EndPaint(hWnd, &ps);
    } break;

    case WM_CTLCOLORSTATIC: {
        static HBRUSH hBrushColor;
        auto rgb = RGB(0, 0xbb, 0xFF);
        if (GetDlgItem(hWnd, IDC_ADDGROUP) == (HWND)lParam) {
            rgb = RGB(0xFF, 0xFF, 0);
        }
        hBrushColor = CreateSolidBrush(rgb);
        return (LRESULT)hBrushColor;
    } break;

    case WM_CREATE: {
        g_script_manager = new SrcMgr(hWnd, hInst);
        g_script_manager->init();
    } break;

    case WM_DESTROY: {
        PostQuitMessage(0);
    } break;

    default:
        return DefWindowProc(hWnd, message, wParam, lParam);
    }
    return 0;
}
