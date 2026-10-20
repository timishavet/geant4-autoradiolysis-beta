@echo off
chcp 65001 >nul
setlocal EnableDelayedExpansion
cd /d "%~dp0"

rem ============================================================
rem  install_data.bat - установка пакетов данных Geant4
rem
rem  Порядок: скачайте 5 архивов .tar.gz (ссылки в этом окне),
rem  положите их рядом с этим файлом и запустите его ОДИН раз.
rem  Батник распакует данные в штатную папку данных Geant4
rem  (share\Geant4\data) и пропишет переменные среды G4*DATA.
rem ============================================================

rem --- 1. установлен ли Geant4 ---
set "G4BAT="
for /f "delims=" %%i in ('where geant4.bat 2^>nul') do if not defined G4BAT set "G4BAT=%%i"
if not defined G4BAT goto :no_geant4
set "G4ROOT=!G4BAT:\bin\geant4.bat=!"

rem --- 2. версия Geant4 из cmake-конфигурации ---
set "G4VER="
set "VLINE="
if exist "!G4ROOT!\lib\cmake\Geant4\Geant4ConfigVersion.cmake" for /f "usebackq delims=" %%L in ("!G4ROOT!\lib\cmake\Geant4\Geant4ConfigVersion.cmake") do (
    set "LINE=%%L"
    if "!LINE:~0,20!"=="set(PACKAGE_VERSION " set "VLINE=!LINE!"
)
if defined VLINE (
    set "G4VER=!VLINE:*"=!"
    set "G4VER=!G4VER:"=!"
    set "G4VER=!G4VER:)=!"
)
if not defined G4VER set /p G4VER=Не удалось определить версию Geant4. Введите её (например 11.4.1): 
if not defined G4VER goto :no_version

rem куда распаковываем (создаётся при необходимости)
set "DATATARGET=!G4ROOT!\data"
rem а здесь пакеты могут уже лежать, если их ставил установщик Geant4
set "G4DATASTD=!G4ROOT!\share\Geant4\data"

rem --- 3. есть ли встроенный tar (Windows 10 1803+) ---
set "TAROK="
where tar >nul 2>&1 && set "TAROK=1"

set "REPORT=%TEMP%\g4_install_report.txt"
if exist "!REPORT!" del "!REPORT!" >nul 2>&1
set /a DONE=0

echo.
echo ============================================================
echo  Установка пакетов данных Geant4 !G4VER!
echo  Папка назначения: !DATATARGET!
echo ============================================================
echo.

call :pkg G4LEDATA G4EMLOW G4EMLOW.8.8 https://cern.ch/geant4-data/datasets/G4EMLOW.8.8.tar.gz
call :pkg G4RADIOACTIVEDATA RadioactiveDecay G4RadioactiveDecay.6.1.2 https://cern.ch/geant4-data/datasets/G4RadioactiveDecay.6.1.2.tar.gz
call :pkg G4LEVELGAMMADATA PhotonEvaporation G4PhotonEvaporation.6.1.2 https://cern.ch/geant4-data/datasets/G4PhotonEvaporation.6.1.2.tar.gz
call :pkg G4ENSDFSTATEDATA G4ENSDFSTATE G4ENSDFSTATE.3.0 https://cern.ch/geant4-data/datasets/G4ENSDFSTATE.3.0.tar.gz
call :pkg G4PARTICLEXSDATA G4PARTICLEXS G4PARTICLEXS.4.2 https://cern.ch/geant4-data/datasets/G4PARTICLEXS.4.2.tar.gz

rem --- остальные пакеты: прописываем переменные, если папки уже есть ---
call :optvar G4ABLADATA G4ABLA
call :optvar G4CHANNELINGDATA G4CHANNELING
call :optvar G4INCLDATA G4INCL
call :optvar G4NEUTRONHPDATA G4NDL
call :optvar G4NUDEXLIBDATA G4NUDEXLIB
call :optvar G4PIIDATA G4PII
call :optvar G4REALSURFACEDATA RealSurface
call :optvar G4SAIDXSDATA G4SAIDDATA
call :optvar G4TENDLDATA G4TENDL
call :optvar G4URRPTDATA G4URRPT

if exist "!REPORT!" goto :missing

echo.
echo ============================================================
echo  Готово: установлено !DONE! из 5 пакетов.
echo  Теперь запустите start.bat.
echo ============================================================
echo.
pause
exit /b 0

rem ============================================================
rem  критичный пакет: VAR, маска папки, имя архива, ссылка
rem ============================================================
:pkg
set "VN=%~1"
set "PAT=%~2"
set "ASTEM=%~3"
set "URL=%~4"

rem папка уже распакована? (сначала куда распаковываем, потом штатная папка Geant4)
set "FND="
for /d %%D in ("!DATATARGET!\%PAT%*") do if not defined FND set "FND=%%~fD"
if not defined FND for /d %%D in ("!G4DATASTD!\%PAT%*") do if not defined FND set "FND=%%~fD"
if defined FND (
    setx !VN! "!FND!" >nul 2>&1
    set "!VN!=!FND!"
    echo   [ок]         %PAT% - уже стоит: !FND!
    set /a DONE+=1
    exit /b 0
)

rem архив лежит рядом с этим батником?
set "ARCH="
for %%F in ("%~dp0%ASTEM%*.tar.gz") do if exist "%%~fF" if not defined ARCH set "ARCH=%%~fF"
if not defined ARCH (
    >>"!REPORT!" echo   %ASTEM%.tar.gz    скачать: !URL!
    echo   [нет]         %ASTEM%.tar.gz - архив не найден рядом с install_data.bat
    exit /b 0
)

if not defined TAROK goto :no_tar

rem распаковка в папку <установка Geant4>\data (создаётся при необходимости)
if not exist "!DATATARGET!" mkdir "!DATATARGET!" >nul 2>&1
set "LOG=%TEMP%\g4_tar.log"
tar -xzf "!ARCH!" -C "!DATATARGET!" >"!LOG!" 2>&1
if errorlevel 1 (
    echo   [ошибка]     не удалось распаковать %ASTEM%.tar.gz, журнал:
    type "!LOG!"
    >>"!REPORT!" echo   %ASTEM%.tar.gz    ошибка распаковки - удалите файл и скачайте заново
    exit /b 1
)
set "FND="
for /d %%D in ("!DATATARGET!\%PAT%*") do if not defined FND set "FND=%%~fD"
if not defined FND (
    echo   [ошибка]     после распаковки не найдена папка %PAT%
    >>"!REPORT!" echo   %ASTEM%.tar.gz    после распаковки нет папки %PAT% - скачайте файл заново
    exit /b 1
)
setx !VN! "!FND!" >nul 2>&1
set "!VN!=!FND!"
echo   [распаковано] %ASTEM% - папка: !FND!
set /a DONE+=1
exit /b 0

rem ============================================================
rem  необязательный пакет: VAR, маска папки
rem ============================================================
:optvar
set "VN=%~1"
set "PAT=%~2"
set "VAL="
for /f "tokens=1* delims==" %%A in ('set !VN! 2^>nul') do set "VAL=%%B"
if defined VAL if exist "!VAL!\" exit /b 0
set "FND="
for /d %%D in ("!DATATARGET!\%PAT%*") do if not defined FND set "FND=%%~fD"
if not defined FND for /d %%D in ("!G4DATASTD!\%PAT%*") do if not defined FND set "FND=%%~fD"
if defined FND (
    setx !VN! "!FND!" >nul 2>&1
    set "!VN!=!FND!"
    echo   [Исправлено] создана переменная !VN! = !FND!
)
exit /b 0

:missing
echo.
echo ============================================================
echo  Не хватает пакетов. Скачайте архивы .tar.gz по ссылкам:
echo ============================================================
echo.
type "!REPORT!"
echo.
echo  Не распаковывайте их вручную - просто положите файлы
echo  рядом с install_data.bat (в эту же папку)
echo  и запустите install_data.bat ещё раз.
echo.
del "!REPORT!" >nul 2>&1
pause
exit /b 1

:no_tar
echo.
echo ОШИБКА: в Windows нет программы tar (нужен Windows 10 1803 или новее).
echo.
echo Распакуйте архивы вручную через 7-Zip в папку:
echo     !DATATARGET!
echo Затем запустите install_data.bat ещё раз - переменные будут прописаны.
echo.
pause
exit /b 1

:no_geant4
echo.
echo ОШИБКА: не найден установленный Geant4.
echo.
echo Установите Geant4 11.4.1 - раздел 3.1 руководства (MANUAL.docx),
echo перезагрузите компьютер и запустите install_data.bat ещё раз.
echo.
pause
exit /b 1

:no_version
echo.
echo ОШИБКА: не удалось определить версию Geant4.
echo.
echo Переустановите Geant4 11.4.1 - раздел 3.1 руководства (MANUAL.docx).
echo.
pause
exit /b 1
