@echo off
setlocal
cd /d "%~dp0"

rem Interactive mode: no arguments -> main.cc loads vis.mac and gui.mac.
.\build\Release\main.exe
if errorlevel 1 (
    echo.
    echo Interactive session failed.
    pause
) else (
    echo.
    echo Interactive session finished.
    pause
)