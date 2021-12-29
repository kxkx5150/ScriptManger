#define _CRT_SECURE_NO_WARNINGS
#include "SrcMgr.h"
#include "resource.h"
#include <CommCtrl.h>
#include <crtdbg.h>
#include <shlobj.h>
#include <string>
#include <tchar.h>
#include <windows.h>
#pragma comment(lib, "Comctl32.lib")

extern SrcMgr* g_srcmgr;
SrcMgr::SrcMgr(HWND hWnd, HINSTANCE hInst)
{
    m_prnthwnd = hWnd;
    m_hInst = hInst;
}
SrcMgr::~SrcMgr()
{
    DeleteObject(m_hFont);
    DeleteObject(m_shFont);
    DeleteObject(m_sshFont);
}
void SrcMgr::init()
{
    create_control();
    set_font();
    auto comcount = m_commands.size();
    if (comcount > 0) {
        select_combobox_item(0);
    }
}
void SrcMgr::add_script(TCHAR* name, TCHAR* exe, TCHAR* batpath, TCHAR* pypath, TCHAR* pydir, TCHAR* args, int windowopt)
{
    Command command;
    command.name = name;
    command.exe = exe;
    command.batpath = batpath;
    command.pypath = pypath;
    command.pydir = pydir;
    command.args = args;
    command.windowopt = windowopt;
    add_combobox_item(command.name);
    m_commands.push_back(command);
}
void SrcMgr::exe_script(int exeidx)
{
    auto comcount = m_commands.size();
    if (comcount == 0) {
        return;
    }

    if (exeidx == -1) {
        exeidx = SendMessage(m_combohwnd, CB_GETCURSEL, 0, 0);
    }
    Command command = m_commands[exeidx];

    TCHAR* args = new TCHAR[4096];
    TCHAR* workdir = new TCHAR[4096];
    TCHAR cmd[] = L"cmd.exe";
    args[0] = 0;
    workdir[0] = 0;
    wcscat(args, L"/k ");

    if (_tcslen(command.batpath) > 0) {
        wcscat(args, command.batpath);
        wcscat(args, L" & python ");
        wcscat(args, command.pypath);
        wcscat(workdir, command.pydir);
    } else {
        wcscat(args, L"python ");
        wcscat(args, command.pypath);
        wcscat(workdir, command.pydir);
    }

    ShellExecute(NULL, L"open", cmd, args, workdir, m_commands[exeidx].windowopt);
}
void SrcMgr::create_control()
{
    m_hFont = create_font(16);
    m_shFont = create_font(13);
    m_sshFont = create_font(10);
    m_lsthFont = create_font(12);

    m_combohwnd = create_combobox(m_prnthwnd, 6, 2, 190, 200, IDC_COMBO);
    m_run_btnhwnd = create_button(m_prnthwnd, 202, 1, 60, 26, ID_EXE, (TCHAR*)L"Run");

    m_dropgrouphwnd = create_group(m_prnthwnd, 2, 34, 263, 120, (TCHAR*)L"Drop argument files");
    m_dd_listhwnd = create_dorp_listbox(m_dropgrouphwnd, 2, 22, 240, 106);
    m_addarghwnd = create_button(m_dropgrouphwnd, 242, 18, 18, 18, IDC_ADD_ARG_BUTTON, (TCHAR*)L"+");
    m_delarghwnd = create_button(m_dropgrouphwnd, 242, 38, 18, 18, IDC_DEL_ARG_BUTTON, (TCHAR*)L"-");
    m_uparghwnd = create_button(m_dropgrouphwnd, 242, 58, 18, 22, IDC_UP_ARG_BUTTON, (TCHAR*)L"Å™");
    m_downarghwnd = create_button(m_dropgrouphwnd, 242, 84, 18, 22, IDC_DOWN_ARG_BUTTON, (TCHAR*)L"Å´");
    DragAcceptFiles(m_dd_listhwnd, TRUE);
    SetWindowSubclass(m_dropgrouphwnd, &SubclassWindowProc, 0, 0);
    SetWindowSubclass(m_dd_listhwnd, &SubclassWindowProc, 0, 0);
    SetWindowLongPtr(m_dd_listhwnd, GWLP_USERDATA, (LONG)this);

    m_addgrouphwnd = create_group(m_prnthwnd, 4, 162, 259, 320, (TCHAR*)L"Add script", IDC_ADDGROUP);
    m_name_edithwnd = create_edittext(m_addgrouphwnd, 4, 18, 120, 18, IDC_NAMETEXT, (TCHAR*)L"");
    m_cmd_edithwnd = create_edittext(m_addgrouphwnd, 4, 46, 120, 18, IDC_EXE_EDIT, (TCHAR*)L"python.exe");

    m_src_pathhwnd = create_edittext(m_addgrouphwnd, 4, 76, 220, 18, IDC_SRCPATH, (TCHAR*)L"");
    m_src_filebtn = create_button(m_addgrouphwnd, 228, 75, 26, 19, IDC_SRC_BUTTON, (TCHAR*)L"...");

    m_venv_chkboxhwnd = create_checkbox(m_addgrouphwnd, 4, 102, 76, 18, IDC_VENVCHK, (TCHAR*)L"Use venv");
    m_venv_pathhwnd = create_edittext(m_addgrouphwnd, 4, 120, 220, 18, IDC_VENVPATH, (TCHAR*)L"");
    m_venv_dirbtn = create_button(m_addgrouphwnd, 228, 119, 26, 19, IDC_VENV_BUTTON, (TCHAR*)L"...");

    m_dir_pathhwnd = create_edittext(m_addgrouphwnd, 4, 161, 220, 18, IDC_WDIRPATH, (TCHAR*)L"");
    m_working_dirbtn = create_button(m_addgrouphwnd, 228, 160, 26, 19, IDC_DIR_BUTTON, (TCHAR*)L"...");
    SetWindowSubclass(m_addgrouphwnd, &SubclassWindowProc, 0, 0);
    SetWindowLongPtr(m_addgrouphwnd, GWLP_USERDATA, (LONG)this);

    m_stor_arg_chkboxhwnd = create_checkbox(m_addgrouphwnd, 4, 196, 126, 18, IDC_STORE_ARGS_CHKBOX, (TCHAR*)L"Store Arguments");

    create_cmd_radiobutton(m_addgrouphwnd, 8, 230, 120, 25);
    m_add_btnhwnd = create_button(m_addgrouphwnd, 20, 270, 200, 32, IDC_ADD_BUTTON, (TCHAR*)L"Add");

    Edit_SetCueBannerText(m_name_edithwnd, L"Name");
    Edit_SetCueBannerText(m_cmd_edithwnd, L"*.exe");
    Edit_SetCueBannerText(m_venv_pathhwnd, L"venv activate script path");
    Edit_SetCueBannerText(m_src_pathhwnd, L"Script path");
    Edit_SetCueBannerText(m_dir_pathhwnd, L"Working directory");
    SendMessage(m_venv_chkboxhwnd, BM_SETCHECK, BST_CHECKED, 0);
    EnableWindow(m_add_btnhwnd, false);
}
HFONT SrcMgr::create_font(int fontsize)
{
    return CreateFont(fontsize, 0, 0, 0,
        FW_NORMAL, FALSE, FALSE, 0,
        SHIFTJIS_CHARSET,
        OUT_DEFAULT_PRECIS,
        CLIP_DEFAULT_PRECIS,
        DEFAULT_QUALITY, DEFAULT_PITCH, L"MS Gothic");
}
void SrcMgr::set_font()
{
    SendMessage(m_combohwnd, WM_SETFONT, (WPARAM)m_hFont, MAKELPARAM(FALSE, 0));
    SendMessage(m_run_btnhwnd, WM_SETFONT, (WPARAM)m_hFont, MAKELPARAM(FALSE, 0));
    SendMessage(m_dropgrouphwnd, WM_SETFONT, (WPARAM)m_shFont, MAKELPARAM(FALSE, 0));
    SendMessage(m_dd_listhwnd, WM_SETFONT, (WPARAM)m_lsthFont, MAKELPARAM(FALSE, 0));
    SendMessage(m_addgrouphwnd, WM_SETFONT, (WPARAM)m_shFont, MAKELPARAM(FALSE, 0));
    SendMessage(m_name_edithwnd, WM_SETFONT, (WPARAM)m_shFont, MAKELPARAM(FALSE, 0));
    SendMessage(m_name_edithwnd, WM_SETFONT, (WPARAM)m_shFont, MAKELPARAM(FALSE, 0));
    SendMessage(m_cmd_edithwnd, WM_SETFONT, (WPARAM)m_shFont, MAKELPARAM(FALSE, 0));

    SendMessage(m_venv_chkboxhwnd, WM_SETFONT, (WPARAM)m_shFont, MAKELPARAM(FALSE, 0));
    SendMessage(m_venv_pathhwnd, WM_SETFONT, (WPARAM)m_shFont, MAKELPARAM(FALSE, 0));
    SendMessage(m_src_pathhwnd, WM_SETFONT, (WPARAM)m_shFont, MAKELPARAM(FALSE, 0));
    SendMessage(m_dir_pathhwnd, WM_SETFONT, (WPARAM)m_shFont, MAKELPARAM(FALSE, 0));
    SendMessage(m_showcmdhwnd, WM_SETFONT, (WPARAM)m_shFont, MAKELPARAM(FALSE, 0));
    SendMessage(m_hidecmdhwnd, WM_SETFONT, (WPARAM)m_shFont, MAKELPARAM(FALSE, 0));
    SendMessage(m_venv_dirbtn, WM_SETFONT, (WPARAM)m_sshFont, MAKELPARAM(FALSE, 0));
    SendMessage(m_src_filebtn, WM_SETFONT, (WPARAM)m_sshFont, MAKELPARAM(FALSE, 0));
    SendMessage(m_working_dirbtn, WM_SETFONT, (WPARAM)m_sshFont, MAKELPARAM(FALSE, 0));
    SendMessage(m_stor_arg_chkboxhwnd, WM_SETFONT, (WPARAM)m_shFont, MAKELPARAM(FALSE, 0));


    SendMessage(m_addarghwnd, WM_SETFONT, (WPARAM)m_hFont, MAKELPARAM(FALSE, 0));
    SendMessage(m_delarghwnd, WM_SETFONT, (WPARAM)m_hFont, MAKELPARAM(FALSE, 0));
}
HWND SrcMgr::create_combobox(HWND hParent, int nX, int nY, int nWidth, int nHeight, int id)
{
    return CreateWindow(
        L"COMBOBOX", L"",
        WS_CHILD | WS_VISIBLE | CBS_DROPDOWNLIST | CBS_AUTOHSCROLL | WS_CLIPSIBLINGS | WS_VSCROLL,
        nX, nY, nWidth, nHeight,
        hParent, (HMENU)id, m_hInst, NULL);
}
HWND SrcMgr::create_button(HWND hParent, int nX, int nY, int nWidth, int nHeight, int id, TCHAR* txt)
{
    return CreateWindow(
        L"BUTTON", txt,
        WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON,
        nX, nY, nWidth, nHeight,
        hParent, (HMENU)id, m_hInst, NULL);
}
HWND SrcMgr::create_dorp_listbox(HWND hParent, int nX, int nY, int nWidth, int nHeight)
{
    return CreateWindow(
        L"LISTBOX", NULL,
        WS_VISIBLE | WS_CHILD | LBS_NOTIFY | WS_VSCROLL,
        nX, nY, nWidth, nHeight,
        hParent, (HMENU)IDS_LISTBOX, m_hInst, NULL);
}
HWND SrcMgr::create_group(HWND hParent, int nX, int nY, int nWidth, int nHeight, TCHAR* txt, int id)
{
    return CreateWindowEx(0,
        L"BUTTON", txt,
        WS_CHILD | WS_VISIBLE | BS_GROUPBOX | WS_EX_CLIENTEDGE,
        nX, nY, nWidth, nHeight,
        hParent, (HMENU)id, m_hInst, NULL);
}
HWND SrcMgr::create_edittext(HWND hParent, int nX, int nY, int nWidth, int nHeight, int id, TCHAR* txt)
{
    return CreateWindow(
        TEXT("EDIT"), txt,
        WS_CHILD | WS_VISIBLE | WS_BORDER | ES_LEFT | ES_AUTOHSCROLL,
        nX, nY, nWidth, nHeight,
        hParent, (HMENU)id, m_hInst, NULL);
}
HWND SrcMgr::create_checkbox(HWND hParent, int nX, int nY, int nWidth, int nHeight, int id, TCHAR* txt)
{
    return CreateWindow(
        L"button", txt,
        WS_VISIBLE | WS_CHILD | BS_AUTOCHECKBOX,
        nX, nY, nWidth, nHeight,
        hParent, (HMENU)id, m_hInst, NULL);
}
void SrcMgr::create_cmd_radiobutton(HWND hParent, int nX, int nY, int nWidth, int nHeight)
{
    m_showcmdhwnd = CreateWindow(
        L"BUTTON", L"Show prompt",
        WS_VISIBLE | WS_CHILD | BS_AUTORADIOBUTTON | WS_GROUP,
        nX, nY, nWidth, nHeight,
        hParent, (HMENU)IDC_SHOWCMD, m_hInst, NULL);

    m_hidecmdhwnd = CreateWindow(
        L"BUTTON", L"Hide prompt",
        WS_VISIBLE | WS_CHILD | BS_AUTORADIOBUTTON,
        nX + 120, nY, nWidth, nHeight,
        hParent, (HMENU)IDC_HIDECMD, m_hInst, NULL);
    SendMessage(m_showcmdhwnd, BM_SETCHECK, BST_CHECKED, 0);
}

void SrcMgr::add_combobox_item(const TCHAR* pszBuf)
{
    SendMessage(m_combohwnd, CB_ADDSTRING, 0, (LPARAM)pszBuf);
}
void SrcMgr::select_combobox_item(int index)
{
    m_activeidx = index;
    SendMessage(m_combohwnd, CB_SETCURSEL, index, 0);
}
int SrcMgr::get_combobox_index(TCHAR* itemstr)
{
    WPARAM index = SendMessage(m_combohwnd, CB_FINDSTRINGEXACT, -1, (LPARAM)itemstr);
    return index;
}
void SrcMgr::resize_window(HWND hWnd, bool addmenu)
{
    RECT rc;
    GetWindowRect(hWnd, &rc);
    int cx = rc.right - rc.left;
    int cy = rc.bottom - rc.top;
    int addarea = 260;

    if (addmenu) {
        SetWindowPos(hWnd, HWND_TOP, 0, 0, cx, cy + addarea, SWP_NOMOVE);
    } else {
        SetWindowPos(hWnd, HWND_TOP, 0, 0, cx, cy - addarea, SWP_NOMOVE);
    }
}
void SrcMgr::check_input_path()
{
    TCHAR srcpathbuf[MAX_PATH];
    GetWindowText(m_src_pathhwnd, srcpathbuf, MAX_PATH);
    DWORD dwAttrib = GetFileAttributes(srcpathbuf);
    if (dwAttrib != INVALID_FILE_ATTRIBUTES && !(dwAttrib & FILE_ATTRIBUTE_DIRECTORY)) {
        EnableWindow(m_add_btnhwnd, true);
    } else {
        EnableWindow(m_add_btnhwnd, false);
    }
}
void SrcMgr::click_add_script()
{
    TCHAR namebuf[MAX_PATH] = { '\0' };
    GetWindowText(m_name_edithwnd, namebuf, MAX_PATH);
    if (_tcslen(namebuf) == 0) {
        wcscpy_s(namebuf, _countof(namebuf), L"no_name");
    }

    int cmbidx = get_combobox_index(namebuf);
    if (cmbidx != -1) {
        for (size_t i = 1; i < INT_MAX; i++) {
            std::wstring namestr = namebuf;
            namestr = namestr + std::to_wstring(i);
            TCHAR* namewchar = &namestr[0];
            cmbidx = get_combobox_index(namewchar);
            if (cmbidx == -1) {
                wcscpy_s(namebuf, _countof(namebuf), namestr.c_str());
                break;
            }
        }
    }

    TCHAR exebuf[MAX_PATH] = { '\0' };
    GetWindowText(m_cmd_edithwnd, exebuf, MAX_PATH);
    if (_tcslen(exebuf) == 0) {
        wcscpy_s(exebuf, _countof(exebuf), L"python.exe");
    }

    TCHAR* scriptname = new TCHAR[MAX_PATH];
    wcscpy_s(scriptname, MAX_PATH, namebuf);
    TCHAR* exename = new TCHAR[MAX_PATH];
    wcscpy_s(exename, MAX_PATH, exebuf);
    TCHAR* srcpathbuf = new TCHAR[MAX_PATH];
    GetWindowText(m_src_pathhwnd, srcpathbuf, MAX_PATH);
    TCHAR* vpathbuf = new TCHAR[MAX_PATH];
    GetWindowText(m_venv_pathhwnd, vpathbuf, MAX_PATH);
    TCHAR* dirpathbuf = new TCHAR[MAX_PATH];
    GetWindowText(m_dir_pathhwnd, dirpathbuf, MAX_PATH);

    TCHAR* argsbuf = new TCHAR[MAX_PATH];
    argsbuf[0] = '\0';

    int wopt = 0;
    if (SendMessage(m_showcmdhwnd, BM_GETCHECK, 0, 0) == BST_CHECKED) {
        wopt = SW_SHOWNORMAL;
    } else {
        wopt = SW_HIDE;
    }

    add_script(
        scriptname,
        exename,
        vpathbuf,
        srcpathbuf,
        dirpathbuf,
        argsbuf,
        wopt);
}

void SrcMgr::trim_tchar(TCHAR* pText, TCHAR* wsBuf)
{
    TCHAR* p = pText;
    TCHAR *pTop = NULL, *pEnd = NULL;
    while ((*p != L'\0') && (*p != L'\r') && (*p != L'\n')) {
        if ((*p != L'\t') && (*p != L' ') && (*p != L'Å@')) {
            // êÊì™Çï€ë∂
            if (!pTop) {
                pTop = p;
            }
            pEnd = NULL;
        } else {
            if (!pEnd) {
                pEnd = p;
            }
        }
        p++;
    }

    if (!pEnd) {
        pEnd = p;
    }

    if (pTop && pEnd) {
        int nLength = (pEnd - pTop);
        memcpy(wsBuf, pTop, nLength << 1);
        wsBuf[nLength] = L'\0';
    } else {
        wsBuf[0] = L'\0';
    }
}
void SrcMgr::add_arg_txt(HWND hDlg)
{
    HWND ehwnd = GetDlgItem(hDlg, IDC_ARG_EDIT);
    TCHAR orgtxt[MAX_PATH];
    TCHAR txt[MAX_PATH];
    GetWindowText(ehwnd, orgtxt, MAX_PATH);
    trim_tchar(orgtxt, txt);
    if (_tcslen(txt) > 0) {
        SendMessage(m_dd_listhwnd, LB_ADDSTRING, 0, (LPARAM)txt);
    }
}
void SrcMgr::click_add_arg()
{
    DialogBox(m_hInst, MAKEINTRESOURCE(IDD_DIALOGBAR), m_prnthwnd, add_arg_proc);
}
void SrcMgr::click_del_arg()
{
    SendMessage(m_dd_listhwnd, LB_DELETESTRING,
        SendMessage(m_dd_listhwnd, LB_GETCURSEL, 0, 0), 0);
}
void SrcMgr::click_up_arg()
{
    int exeidx = SendMessage(m_dd_listhwnd, LB_GETCURSEL, 0, 0);
    if (exeidx < 1)
        return;

    TCHAR tmptxt[MAX_PATH];
    SendMessage(m_dd_listhwnd, LB_GETTEXT, exeidx - 1, (LPARAM)tmptxt);
    TCHAR txt[MAX_PATH];
    SendMessage(m_dd_listhwnd, LB_GETTEXT, exeidx, (LPARAM)txt);

    SendMessage(m_dd_listhwnd, LB_INSERTSTRING, exeidx - 1, (LPARAM)txt);
    SendMessage(m_dd_listhwnd, LB_DELETESTRING, exeidx, 0);
    SendMessage(m_dd_listhwnd, LB_DELETESTRING, exeidx, 0);
    SendMessage(m_dd_listhwnd, LB_INSERTSTRING, exeidx, (LPARAM)tmptxt);
    SendMessage(m_dd_listhwnd, LB_SETCURSEL, exeidx - 1, 0);
}
void SrcMgr::click_down_arg()
{
    int exeidx = SendMessage(m_dd_listhwnd, LB_GETCURSEL, 0, 0);
    if (exeidx == -1)
        return;
    int items = SendMessage(m_dd_listhwnd, LB_GETCOUNT, 0, 0);
    if (exeidx == items - 1)
        return;

    TCHAR tmptxt[MAX_PATH];
    SendMessage(m_dd_listhwnd, LB_GETTEXT, exeidx + 1, (LPARAM)tmptxt);
    TCHAR txt[MAX_PATH];
    SendMessage(m_dd_listhwnd, LB_GETTEXT, exeidx, (LPARAM)txt);

    SendMessage(m_dd_listhwnd, LB_DELETESTRING, exeidx + 1, 0);
    SendMessage(m_dd_listhwnd, LB_INSERTSTRING, exeidx + 1, (LPARAM)txt);
    SendMessage(m_dd_listhwnd, LB_DELETESTRING, exeidx, 0);
    SendMessage(m_dd_listhwnd, LB_INSERTSTRING, exeidx, (LPARAM)tmptxt);
    SendMessage(m_dd_listhwnd, LB_SETCURSEL, exeidx + 1, 0);
}
void SrcMgr::drop_files_into_listbox(HDROP hdrop)
{
    TCHAR filepath[MAX_PATH];
    int num = DragQueryFile(hdrop, -1, NULL, 0);
    for (int i = 0; i < num; i++) {
        DragQueryFile(hdrop, i, filepath, sizeof(filepath) / sizeof(TCHAR));
        SendMessage(m_dd_listhwnd, LB_ADDSTRING, 0, (LPARAM)filepath);
    }
    DragFinish(hdrop);
}
void SrcMgr::open_file_dialog(HWND hwnd, HWND pathhwnd, const TCHAR* filtertxt)
{
    TCHAR szFileName[MAX_PATH];
    TCHAR szFileTitle[MAX_PATH];
    szFileTitle[0] = '\0';
    szFileName[0] = '\0';

    static OPENFILENAME ofn;
    ofn.lStructSize = sizeof(OPENFILENAME);
    ofn.hwndOwner = hwnd;
    ofn.lpstrFilter = filtertxt;
    ofn.nFilterIndex = 1;
    ofn.lpstrFile = szFileName;
    ofn.nMaxFile = MAX_PATH;
    ofn.nMaxFileTitle = _MAX_FNAME + _MAX_EXT;
    ofn.lpstrInitialDir = NULL;
    ofn.lpstrFileTitle = szFileTitle;
    ofn.Flags = OFN_EXPLORER | OFN_ENABLESIZING | OFN_FILEMUSTEXIST | 0;

    if (GetOpenFileName(&ofn)) {
        SetWindowText(pathhwnd, szFileName);

        if (pathhwnd == m_src_pathhwnd) {
            TCHAR srcpathbuf[MAX_PATH];
            GetWindowText(pathhwnd, srcpathbuf, MAX_PATH);
            std::wstring dirstr = srcpathbuf;
            dirstr = dirstr.substr(0, dirstr.find_last_of(L"/\\"));
            SetWindowText(g_srcmgr->m_dir_pathhwnd, dirstr.c_str());

            std::wstring vevnstr = L"\\venv";
            vevnstr = dirstr + vevnstr;

            DWORD dwAttrib = GetFileAttributes(vevnstr.c_str());
            if (dwAttrib != INVALID_FILE_ATTRIBUTES && (dwAttrib & FILE_ATTRIBUTE_DIRECTORY)) {
                SetWindowText(g_srcmgr->m_venv_pathhwnd, vevnstr.c_str());
            }
        }
    }
}
void SrcMgr::open_directory_dialog(HWND hwnd, HWND dirhwnd)
{
    BROWSEINFO bi;
    ZeroMemory(&bi, sizeof(bi));
    TCHAR szDisplayName[MAX_PATH];
    szDisplayName[0] = '\0';

    bi.hwndOwner = NULL;
    bi.pidlRoot = NULL;
    bi.pszDisplayName = szDisplayName;
    bi.lpszTitle = _T("Please select a folder");
    bi.ulFlags = BIF_RETURNONLYFSDIRS;
    bi.lParam = NULL;
    bi.iImage = 0;

    LPITEMIDLIST pidl = SHBrowseForFolder(&bi);
    TCHAR szPathName[MAX_PATH];
    if (NULL != pidl) {
        BOOL bRet = SHGetPathFromIDList(pidl, szPathName);
        if (FALSE == bRet)
            return;
        SetWindowText(dirhwnd, szPathName);
    }
}
INT_PTR CALLBACK add_arg_proc(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
    UNREFERENCED_PARAMETER(lParam);
    switch (message) {
    case WM_INITDIALOG:
        return (INT_PTR)TRUE;

    case WM_COMMAND:
        switch (LOWORD(wParam)) {

        case IDC_ARG_OK_BUTTON: {
            g_srcmgr->add_arg_txt(hDlg);
            EndDialog(hDlg, LOWORD(wParam));
            return (INT_PTR)TRUE;
        } break;

        case IDC_ARG_CANCEL_BUTTON: {
            EndDialog(hDlg, LOWORD(wParam));
            return (INT_PTR)TRUE;
        } break;

        default:
            break;
        }
    }
    return (INT_PTR)FALSE;
}
LRESULT CALLBACK SubclassWindowProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam, UINT_PTR uIdSubclass, DWORD_PTR dwRefData)
{
    switch (uMsg) {
    case WM_DROPFILES: {
        HDROP hdrop = (HDROP)wParam;
        g_srcmgr->drop_files_into_listbox(hdrop);
    } break;

    case WM_COMMAND: {
        switch (LOWORD(wParam)) {
        case IDC_VENV_BUTTON: {
            g_srcmgr->open_file_dialog(hWnd,
                g_srcmgr->m_venv_pathhwnd,
                L"Script (*.bat)\0*.bat\0All Files (*.*)\0*.*\0");
        } break;

        case IDC_SRC_BUTTON: {
            g_srcmgr->open_file_dialog(hWnd,
                g_srcmgr->m_src_pathhwnd,
                L"Python Script (*.py)\0*.py\0All Files (*.*)\0*.*\0");
        } break;

        case IDC_DIR_BUTTON: {
            g_srcmgr->open_directory_dialog(hWnd, g_srcmgr->m_dir_pathhwnd);
        } break;

        case IDC_VENVCHK: {
            if (BST_CHECKED == SendMessage(g_srcmgr->m_venv_chkboxhwnd, BM_GETCHECK, 0, 0)) {
                EnableWindow(g_srcmgr->m_venv_pathhwnd, TRUE);
                EnableWindow(g_srcmgr->m_venv_dirbtn, TRUE);
            } else {
                EnableWindow(g_srcmgr->m_venv_pathhwnd, FALSE);
                EnableWindow(g_srcmgr->m_venv_dirbtn, FALSE);
            }
        } break;

        case IDC_NAMETEXT:
        case IDC_VENVPATH:
        case IDC_SRCPATH:
        case IDC_WDIRPATH: {
            switch (HIWORD(wParam)) {
            case EN_CHANGE:
                g_srcmgr->check_input_path();
                break;
            }
        } break;

        case IDC_ADD_BUTTON: {
            g_srcmgr->click_add_script();
        } break;

        case IDC_ADD_ARG_BUTTON: {
            g_srcmgr->click_add_arg();
        } break;

        case IDC_DEL_ARG_BUTTON: {
            g_srcmgr->click_del_arg();
        } break;

        case IDC_UP_ARG_BUTTON: {
            g_srcmgr->click_up_arg();
        } break;

        case IDC_DOWN_ARG_BUTTON: {
            g_srcmgr->click_down_arg();
        } break;
        }
    } break;

    case WM_NCDESTROY:
        RemoveWindowSubclass(hWnd, SubclassWindowProc, uIdSubclass);
        break;
    }
    return DefSubclassProc(hWnd, uMsg, wParam, lParam);
}
