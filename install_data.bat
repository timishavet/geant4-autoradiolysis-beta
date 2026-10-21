@echo off
chcp 65001 >nul
setlocal EnableDelayedExpansion
cd /d "%~dp0"

rem /elevated - запуск уже с повышенными правами (повторно не повышаем)
if /i "%~1"=="/elevated" set "ELEVATED=1"

rem ============================================================
rem  install_data.bat - установка пакетов данных Geant4
rem
rem  Порядок: скачайте 5 архивов .tar.gz (ссылки в этом окне),
rem  положите их рядом с этим файлом и запустите его ОДИН раз.
rem  Права администратора скрипт запросит сам.
rem
rem  Батник распакует данные в папку data рядом с каталогом
rem  установки Geant4 и пропишет переменные среды G4*DATA.
rem  Обязательных пакетов пять, остальные - по желанию.
rem ============================================================

rem --- 0. права администратора ---
if defined ELEVATED goto :as_admin
>"%SYSTEMROOT%\system32\cacls.exe" "%SYSTEMROOT%\system32\config\system" >nul 2>&1
if not errorlevel 1 goto :as_admin
echo Requesting admin rights...
powershell -NoProfile -Command "Start-Process 'cmd.exe' -ArgumentList '/c \"\"%~f0\" /elevated\"' -Verb RunAs"
exit /b 0

:as_admin
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
if not defined G4VER set "G4VER=11.4.1"

rem куда распаковываем (создаётся при необходимости)
set "DATATARGET=!G4ROOT!\data"

rem --- 3. есть ли встроенный tar (Windows 10 1803+) ---
set "TAROK="
where tar >nul 2>&1 && set "TAROK=1"

set "REPORT=%TEMP%\g4_install_report.txt"
if exist "!REPORT!" del "!REPORT!" >nul 2>&1
set /a DONE=0
set /a CRITOK=0
set "OPT_MISSING="

echo.
echo ============================================================
echo  Установка пакетов данных Geant4 !G4VER!
echo  Папка назначения: !DATATARGET!
echo ============================================================
echo.

rem --- 4. обязательные пакеты ---
call :pkg G4LEDATA G4EMLOW G4EMLOW.8.8 1 https://cern.ch/geant4-data/datasets/G4EMLOW.8.8.tar.gz
call :pkg G4RADIOACTIVEDATA RadioactiveDecay G4RadioactiveDecay.6.1.2 1 https://cern.ch/geant4-data/datasets/G4RadioactiveDecay.6.1.2.tar.gz
call :pkg G4LEVELGAMMADATA PhotonEvaporation G4PhotonEvaporation.6.1.2 1 https://cern.ch/geant4-data/datasets/G4PhotonEvaporation.6.1.2.tar.gz
call :pkg G4ENSDFSTATEDATA G4ENSDFSTATE G4ENSDFSTATE.3.0 1 https://cern.ch/geant4-data/datasets/G4ENSDFSTATE.3.0.tar.gz
call :pkg G4PARTICLEXSDATA G4PARTICLEXS G4PARTICLEXS.4.2 1 https://cern.ch/geant4-data/datasets/G4PARTICLEXS.4.2.tar.gz

rem --- 5. необязательные пакеты (ставятся, если архивы лежат рядом) ---
call :pkg G4NEUTRONHPDATA G4NDL G4NDL.4.7.1 0 https://cern.ch/geant4-data/datasets/G4NDL.4.7.1.tar.gz
call :pkg G4PIIDATA G4PII G4PII.1.3 0 https://cern.ch/geant4-data/datasets/G4PII.1.3.tar.gz
call :pkg G4REALSURFACEDATA RealSurface G4RealSurface.2.2 0 https://cern.ch/geant4-data/datasets/G4RealSurface.2.2.tar.gz
call :pkg G4SAIDXSDATA G4SAIDDATA G4SAIDDATA.2.0 0 https://cern.ch/geant4-data/datasets/G4SAIDDATA.2.0.tar.gz
call :pkg G4ABLADATA G4ABLA G4ABLA.3.3 0 https://cern.ch/geant4-data/datasets/G4ABLA.3.3.tar.gz
call :pkg G4INCLDATA G4INCL G4INCL.1.3 0 https://cern.ch/geant4-data/datasets/G4INCL.1.3.tar.gz
call :pkg G4CHANNELINGDATA G4CHANNELING G4CHANNELING.2.0 0 https://cern.ch/geant4-data/datasets/G4CHANNELING.2.0.tar.gz
call :pkg G4TENDLDATA G4TENDL G4TENDL.1.4 0 https://cern.ch/geant4-data/datasets/G4TENDL.1.4.tar.gz
call :pkg G4NUDEXLIBDATA G4NUDEXLIB G4NUDEXLIB.1.0 0 https://cern.ch/geant4-data/datasets/G4NUDEXLIB.1.0.tar.gz
call :pkg G4URRPTDATA G4URRPT G4URRPT.1.1 0 https://cern.ch/geant4-data/datasets/G4URRPT.1.1.tar.gz

if exist "!REPORT!" goto :missing

echo.
echo ============================================================
echo  Готово: обязательных пакетов !CRITOK! из 5 (всего !DONE! из 15).
if defined OPT_MISSING (
    echo.
    echo  Не установлены необязательные пакеты: !OPT_MISSING:~2!
    echo  На работу программы они не влияют. Скачать можно на странице
    echo  https://geant4.web.cern.ch/download/11.4.1.html , раздел Datasets.
)
echo  Теперь запустите start.bat.
echo ============================================================
echo.
pause
exit /b 0

rem ============================================================
rem  пакет: VAR, маска папки, имя архива, обязательный (1/0), ссылка
rem ============================================================
:pkg
set "VN=%~1"
set "PAT=%~2"
set "ASTEM=%~3"
set "CRIT=%~4"
set "URL=%~5"

rem папка уже распакована?
set "FND="
for /d %%D in ("!DATATARGET!\%PAT%*") do if not defined FND set "FND=%%~fD"
if defined FND (
    setx !VN! "!FND!" >nul 2>&1
    set "!VN!=!FND!"
    if "!CRIT!"=="1" echo   [ок]         %ASTEM% - уже стоит
    set /a DONE+=1
    if "!CRIT!"=="1" set /a CRITOK+=1
    exit /b 0
)

rem архив лежит рядом с этим батником?
set "ARCH="
for %%F in ("%~dp0%ASTEM%*.tar.gz") do if exist "%%~fF" if not defined ARCH set "ARCH=%%~fF"
if not defined ARCH (
    if "!CRIT!"=="1" (
        >>"!REPORT!" echo   %ASTEM%.tar.gz    скачать: !URL!
        echo   [нет]         %ASTEM%.tar.gz - архив не найден рядом с install_data.bat
    ) else (
        set "OPT_MISSING=!OPT_MISSING!, %ASTEM%"
    )
    exit /b 0
)

if not defined TAROK goto :no_tar

rem распаковка в папку data рядом с каталогом установки Geant4
if not exist "!DATATARGET!" mkdir "!DATATARGET!" >nul 2>&1
set "LOG=%TEMP%\g4_tar.log"
tar -xzf "!ARCH!" -C "!DATATARGET!" >"!LOG!" 2>&1
if errorlevel 1 (
    echo   [ошибка]     не удалось распаковать %ASTEM%.tar.gz, журнал:
    type "!LOG!"
    if "!CRIT!"=="1" >>"!REPORT!" echo   %ASTEM%.tar.gz    ошибка распаковки - удалите файл и скачайте заново
    exit /b 1
)
set "FND="
for /d %%D in ("!DATATARGET!\%PAT%*") do if not defined FND set "FND=%%~fD"
if not defined FND (
    echo   [ошибка]     после распаковки не найдена папка %PAT%
    if "!CRIT!"=="1" >>"!REPORT!" echo   %ASTEM%.tar.gz    после распаковки нет папки %PAT% - скачайте файл заново
    exit /b 1
)
setx !VN! "!FND!" >nul 2>&1
set "!VN!=!FND!"
echo   [распаковано] %ASTEM% - папка: !FND!
set /a DONE+=1
if "!CRIT!"=="1" set /a CRITOK+=1
exit /b 0

:missing
echo.
echo ============================================================
echo  Не хватает обязательных пакетов. Скачайте архивы .tar.gz по ссылкам:
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
echo Установите Geant4 11.4.1 - раздел 5.1 руководства (MANUAL.docx),
echo перезагрузите компьютер и запустите install_data.bat ещё раз.
echo.
pause
exit /b 1

:no_version
echo.
echo ОШИБКА: не удалось определить версию Geant4 (взят�� версия 11.4.1 по умолчанию).
echo.
echo Переустановите Geant4 11.4.1 - раздел 5.1 руководства (MANUAL.docx).
echo.
pause
exit /b 1
