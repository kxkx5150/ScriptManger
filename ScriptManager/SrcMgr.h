#pragma once
#include <Windows.h>
#include <deque>
#include <string>

struct Command {
    TCHAR* name;
    TCHAR* exe;
    TCHAR* batpath;
    TCHAR* pypath;
    TCHAR* pydir;
    TCHAR* args;
    TCHAR* cmd;
    int windowopt;
};

LRESULT CALLBACK SubclassWindowProc(HWND hWnd, UINT uMsg, WPARAM wParam,
    LPARAM lParam, UINT_PTR uIdSubclass, DWORD_PTR dwRefData);
INT_PTR CALLBACK add_arg_proc(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam);

class SrcMgr {
public:
    HWND m_prnthwnd = nullptr;
    HINSTANCE m_hInst;

    HWND m_cmd_edithwnd = nullptr;
    HWND m_venv_chkboxhwnd = nullptr;
    HWND m_venv_pathhwnd = nullptr;
    HWND m_src_pathhwnd = nullptr;
    HWND m_dir_pathhwnd = nullptr;
    HWND m_venv_dirbtn = nullptr;
    HWND m_stor_arg_chkboxhwnd = nullptr;
    HWND m_combohwnd = nullptr;

private:
    HFONT m_hFont = nullptr;
    HFONT m_shFont = nullptr;
    HFONT m_sshFont = nullptr;
    HFONT m_lsthFont = nullptr;

    HWND m_run_btnhwnd = nullptr;
    HWND m_delete_btnhwnd = nullptr;

    HWND m_dd_listhwnd = nullptr;

    HWND m_name_edithwnd = nullptr;
    HWND m_showcmdhwnd = nullptr;
    HWND m_hidecmdhwnd = nullptr;
    HWND m_addgrouphwnd = nullptr;
    HWND m_dropgrouphwnd = nullptr;
    HWND m_add_btnhwnd = nullptr;
    HWND m_update_btnhwnd = nullptr;

    HWND m_addarghwnd = nullptr;
    HWND m_delarghwnd = nullptr;
    HWND m_uparghwnd = nullptr;
    HWND m_downarghwnd = nullptr;

    HWND m_src_filebtn = nullptr;
    HWND m_working_dirbtn = nullptr;

    std::deque<Command> m_commands;
    int m_activeidx = -1;

public:
    SrcMgr(HWND hWnd, HINSTANCE hInst);
    ~SrcMgr();

    void init();
    void resize_window(HWND hWnd, bool addmenu);

    void change_venv_checkbox();
    void change_select_combobox();
    void exe_script(int exeidx = -1);
    void delete_script(int exeidx = -1);
    void delete_command(int exeidx);

    void change_script_path();
    void set_script_value(int exeidx);
    void reset_script_value();

    void open_file_dialog(HWND hwnd, HWND pathhwnd, const TCHAR* filtertxt);
    void open_directory_dialog(HWND hwnd, HWND dirhwnd);
    void click_add_script(int index=-1);
    void drop_files_into_listbox(HDROP hdrop);

    void click_add_arg();
    void click_del_arg();
    void click_up_arg();
    void click_down_arg();

    void add_arg_txt(HWND hDlg);
    void trim_tchar(TCHAR* pText);
    void replace_string(TCHAR* strbuf, int maxlen, std::wstring sword, std::wstring rword);

private:
    void set_font();
    void create_control();
    HFONT create_font(int fontsize);
    HWND create_group(HWND hParent, int nX, int nY, int nWidth, int nHeight, TCHAR* txt, int id = NULL);
    HWND create_combobox(HWND hParent, int nX, int nY, int nWidth, int nHeight, int id);
    HWND create_button(HWND hParent, int nX, int nY, int nWidth, int nHeight, int id, TCHAR* txt);
    HWND create_dorp_listbox(HWND hParent, int nX, int nY, int nWidth, int nHeight);
    HWND create_edittext(HWND hParent, int nX, int nY, int nWidth, int nHeight, int id, TCHAR* txt);
    HWND create_checkbox(HWND hParent, int nX, int nY, int nWidth, int nHeight, int id, TCHAR* txt);
    void create_cmd_radiobutton(HWND hParent, int nX, int nY, int nWidth, int nHeight);

    void add_script(TCHAR* name, TCHAR* exe, TCHAR* batpath, TCHAR* pypath, 
        TCHAR* pydir, TCHAR* args, TCHAR* cmd, int windowopt, int index);

    void select_combobox_item(int index);
    int get_combobox_index(TCHAR* itemstr);
};
