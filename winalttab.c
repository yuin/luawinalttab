#define _UNICODE 1
#define UNICODE 1
#include <memory.h>
#include <windows.h>
#include <windowsx.h>
#include <psapi.h>
#include <lua.h>
#include <lauxlib.h>

/* constants {{{ */
#define LWAT_MODNAME   "winalttab"
#define LWAT_VERSION   "1.0.0"
/* }}} */

/* macros {{{ */
#if defined _WIN64 || defined WIN64
#  define SetWindowLongPtrW64( w, i, l ) \
                SetWindowLongPtrW( w, i, (LONG_PTR)(l) )
#  define GetWindowLongPtrW64( w, i) \
                GetWindowLongPtrW( w, i)
#else
#  define SetWindowLongPtrW64( w, i, l ) \
                SetWindowLongPtrW( w, i, (LONG)(l) )
#  define GetWindowLongPtrW64( w, i) \
                GetWindowLongPtrW( w, i)
#endif
/* }}} */

/* compatibility stuff {{{ */
#if !defined(LUA_VERSION_NUM) || LUA_VERSION_NUM < 502
static void luaL_setfuncs (lua_State *l, const luaL_Reg *reg, int nup)
{
    int i;
    luaL_checkstack(l, nup, "too many upvalues");
    for (; reg->name != NULL; reg++) {  /* fill the table with given functions */
        for (i = 0; i < nup; i++)  /* copy upvalues to the top */
            lua_pushvalue(l, -nup);
        lua_pushcclosure(l, reg->func, nup);  /* closure with those upvalues */
        lua_setfield(l, -(nup + 2), reg->name);
    }
    lua_pop(l, nup);  /* remove upvalues */
}
#endif
/* }}} */

static BOOL get_exe_by_hwnd(HWND hwnd, TCHAR *dest, size_t dest_size) { /* {{{ */
  DWORD processID = NULL;
  memset(dest, 0, dest_size);
  GetWindowThreadProcessId( hwnd, &processID);
  HANDLE hProcess = OpenProcess(
    PROCESS_QUERY_INFORMATION | PROCESS_VM_READ, FALSE, processID);
  if(!hProcess) return FALSE;
  HMODULE hModule = NULL;
  DWORD dummy = 0;
  if(!EnumProcessModules( hProcess, &hModule, sizeof(HMODULE), &dummy))
    return FALSE;
  if(!GetModuleFileNameEx( hProcess, hModule, dest, dest_size))
    return FALSE;
  CloseHandle( hProcess);

  return TRUE;
} /* }}} */

static char* oschar2utf8(const TCHAR *src) { // {{{
  size_t size = WideCharToMultiByte(CP_UTF8, 0, src, -1, NULL, 0, NULL, NULL);
  size++;
  char *buff = malloc(sizeof(char) * size);
  size = WideCharToMultiByte(CP_UTF8, 0, src, -1, buff, size, NULL, NULL);
  buff[size] = 0;
  return buff;
} // }}}

static int is_alttab_window(HWND hwnd){ /* {{{ */
  TCHAR buf[10];
  int no_owner;
  LONG exstyle;

  if(!IsWindowVisible(hwnd)) return 0;
  if(GetParent(hwnd) != 0) return 0;
  no_owner = GetWindow(hwnd, GW_OWNER) == 0;
  exstyle = GetWindowLongPtrW64(hwnd, GWL_EXSTYLE);
  if( ((exstyle & WS_EX_TOOLWINDOW) == 0 && no_owner) ||
      ((exstyle & WS_EX_APPWINDOW) != 0 && !no_owner)){
    if(GetWindowTextW(hwnd, buf, 10)) {
      return 1;
    }
  }
  return 0;
} /* }}} */

struct window_proc_data { /* {{{ */
  lua_State *L;
  int       index;
}; /* }}} */

BOOL CALLBACK enum_window_proc(HWND hwnd , LPARAM lp) { /* {{{ */
  struct window_proc_data *data = (struct window_proc_data *)lp;
  lua_State *L = data->L;
  TCHAR os_title[1024];
  TCHAR os_exe[1024];
  
  if(is_alttab_window(hwnd)) {
    lua_pushinteger(L, data->index);

    lua_newtable(L);

    GetWindowTextW(hwnd , os_title , 1024);
    char *title = oschar2utf8(os_title);
    lua_pushstring(L, title);
    lua_setfield(L, -2, "title");
    free(title);

    lua_pushinteger(L, (lua_Integer)hwnd); 
    lua_setfield(L, -2, "hwnd");

    get_exe_by_hwnd(hwnd, os_exe, 1024);
    char *exe = oschar2utf8(os_exe);
    lua_pushstring(L, exe);
    lua_setfield(L, -2, "path");
    free(exe);

    lua_settable(L, -3);
    data->index++;
  }
  return TRUE;
} /* }}} */

static int lua_winalttab_list(lua_State *L){ /* {{{ */
  struct window_proc_data data = {L, 1};
  lua_newtable(L);
  EnumWindows(enum_window_proc, (LPARAM)&data);
  return 1;
} /* }}} */

static int lua_winalttab_activate(lua_State *L){ /* {{{ */
  HWND hwnd = (HWND)luaL_checkinteger(L, 1);
  ShowWindow(hwnd, SW_SHOWNORMAL);
  SetForegroundWindow(hwnd);
  SetActiveWindow(hwnd);
  return 0;
} /* }}} */

static int lua_winalttab_new(lua_State *L){ /* {{{ */
  luaL_Reg reg[] = {
      {"list", lua_winalttab_list},
      {"activate", lua_winalttab_activate},
      {NULL, NULL}
  };
  lua_newtable(L);
  luaL_setfuncs(L, reg, 0);

  lua_pushliteral(L, LWAT_VERSION);
  lua_setfield(L, -2, "_NAME");
  lua_pushliteral(L, LWAT_VERSION);
  lua_setfield(L, -2, "_VERSION");
  return 1;
} /* }}} */

__declspec(dllexport) int luaopen_winalttab(lua_State *L) { /* {{{ */
    lua_winalttab_new(L);
    return 1;
} /* }}} */

