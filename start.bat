@echo off
chcp 65001 >nul
setlocal EnableDelayedExpansion
cd /d "%~dp0"

rem ============================================================
rem   start.bat - запуск «Программы Б»
rem   с проверкой установки и данных Geant4
rem   Всё в порядке: окно программы откроется молча.
rem   Проблема: будет написано, что именно не так и что сделать.
rem ============================================================

rem --- 1. Python ---
set "LAUNCHER="
where pyw.exe >nul 2>&1
if not errorlevel 1 (
    set "LAUNCHER=pyw"
) else (
    where pythonw.exe >nul 2>&1
    if not errorlevel 1 set "LAUNCHER=pythonw"
)
if not defined LAUNCHER goto :no_python

rem --- 2. tkinter ---
%LAUNCHER% -c "import tkinter" >nul 2>&1
if errorlevel 1 goto :no_tkinter

rem --- 3. matplotlib ---
%LAUNCHER% -c "import matplotlib" >nul 2>&1
if errorlevel 1 goto :no_matplotlib

rem --- 4. Geant4 в PATH ---
set "G4BAT="
for /f "delims=" %%i in ('where geant4.bat 2^>nul') do if not defined G4BAT set "G4BAT=%%i"
if not defined G4BAT goto :no_geant4

rem --- корень установки Geant4 и папка данных ---
set "G4ROOT=!G4BAT:\bin\geant4.bat=!"
rem куда ставит install_data.bat (создаётся при необходимости)
set "DATATARGET=!G4ROOT!\data"

set "REPORT=%TEMP%\g4data_report.txt"
if exist "!REPORT!" del "!REPORT!" >nul 2>&1

rem --- 5. критичные пакеты данных ---
call :checkcrit G4LEDATA G4EMLOW
call :checkcrit G4RADIOACTIVEDATA RadioactiveDecay
call :checkcrit G4LEVELGAMMADATA PhotonEvaporation
call :checkcrit G4ENSDFSTATEDATA G4ENSDFSTATE
call :checkcrit G4PARTICLEXSDATA G4PARTICLEXS

rem --- 6. остальные пакеты: если скачаны - прописываем переменные ---
call :checkopt G4ABLADATA G4ABLA
call :checkopt G4CHANNELINGDATA G4CHANNELING
call :checkopt G4INCLDATA G4INCL
call :checkopt G4NEUTRONHPDATA G4NDL
call :checkopt G4NUDEXLIBDATA G4NUDEXLIB
call :checkopt G4PIIDATA G4PII
call :checkopt G4REALSURFACEDATA RealSurface
call :checkopt G4SAIDXSDATA G4SAIDDATA
call :checkopt G4TENDLDATA G4TENDL
call :checkopt G4URRPTDATA G4URRPT

if exist "!REPORT!" goto :missing_data

rem --- 7. сборка программы ---
if not exist "build\Release\main.exe" goto :no_main_exe

start "" %LAUNCHER% program_b.py
exit /b 0

rem ============================================================
rem  критичный пакет: имя переменной, начало имени папки
rem ============================================================
:checkcrit
set "VN=%~1"
set "PAT=%~2"
set "VAL="
for /f "tokens=1* delims==" %%A in ('set !VN! 2^>nul') do set "VAL=%%B"
if defined VAL if exist "!VAL!\" exit /b 0
set "DIRFOUND="
for /d %%D in ("!DATATARGET!\%PAT%*") do if not defined DIRFOUND set "DIRFOUND=%%~fD"
if defined DIRFOUND (
    setx !VN! "!DIRFOUND!" >nul 2>&1
    set "!VN!=!DIRFOUND!"
    echo [Исправлено] создана переменная !VN! = !DIRFOUND!
    exit /b 0
)
>>"!REPORT!" echo   !VN!  -  папка данных %PAT% не найдена
exit /b 0

rem ============================================================
rem  необязательный пакет: имя переменной, начало имени папки
rem ============================================================
:checkopt
set "VN=%~1"
set "PAT=%~2"
set "VAL="
for /f "tokens=1* delims==" %%A in ('set !VN! 2^>nul') do set "VAL=%%B"
if defined VAL if exist "!VAL!\" exit /b 0
set "DIRFOUND="
for /d %%D in ("!DATATARGET!\%PAT%*") do if not defined DIRFOUND set "DIRFOUND=%%~fD"
if defined DIRFOUND (
    setx !VN! "!DIRFOUND!" >nul 2>&1
    set "!VN!=!DIRFOUND!"
    echo [Исправлено] создана переменная !VN! = !DIRFOUND!
)
exit /b 0

:missing_data
echo.
echo ============================================================
echo  ОШИБКА: в установке Geant4 не хватает пакетов данных
echo ============================================================
echo.
echo  Не найдено:
echo.
type "!REPORT!"
echo.
echo  Скачайте архивы .tar.gz (ссылки - в install_data.bat
echo  и в руководстве, раздел 3.2), положите их рядом с start.bat
echo  и запустите install_data.bat один раз.
echo.
del "!REPORT!" >nul 2>&1
pause
exit /b 1

:no_python
echo.
echo ОШИБКА: не найден Python.
echo.
echo Установите Python 3.12 или новее с сайта https://www.python.org/downloads/windows/
echo В установщике обязательно включите "Add python.exe to PATH".
echo.
pause
exit /b 1

:no_tkinter
echo.
echo ОШИБКА: Python установлен без компонента tkinter.
echo.
echo Переустановите Python и включите компонент "tcl/tk and IDLE".
echo.
pause
exit /b 1

:no_matplotlib
echo.
echo ОШИБКА: не установлен пакет matplotlib.
echo.
echo Откройте командную строку (Win+R, введите cmd, нажмите Enter) и выполните:
echo     py -m pip install matplotlib
echo.
pause
exit /b 1

:no_geant4
echo.
echo ОШИБКА: не найден Geant4.
echo.
echo Установите Geant4 11.4.1 - при установке отметьте добавление в PATH.
echo После установки закройте и снова откройте этот файл.
echo.
pause
exit /b 1

:no_main_exe
echo.
echo ОШИБКА: не найден файл build\Release\main.exe.
echo.
echo Архив распакован не полностью. Распакуйте его заново.
echo.
pause
exit /b 1
