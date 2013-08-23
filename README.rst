luawinalttab
===========================
Overview
--------------

- luawinalttab gives windows *alt+tab* abilities to Lua.

Supported environments
----------------------------------

- Lua 5.1.4
- gcc with MinGW-W64

API
--------------

``winalttab.list()``

Returns a list of running applications.

returns
    table such as ``{ {hwnd=1000, title="explorer", path="c:\Windows\explorer.exe"} }``

``winalttab.activate(hwnd)``

Activates a window.

args
    hwnd: HWND value 
