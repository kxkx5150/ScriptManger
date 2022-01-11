#pragma once
#include <Windows.h>
#include <string>
#include <tchar.h>
#include <vector>

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
LRESULT CALLBACK search_proc(HWND hWnd, UINT uMsg, WPARAM wParam,
    LPARAM lParam, UINT_PTR uIdSubclass, DWORD_PTR dwRefData);
LRESULT CALLBACK search_listproc(HWND hWnd, UINT uMsg, WPARAM wParam,
    LPARAM lParam, UINT_PTR uIdSubclass, DWORD_PTR dwRefData);
INT_PTR CALLBACK add_arg_proc(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam);
LRESULT CALLBACK LowLevelKeyboardProc(int nCode, WPARAM wParam, LPARAM lParam);
LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);

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
    HWND m_dd_listhwnd = nullptr;
    HWND m_search_listhwnd = nullptr;
    std::vector<Command> m_commands;

private:
    HFONT m_hFont = nullptr;
    HFONT m_shFont = nullptr;
    HFONT m_sshFont = nullptr;
    HFONT m_lsthFont = nullptr;

    HWND m_combogrouphwnd = nullptr;

    HWND m_run_btnhwnd = nullptr;
    HWND m_delete_btnhwnd = nullptr;

    HWND m_dd_clearhwnd = nullptr;
    HWND m_addarghwnd = nullptr;
    HWND m_delarghwnd = nullptr;
    HWND m_uparghwnd = nullptr;
    HWND m_downarghwnd = nullptr;

    HWND m_name_edithwnd = nullptr;

    HWND m_cmd_txt = nullptr;
    HWND m_autoclose = nullptr;
    HWND m_showcmdhwnd = nullptr;
    HWND m_hidecmdhwnd = nullptr;

    HWND m_addgrouphwnd = nullptr;
    HWND m_dropgrouphwnd = nullptr;

    HWND m_add_btnhwnd = nullptr;
    HWND m_update_btnhwnd = nullptr;
    HWND m_clear_btnhwnd = nullptr;

    HWND m_src_filebtn = nullptr;
    HWND m_working_dirbtn = nullptr;

    HWND m_search_grouphwnd = nullptr;
    HWND m_search_edithwnd = nullptr;

    int m_activeidx = -1;
    std::vector<Command> m_search_commands;
    std::wstring m_commandline_args = L"";

public:
    SrcMgr(HWND hWnd, HINSTANCE hInst);
    ~SrcMgr();

    void init();
    void receive_args(int idx, const TCHAR* cmdline);
    void resize_window(HWND hWnd, bool addmenu, int addarea);

    void change_venv_checkbox();
    void change_select_combobox();

    int get_ddlist_value(TCHAR* listtxt);
    void exe_script(Command command);
    void delete_script(int exeidx = -1);
    void delete_command(int exeidx);

    void change_script_path();
    void set_script_value(int exeidx);
    void set_ddlist_value(const TCHAR* args, const TCHAR* delim);
    void reset_script_value();

    void open_file_dialog(HWND hwnd, HWND pathhwnd, const TCHAR* filtertxt);
    void open_directory_dialog(HWND hwnd, HWND dirhwnd);
    void click_add_script(int index = -1);
    void drop_files_into_listbox(HDROP hdrop);

    void click_add_arg();
    void click_del_arg();
    void click_up_arg();
    void click_down_arg();

    int create_search_list_item(const TCHAR* str = L"");
    void clear_search_editor();

    void add_arg_txt(HWND hDlg);
    void trim_tchar(TCHAR* pText);
    void replace_string(TCHAR* strbuf, int maxlen, std::wstring sword, std::wstring rword);
    std::vector<std::wstring> split(std::wstring& input, TCHAR delimiter);

    void write_setting_csv();
    void read_setting_csv();
    int write_file(TCHAR* filename, TCHAR* args, bool utf8 = false);
    TCHAR* read_file(const TCHAR* filename);
    void exe_directory_path(TCHAR* path);
    void input_search(HWND hWnd, TCHAR ch);
    void set_focus_search_editor();
    void exec_search_command(int idx);

private:
    void set_font();
    void create_control();
    HFONT create_font(int fontsize);
    HWND create_group(HWND hParent, int nX, int nY, int nWidth, int nHeight, TCHAR* txt, int id = NULL);
    HWND create_combobox(HWND hParent, int nX, int nY, int nWidth, int nHeight, int id);
    HWND create_button(HWND hParent, int nX, int nY, int nWidth, int nHeight, int id, TCHAR* txt);
    HWND create_listbox(HWND hParent, int nX, int nY, int nWidth, int nHeight, int id);
    HWND create_edittext(HWND hParent, int nX, int nY, int nWidth, int nHeight, int id, TCHAR* txt);
    HWND create_checkbox(HWND hParent, int nX, int nY, int nWidth, int nHeight, int id, TCHAR* txt);
    void create_cmd_radiobutton(HWND hParent, int nX, int nY, int nWidth, int nHeight);

    void add_script(Command command, int index);

    void select_combobox_item(int index);
    int get_combobox_index(TCHAR* itemstr);

    void BuildList();

    bool check_toml(std::wstring dirstr, const TCHAR* vpath);
    void check_venv(std::wstring dirstr, const TCHAR* vpath);
};
