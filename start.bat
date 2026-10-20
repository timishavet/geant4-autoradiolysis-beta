@echo off
chcp 65001 >nul
setlocal
cd /d "%~dp0"

set "LAUNCHER="
where pyw.exe >nul 2>&1
if not errorlevel 1 (
    set "LAUNCHER=pyw"
) else (
    where pythonw.exe >nul 2>&1
    if not errorlevel 1 set "LAUNCHER=pythonw"
)

if not defined LAUNCHER goto :no_python

%LAUNCHER% -c "import tkinter" >nul 2>&1
if errorlevel 1 goto :no_tkinter

%LAUNCHER% -c "import matplotlib" >nul 2>&1
if errorlevel 1 goto :no_matplotlib

where geant4.bat >nul 2>&1
if errorlevel 1 goto :no_geant4

if not defined G4RADIOACTIVEDATA goto :no_decay_data

if not exist "build\Release\main.exe" goto :no_main_exe

start "" %LAUNCHER% program_b.py
exit /b 0

:no_python
echo ОШИБКА: не найден Python.
echo.
echo Установите Python 3.12 или новее с сайта https://www.python.org/downloads/windows/
echo В установщике обязательно включите "Add python.exe to PATH".
echo.
pause
exit /b 1

:no_tkinter
echo ОШИБКА: Python установлен без компонента tkinter.
echo.
echo Переустановите Python и включите компонент "tcl/tk and IDLE".
echo.
pause
exit /b 1

:no_matplotlib
echo ОШИБКА: не установлен пакет matplotlib.
echo.
echo Откройте командную строку (Win+R, введите cmd, нажмите Enter) и выполните:
echo     py -m pip install matplotlib
echo.
pause
exit /b 1

:no_geant4
echo ОШИБКА: не найден Geant4.
echo.
echo Установите Geant4 11.4.1. После установки закройте и снова откройте этот файл.
echo.
pause
exit /b 1

:no_decay_data
echo ОШИБКА: не найдены данные распадов Geant4.
echo Переменная G4RADIOACTIVEDATA не задана.
echo.
echo Переустановите Geant4 11.4.1 - установщик должен создать её автоматически.
echo.
pause
exit /b 1

:no_main_exe
echo ОШИБКА: не найден файл build\Release\main.exe.
echo.
echo Архив распакован не полностью. Распакуйте его заново.
echo.
pause
exit /b 1
