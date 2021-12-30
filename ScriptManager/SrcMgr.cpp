#include "SrcMgr.h"
#include "resource.h"
#include <CommCtrl.h>
#include <shlobj.h>
#include <tchar.h>
#pragma comment(lib, "Comctl32.lib")

SrcMgr* g_srcmgr;
SrcMgr::SrcMgr(HWND hWnd, HINSTANCE hInst)
{
    m_prnthwnd = hWnd;
    m_hInst = hInst;
    g_srcmgr = this;
}
SrcMgr::~SrcMgr()
{
    DeleteObject(m_hFont);
    DeleteObject(m_shFont);
    DeleteObject(m_sshFont);
    DeleteObject(m_lsthFont);
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
void SrcMgr::change_select_combobox()
{
    int exeidx = SendMessage(m_combohwnd, CB_GETCURSEL, 0, 0);
    if (exeidx == -1) {
        reset_script_value();
        EnableWindow(m_run_btnhwnd, false);
        EnableWindow(m_delete_btnhwnd, false);
    } else {
        set_script_value(exeidx);
        EnableWindow(m_run_btnhwnd, true);
        EnableWindow(m_delete_btnhwnd, true);
        EnableWindow(m_update_btnhwnd, true);
    }
}
void SrcMgr::set_script_value(int exeidx)
{
    Command cmd = m_commands[exeidx];
    SetWindowText(m_name_edithwnd, cmd.name);
    SetWindowText(m_cmd_edithwnd, cmd.exe);

    SetWindowText(m_venv_pathhwnd, cmd.batpath);
    if (0 < _tcslen(cmd.batpath)) {
        SendMessage(m_venv_chkboxhwnd, BM_SETCHECK, BST_CHECKED, 0);
    } else {
        SendMessage(m_venv_chkboxhwnd, BM_SETCHECK, BST_UNCHECKED, 0);
    }
    change_venv_checkbox();

    SetWindowText(m_src_pathhwnd, cmd.pypath);
    change_script_path();
    SetWindowText(m_dir_pathhwnd, cmd.pydir);

    if (cmd.windowopt == SW_SHOWNORMAL) {
        SendMessage(m_showcmdhwnd, BM_SETCHECK, BST_CHECKED, 0);
        SendMessage(m_hidecmdhwnd, BM_SETCHECK, BST_UNCHECKED, 0);
    } else {
        SendMessage(m_showcmdhwnd, BM_SETCHECK, BST_UNCHECKED, 0);
        SendMessage(m_hidecmdhwnd, BM_SETCHECK, BST_CHECKED, 0);
    }

    if (0 < _tcslen(cmd.args)) {
        SendMessage(m_stor_arg_chkboxhwnd, BM_SETCHECK, BST_CHECKED, 0);
    } else {
        SendMessage(m_stor_arg_chkboxhwnd, BM_SETCHECK, BST_UNCHECKED, 0);
    }

    SendMessage(m_dd_listhwnd, LB_RESETCONTENT, 0, 0);
    if (0 < _tcslen(cmd.args)) {
        TCHAR* buftmp = new TCHAR[8191];
        buftmp[0] = '\0';
        TCHAR delim[] = L"\n";
        std::wstring s = std::wstring(cmd.args);
        std::wstring item;
        for (TCHAR ch : s) {
            if (ch == delim[0]) {
                if (!item.empty())
                    SendMessage(m_dd_listhwnd, LB_ADDSTRING, 0, (LPARAM)item.c_str());
                item.clear();
            } else {
                item += ch;
            }
        }
        if (!item.empty())
            SendMessage(m_dd_listhwnd, LB_ADDSTRING, 0, (LPARAM)item.c_str());
    }
}
void SrcMgr::reset_script_value()
{
    SetWindowText(m_name_edithwnd, L"");
    SetWindowText(m_cmd_edithwnd, L"python.exe");
    SendMessage(m_venv_chkboxhwnd, BM_SETCHECK, BST_UNCHECKED, 0);
    SetWindowText(m_venv_pathhwnd, L"");
    change_venv_checkbox();
    SetWindowText(m_src_pathhwnd, L"");
    change_script_path();
    SetWindowText(m_dir_pathhwnd, L"");
    SendMessage(m_showcmdhwnd, BM_SETCHECK, BST_CHECKED, 0);
    SendMessage(m_hidecmdhwnd, BM_SETCHECK, BST_UNCHECKED, 0);
    SendMessage(m_stor_arg_chkboxhwnd, BM_SETCHECK, BST_UNCHECKED, 0);
    SendMessage(m_dd_listhwnd, LB_RESETCONTENT, 0, 0);
    SendMessage(m_combohwnd, CB_SETCURSEL, -1, 0);
}
void SrcMgr::change_venv_checkbox()
{
    if (BST_CHECKED == SendMessage(m_venv_chkboxhwnd, BM_GETCHECK, 0, 0)) {
        EnableWindow(m_venv_pathhwnd, TRUE);
        EnableWindow(m_venv_dirbtn, TRUE);
    } else {
        EnableWindow(m_venv_pathhwnd, FALSE);
        EnableWindow(m_venv_dirbtn, FALSE);
    }
}
void SrcMgr::change_script_path()
{
    TCHAR srcpathbuf[MAX_PATH];
    GetWindowText(m_src_pathhwnd, srcpathbuf, MAX_PATH);
    trim_tchar(srcpathbuf);
    if (_tcslen(srcpathbuf) == 0) {
        EnableWindow(m_add_btnhwnd, false);
        EnableWindow(m_update_btnhwnd, false);
    } else {
        EnableWindow(m_add_btnhwnd, true);
        int exeidx = SendMessage(m_combohwnd, CB_GETCURSEL, 0, 0);
        if (exeidx == -1) {
            EnableWindow(m_update_btnhwnd, false);
        } else {
            EnableWindow(m_update_btnhwnd, true);
        }
    }

    //DWORD dwAttrib = GetFileAttributes(srcpathbuf);
    //if (dwAttrib != INVALID_FILE_ATTRIBUTES && !(dwAttrib & FILE_ATTRIBUTE_DIRECTORY)) {
    //    EnableWindow(m_add_btnhwnd, true);
    //} else {
    //    EnableWindow(m_add_btnhwnd, false);
    //}
}
void SrcMgr::delete_script(int exeidx)
{
    auto comcount = m_commands.size();
    if (comcount == 0)
        return;
    if (exeidx == -1)
        exeidx = SendMessage(m_combohwnd, CB_GETCURSEL, 0, 0);
    if (exeidx == -1)
        return;
    SendMessage(m_combohwnd, CB_DELETESTRING, exeidx, 0);
    delete_command(exeidx);
    reset_script_value();
}
void SrcMgr::delete_command(int exeidx)
{
    Command cmd = m_commands[exeidx];
    delete[] cmd.name;
    delete[] cmd.exe;
    delete[] cmd.batpath;
    delete[] cmd.pypath;
    delete[] cmd.pydir;
    delete[] cmd.args;
    delete[] cmd.cmd;
    m_commands.erase(m_commands.begin() + exeidx);
}
void SrcMgr::add_script(TCHAR* name, TCHAR* exe, TCHAR* batpath, TCHAR* pypath, TCHAR* pydir, 
    TCHAR* args, TCHAR* cmd, int windowopt, int index)
{
    Command command;
    command.name = name;
    command.exe = exe;
    command.batpath = batpath;
    command.pypath = pypath;
    command.pydir = pydir;
    command.args = args;
    command.cmd = cmd;
    command.windowopt = windowopt;

    if (index == -1) {
        SendMessage(m_combohwnd, CB_ADDSTRING, 0, (LPARAM)command.name);
        m_commands.push_back(command);
        reset_script_value();
    } else {
        SendMessage(m_combohwnd, CB_INSERTSTRING, index, (LPARAM)command.name);
        SendMessage(m_combohwnd, CB_SETCURSEL, index, 0);
        delete_command(index);
        m_commands.insert(m_commands.begin() + index, command);
    }
}
void SrcMgr::replace_string(TCHAR* strbuf, int maxlen, std::wstring sword, std::wstring rword)
{
    std::wstring sstr = std::wstring(strbuf);
    std::wstring::size_type Pos(sstr.find(sword));
    while (Pos != std::string::npos) {
        sstr.replace(Pos, sword.length(), rword);
        Pos = sstr.find(sword, Pos + rword.length());
    }
    wcscpy_s(strbuf, maxlen, sstr.c_str());
}
void SrcMgr::exe_script(int exeidx)
{
    auto comcount = m_commands.size();
    if (comcount == 0)
        return;
    if (exeidx == -1)
        exeidx = SendMessage(m_combohwnd, CB_GETCURSEL, 0, 0);

    Command command = m_commands[exeidx];
    TCHAR* args = new TCHAR[8191];
    args[0] = '\0';
    wcscat_s(args, 8191, L"/k");
    if (_tcslen(command.batpath) > 0) {
        wcscat_s(args, 8191, L" ");
        wcscat_s(args, 8191, command.batpath);
        wcscat_s(args, 8191, L" & ");
    } else {
        wcscat_s(args, 8191, L" ");
    }
    wcscat_s(args, 8191, command.exe);
    wcscat_s(args, 8191, L" ");
    wcscat_s(args, 8191, command.pypath);

    if (0 < _tcslen(command.args)) {
        wcscat_s(args, 8191, L" ");
        TCHAR* buftmp = new TCHAR[8191];
        wcscpy_s(buftmp, 8191, command.args);
        replace_string(buftmp, 8191, L"\n", L"\ ");
        wcscat_s(args, 8191, buftmp);
    }

    ShellExecute(NULL, L"open", command.cmd, args, command.pydir, m_commands[exeidx].windowopt);
    delete[] args;
}
void SrcMgr::create_control()
{
    m_hFont = create_font(16);
    m_shFont = create_font(13);
    m_sshFont = create_font(10);
    m_lsthFont = create_font(12);

    m_combohwnd = create_combobox(m_prnthwnd, 6, 2, 180, 200, IDC_COMBO);
    m_run_btnhwnd = create_button(m_prnthwnd, 190, 1, 76, 26, ID_EXE, (TCHAR*)L"Run");
    m_delete_btnhwnd = create_button(m_prnthwnd, 269, 1, 54, 26, ID_COMB_DELETE, (TCHAR*)L"Delete");

    m_dropgrouphwnd = create_group(m_prnthwnd, 2, 34, 320, 120, (TCHAR*)L"Drop argument files");
    m_dd_listhwnd = create_dorp_listbox(m_dropgrouphwnd, 2, 22, 299, 106);
    m_addarghwnd = create_button(m_dropgrouphwnd, 300, 18, 18, 18, IDC_ADD_ARG_BUTTON, (TCHAR*)L"+");
    m_delarghwnd = create_button(m_dropgrouphwnd, 300, 38, 18, 18, IDC_DEL_ARG_BUTTON, (TCHAR*)L"-");
    m_uparghwnd = create_button(m_dropgrouphwnd, 300, 58, 18, 22, IDC_UP_ARG_BUTTON, (TCHAR*)L"ª");
    m_downarghwnd = create_button(m_dropgrouphwnd, 300, 84, 18, 22, IDC_DOWN_ARG_BUTTON, (TCHAR*)L"«");
    DragAcceptFiles(m_dd_listhwnd, TRUE);
    SetWindowSubclass(m_dropgrouphwnd, &SubclassWindowProc, 0, 0);
    SetWindowSubclass(m_dd_listhwnd, &SubclassWindowProc, 0, 0);
    SetWindowLongPtr(m_dd_listhwnd, GWLP_USERDATA, (LONG)this);

    m_addgrouphwnd = create_group(m_prnthwnd, 2, 162, 320, 320, (TCHAR*)L"Add script", IDC_ADDGROUP);
    m_name_edithwnd = create_edittext(m_addgrouphwnd, 4, 18, 160, 18, IDC_NAMETEXT, (TCHAR*)L"");
    m_cmd_edithwnd = create_edittext(m_addgrouphwnd, 4, 46, 160, 18, IDC_EXE_EDIT, (TCHAR*)L"python.exe");

    m_src_pathhwnd = create_edittext(m_addgrouphwnd, 4, 76, 284, 18, IDC_SRCPATH, (TCHAR*)L"");
    m_src_filebtn = create_button(m_addgrouphwnd, 292, 75, 26, 19, IDC_SRC_BUTTON, (TCHAR*)L"...");

    m_venv_chkboxhwnd = create_checkbox(m_addgrouphwnd, 4, 102, 76, 18, IDC_VENVCHK, (TCHAR*)L"Use venv");
    m_venv_pathhwnd = create_edittext(m_addgrouphwnd, 4, 120, 284, 18, IDC_VENVPATH, (TCHAR*)L"");
    m_venv_dirbtn = create_button(m_addgrouphwnd, 292, 119, 26, 19, IDC_VENV_BUTTON, (TCHAR*)L"...");

    m_dir_pathhwnd = create_edittext(m_addgrouphwnd, 4, 161, 284, 18, IDC_WDIRPATH, (TCHAR*)L"");
    m_working_dirbtn = create_button(m_addgrouphwnd, 292, 160, 26, 19, IDC_DIR_BUTTON, (TCHAR*)L"...");
    SetWindowSubclass(m_addgrouphwnd, &SubclassWindowProc, 0, 0);
    SetWindowLongPtr(m_addgrouphwnd, GWLP_USERDATA, (LONG)this);

    m_stor_arg_chkboxhwnd = create_checkbox(m_addgrouphwnd, 44, 196, 126, 18, IDC_STORE_ARGS_CHKBOX, (TCHAR*)L"Store Arguments");

    create_cmd_radiobutton(m_addgrouphwnd, 44, 226, 120, 25);
    m_add_btnhwnd = create_button(m_addgrouphwnd, 49, 270, 100, 32, IDC_ADD_BUTTON, (TCHAR*)L"Add");
    m_update_btnhwnd = create_button(m_addgrouphwnd, 170, 270, 100, 32, ID_UPDATE_BUTTON, (TCHAR*)L"Update");

    Edit_SetCueBannerText(m_name_edithwnd, L"Name");
    Edit_SetCueBannerText(m_cmd_edithwnd, L"*.exe");
    Edit_SetCueBannerText(m_venv_pathhwnd, L"venv activate script path");
    Edit_SetCueBannerText(m_src_pathhwnd, L"Script path");
    Edit_SetCueBannerText(m_dir_pathhwnd, L"Working directory");

    reset_script_value();
    change_select_combobox();
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
    SendMessage(m_delete_btnhwnd, WM_SETFONT, (WPARAM)m_shFont, MAKELPARAM(FALSE, 0));

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
    int addarea = 329;

    if (addmenu) {
        ShowWindow(m_addgrouphwnd, SW_SHOWNORMAL);
        SetWindowPos(hWnd, HWND_TOP, 0, 0, cx, cy + addarea, SWP_NOMOVE);
    } else {
        ShowWindow(m_addgrouphwnd, SW_HIDE);
        SetWindowPos(hWnd, HWND_TOP, 0, 0, cx, cy - addarea, SWP_NOMOVE);
    }
}
void SrcMgr::click_add_script(int index)
{
    TCHAR namebuf[MAX_PATH] = { '\0' };
    GetWindowText(m_name_edithwnd, namebuf, MAX_PATH);
    trim_tchar(namebuf);
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
    trim_tchar(exebuf);
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
    if (BST_CHECKED == SendMessage(m_venv_chkboxhwnd, BM_GETCHECK, 0, 0)) {
        GetWindowText(m_venv_pathhwnd, vpathbuf, MAX_PATH);
        trim_tchar(vpathbuf);
    } else {
        vpathbuf[0] = '\0';
    }
    TCHAR* dirpathbuf = new TCHAR[MAX_PATH];
    GetWindowText(m_dir_pathhwnd, dirpathbuf, MAX_PATH);
    trim_tchar(dirpathbuf);

    TCHAR* argsbuf = new TCHAR[8191];
    argsbuf[0] = '\0';
    if (BST_CHECKED == SendMessage(m_stor_arg_chkboxhwnd, BM_GETCHECK, 0, 0)) {
        TCHAR tmpbuf[MAX_PATH] = { '\0' };
        int items = SendMessage(m_dd_listhwnd, LB_GETCOUNT, 0, 0);
        for (size_t i = 0; i < items; i++) {
            SendMessage(m_dd_listhwnd, LB_GETTEXT, i, (LPARAM)tmpbuf);
            wcscat_s(argsbuf, 8191, tmpbuf);
            tmpbuf[0] = '\0';
            if (i != items - 1) {
                wcscat_s(argsbuf, 8191, L"\n");
            }
        }
    }

    int wopt = SW_SHOWNORMAL;
    if (SendMessage(m_showcmdhwnd, BM_GETCHECK, 0, 0) != BST_CHECKED) {
        wopt = SW_HIDE;
    }

    TCHAR* cmd = new TCHAR[MAX_PATH];
    cmd[0] = '\0';
    wcscat_s(cmd, MAX_PATH, L"cmd.exe");

    add_script(
        scriptname,
        exename,
        vpathbuf,
        srcpathbuf,
        dirpathbuf,
        argsbuf,
        cmd,
        wopt,
        index
    );
}
void SrcMgr::trim_tchar(TCHAR* pText)
{
    TCHAR wsBuf[MAX_PATH];
    wcscpy_s(wsBuf, MAX_PATH, pText);
    TCHAR* p = wsBuf;
    TCHAR *pTop = NULL, *pEnd = NULL;
    while ((*p != L'\0') && (*p != L'\r') && (*p != L'\n')) {
        if ((*p != L'\t') && (*p != L' ') && (*p != L'@')) {
            // æ“ª‚ð•Û‘¶
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
        long long nLength = (pEnd - pTop);
        wmemcpy(pText, pTop, nLength << 1);
        pText[nLength] = L'\0';
    } else {
        pText[0] = L'\0';
    }
}
void SrcMgr::add_arg_txt(HWND hDlg)
{
    HWND ehwnd = GetDlgItem(hDlg, IDC_ARG_EDIT);
    TCHAR orgtxt[MAX_PATH];
    GetWindowText(ehwnd, orgtxt, MAX_PATH);
    trim_tchar(orgtxt);
    if (_tcslen(orgtxt) > 0) {
        SendMessage(m_dd_listhwnd, LB_ADDSTRING, 0, (LPARAM)orgtxt);
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
    TCHAR _filepath[MAX_PATH];
    TCHAR filepath[MAX_PATH];
    _filepath[0] = '\0';
    filepath[0] = '\0';

    int num = DragQueryFile(hdrop, -1, NULL, 0);
    for (int i = 0; i < num; i++) {
        DragQueryFile(hdrop, i, _filepath, sizeof(_filepath) / sizeof(TCHAR));
        wcscat_s(filepath, MAX_PATH, L"\"");
        wcscat_s(filepath, MAX_PATH, _filepath);
        wcscat_s(filepath, MAX_PATH, L"\"");
        SendMessage(m_dd_listhwnd, LB_ADDSTRING, 0, (LPARAM)filepath);
        _filepath[0] = '\0';
        filepath[0] = '\0';
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
            g_srcmgr->change_venv_checkbox();
        } break;

        case IDC_NAMETEXT:
        case IDC_VENVPATH:
        case IDC_SRCPATH:
        case IDC_WDIRPATH: {
            switch (HIWORD(wParam)) {
            case EN_CHANGE:
                g_srcmgr->change_script_path();
                break;
            }
        } break;

        case IDC_ADD_BUTTON: {
            g_srcmgr->click_add_script();
        } break;

        case ID_UPDATE_BUTTON: {
            int exeidx = SendMessage(g_srcmgr->m_combohwnd, CB_GETCURSEL, 0, 0);
            SendMessage(g_srcmgr->m_combohwnd, CB_DELETESTRING, exeidx, 0);
            g_srcmgr->click_add_script(exeidx);
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

        case ID_COMB_DELETE: {
            g_srcmgr->delete_script();
        } break;
        }
    } break;

    case WM_NCDESTROY:
        RemoveWindowSubclass(hWnd, SubclassWindowProc, uIdSubclass);
        break;
    }

    return DefSubclassProc(hWnd, uMsg, wParam, lParam);
}
