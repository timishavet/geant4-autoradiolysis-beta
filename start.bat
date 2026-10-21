@echo off
chcp 65001 >nul
setlocal EnableDelayedExpansion
cd /d "%~dp0"

rem ============================================================
rem   start.bat - запуск «Программы Б»
rem   с проверкой установки и данных Geant4
rem   Всё в порядке: окно программы откроется молча.
rem   Проблема: будет написано, что именно не так и что сделать.
rem
rem   Переменные G4*DATA не создаются и не изменяются:
rem   их прописывает install_data.bat.
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
set "DATATARGET=!G4ROOT!\data"

set "REPORT=%TEMP%\g4data_report.txt"
if exist "!REPORT!" del "!REPORT!" >nul 2>&1

rem --- 5. критичные пакеты данных: переменная задана и папка на месте ---
call :checkpkg G4LEDATA G4EMLOW 1
call :checkpkg G4RADIOACTIVEDATA RadioactiveDecay 1
call :checkpkg G4LEVELGAMMADATA PhotonEvaporation 1
call :checkpkg G4ENSDFSTATEDATA G4ENSDFSTATE 1
call :checkpkg G4PARTICLEXSDATA G4PARTICLEXS 1

rem --- 6. остальные пакеты: тоже проверяем, но не ругаемся ---
call :checkpkg G4NEUTRONHPDATA G4NDL 0
call :checkpkg G4PIIDATA G4PII 0
call :checkpkg G4REALSURFACEDATA RealSurface 0
call :checkpkg G4SAIDXSDATA G4SAIDDATA 0
call :checkpkg G4ABLADATA G4ABLA 0
call :checkpkg G4INCLDATA G4INCL 0
call :checkpkg G4CHANNELINGDATA G4CHANNELING 0
call :checkpkg G4TENDLDATA G4TENDL 0
call :checkpkg G4NUDEXLIBDATA G4NUDEXLIB 0
call :checkpkg G4URRPTDATA G4URRPT 0

if exist "!REPORT!" goto :missing_data

rem --- 7. сборка программы ---
if not exist "build\Release\main.exe" goto :no_main_exe

start "" %LAUNCHER% program_b.py
exit /b 0

rem ============================================================
rem  пакет: имя переменной, начало имени папки, обязательный (1/0)
rem ============================================================
:checkpkg
set "VN=%~1"
set "PAT=%~2"
set "CRIT=%~3"
set "VAL="
for /f "tokens=1* delims==" %%A in ('set !VN! 2^>nul') do set "VAL=%%B"
if defined VAL if exist "!VAL!\" exit /b 0
if "!CRIT!"=="1" >>"!REPORT!" echo   !VN!  -  пакет данных %PAT% не установлен
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
echo  и в руководстве, раздел 5.2), положите их рядом с start.bat
echo  и запустите install_data.bat от имени администратора:
echo  он и распакует пакеты, и пропишет переменные G4*DATA.
echo.
del "!REPORT!" >nul 2>&1
pause
exit /b 1

:no_python
echo.
echo ============================================================
echo ОШИБКА: не найден Python.
echo ============================================================
echo.
echo Установите Python 3.12 или новее с сайта https://www.python.org/downloads/windows/
echo В установщике обязательно включите "Add python.exe to PATH".
echo.
pause
exit /b 1

:no_tkinter
echo.
echo ============================================================
echo ОШИБКА: Python установлен без компонента tkinter.
echo ============================================================
echo.
echo Переустановите Python и включите компонент "tcl/tk and IDLE".
echo.
pause
exit /b 1

:no_matplotlib
echo.
echo ============================================================
echo ОШИБКА: не установлен пакет matplotlib.
echo ============================================================
echo.
echo Откройте командную строку (Win+R, введите cmd, нажмите Enter) и выполните:
echo     py -m pip install matplotlib
echo.
pause
exit /b 1

:no_geant4
echo.
echo ============================================================
echo ОШИБКА: не найден Geant4.
echo ============================================================
echo.
echo Установите Geant4 11.4.1 - раздел 5.1 руководства, не забудьте
echo отметить добавление в PATH и перезагрузите компьютер.
echo.
pause
exit /b 1

:no_main_exe
echo.
echo ============================================================
echo ОШИБКА: не найден файл build\Release\main.exe.
echo ============================================================
echo.
echo Архив распакован не полностью. Распакуйте его заново.
echo.
pause
exit /b 1
