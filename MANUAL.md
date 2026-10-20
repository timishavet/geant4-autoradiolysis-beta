# geant4-autoradiolysis-beta

## Руководство по установке зависимостей и программы

Версия документа: 1.0  
Дата проверки: 20.10.2026  
Платформа: Windows 10/11 x64

---

## 1. Назначение документа

Документ описывает:

1. установку всех зависимостей, необходимых для сборки и запуска geant4-autoradiolysis-beta;
2. установку самой программы;
3. проверку правильности установки;
4. запуск программы после установки.

geant4-autoradiolysis-beta — пакет программ на C++/Geant4 для моделирования радиолиза в цилиндрической ёмкости. Пакет состоит из:

- исполняемого файла `main.exe` (C++/Geant4);
- графического интерфейса `program_b.py` (Python/tkinter).

---

## 2. Требования к системе

### 2.1. Минимальные требования

| Компонент | Требование |
|---|---|
| Операционная система | Windows 10 x64 или Windows 11 x64 |
| Процессор | x64, 2 ядра |
| Оперативная память | 8 ГБ |
| Свободное место на диске | не менее 8 ГБ |
| Права | установка зависимостей и изменение переменных среды |
| Графический адаптер | поддержка OpenGL (для интерактивного режима) |

### 2.2. Рекомендуемые требования

| Компонент | Рекомендация |
|---|---|
| Процессор | 8 ядер и больше |
| Оперативная память | 16 ГБ и больше |
| Свободное место на диске | 15 ГБ и больше |
| Python | 3.12 или новее x64 |
| Visual Studio | 2022 или 2026 с рабочей нагрузкой C++ |

### 2.3. Проверенная конфигурация

Установка и запуск проверены на следующей конфигурации:

| Компонент | Версия |
|---|---|
| Windows | Windows 11 Pro x64, 10.0.26200 |
| Geant4 | 11.4.1 |
| CMake | 4.3.1 |
| Visual Studio | 2026 Insiders, MSVC 14.51.36231 |
| Python | 3.14.5 |
| tkinter | 8.6 |
| matplotlib | 3.10.9 |

---

## 3. Состав зависимостей

### 3.1. Обязательные зависимости

| Зависимость | Назначение | Проверка |
|---|---|---|
| Windows 10/11 x64 | операционная система | `winver` |
| Geant4 11.4.x | физика, геометрия, радиоактивный распад | `Test-Path "<путь_Geant4>\lib\cmake\Geant4\Geant4Config.cmake"` |
| Наборы данных Geant4 | данные ядер, распадов, физики частиц | `$env:G4RADIOACTIVEDATA` |
| `<путь_Geant4>\bin` в `PATH` | DLL Geant4 при запуске `main.exe` | `where.exe geant4.bat` |
| Visual C++ Runtime | системные DLL для `main.exe` | запуск `main.exe` |
| Python 3.12 или новее x64 | графический интерфейс `program_b.py` | `py --version` |
| tkinter | графическая библиотека Python | `py -c "import tkinter"` |
| CMake 3.16+ | сборка C++-части | `cmake --version` |
| Visual Studio с C++ | компилятор MSVC и Windows SDK | `cl` в Developer Prompt |

### 3.2. Необязательные зависимости

| Зависимость | Назначение | Установка |
|---|---|---|
| matplotlib | построение графиков бета-спектров | `py -m pip install matplotlib` |
| Git | клонирование репозитория | с официального сайта Git |
| MS Word | только для просмотра `.docx`-версии мануала | не требуется для программы |

### 3.3. Важные замечания

- Программа рассчитана на Geant4 **11.4.x**. Geant4 11.3 и более старые версии не подходят.
- Для данной сборки Geant4 Qt не требуется: используются UI Win32 и визуализация OpenGL Win32.
- Python-пакеты, кроме необязательного `matplotlib`, не требуются.
- Если команда `python` не найдена, используйте `py`. На проверенной машине `python` отсутствует в `PATH`, а `py` работает.
- После изменения переменных среды закройте и снова откройте терминал.

---

## 4. Установка зависимостей

### 4.1. Visual Studio с рабочей нагрузкой C++

1. Скачайте Visual Studio Community, Professional или Enterprise с официального сайта Microsoft.
2. Запустите установщик Visual Studio Installer.
3. Выберите рабочую нагрузку:

   **Разработка классических приложений на C++ / Desktop development with C++**

4. В правой части окна установщика убедитесь, что выбраны:

   - MSVC — средства сборки C++ x64/x86;
   - Windows 10 SDK или Windows 11 SDK;
   - C++ CMake tools for Windows (необязательно, если CMake устанавливается отдельно).

5. Нажмите «Установить» и дождитесь завершения.
6. Перезагрузите компьютер, если установщик этого потребует.

Проверка:

1. Откройте «Developer Command Prompt for VS» или «Developer PowerShell for VS».
2. Выполните:

```bat
cl
```

Ожидается вывод версии компилятора Microsoft C/C++.

Если `cl` не найден, откройте Visual Studio Installer, нажмите «Изменить» и добавьте компоненты C++.

### 4.2. CMake

1. Скачайте установщик CMake для Windows x64 с официального сайта CMake.
2. При установке выберите:

   - **Add CMake to the system PATH for all users**  
     или  
   - **Add CMake to the system PATH for the current user**.

3. Завершите установку.
4. Закройте и снова откройте терминал.

Проверка:

```powershell
cmake --version
```

Ожидается строка вида:

```text
cmake version 4.3.1
```

Подходит CMake 3.16 и новее. Более старые версии не подходят.

### 4.3. Geant4 11.4.1

#### 4.3.1. Установка

1. Скачайте Windows-установщик Geant4 11.4.1 с официальной страницы загрузки Geant4.
2. Запустите установщик.
3. Укажите каталог установки. Путь может быть любым, например:

```text
C:\Geant4\geant4_11.4.1
```

Запомните этот путь: он понадобится для переменных среды, `PATH` и CMake. В примерах ниже используется переменная PowerShell `$G4`; замените её значение на свой путь:

```powershell
$G4 = "C:\Geant4\geant4_11.4.1"
```

4. В компонентах установки включите:

   - библиотеки Geant4;
   - заголовочные файлы;
   - наборы данных Geant4 (Data files);
   - UI Win32;
   - визуализацию OpenGL Win32.

5. Установку примеров (examples) можно отключить, если не планируется их использование.
6. Дождитесь завершения установки.

#### 4.3.2. Переменные среды

Установщик Geant4 обычно создаёт переменные среды автоматически. Если после установки переменные не заданы, их нужно создать вручную.

Проверка:

```powershell
Get-ChildItem Env: | Where-Object Name -like 'G4*DATA'
```

Для Geant4 11.4.1 должны быть заданы переменные:

| Переменная | Каталог внутри `share\Geant4\data` |
|---|---|
| `G4ABLADATA` | `G4ABLA3.3` |
| `G4CHANNELINGDATA` | `G4CHANNELING2.0` |
| `G4ENSDFSTATEDATA` | `G4ENSDFSTATE3.0` |
| `G4INCLDATA` | `G4INCL1.3` |
| `G4LEDATA` | `G4EMLOW8.8` |
| `G4LEVELGAMMADATA` | `PhotonEvaporation6.1.2` |
| `G4NEUTRONHPDATA` | `G4NDL4.7.1` |
| `G4NUDEXLIBDATA` | `G4NUDEXLIB1.0` |
| `G4PARTICLEXSDATA` | `G4PARTICLEXS4.2` |
| `G4PIIDATA` | `G4PII1.3` |
| `G4RADIOACTIVEDATA` | `RadioactiveDecay6.1.2` |
| `G4REALSURFACEDATA` | `RealSurface2.2` |
| `G4SAIDXSDATA` | `G4SAIDDATA2.0` |
| `G4TENDLDATA` | `G4TENDL1.4` |
| `G4URRPTDATA` | `G4URRPT1.1` |

Если установщик не создал переменные, откройте PowerShell от имени администратора и выполните:

```powershell
$G4 = "C:\Geant4\geant4_11.4.1"
$data = Join-Path $G4 "share\Geant4\data"

$vars = @{
    "G4ABLADATA"        = "G4ABLA3.3"
    "G4CHANNELINGDATA"  = "G4CHANNELING2.0"
    "G4ENSDFSTATEDATA"  = "G4ENSDFSTATE3.0"
    "G4INCLDATA"        = "G4INCL1.3"
    "G4LEDATA"          = "G4EMLOW8.8"
    "G4LEVELGAMMADATA"  = "PhotonEvaporation6.1.2"
    "G4NEUTRONHPDATA"   = "G4NDL4.7.1"
    "G4NUDEXLIBDATA"    = "G4NUDEXLIB1.0"
    "G4PARTICLEXSDATA"  = "G4PARTICLEXS4.2"
    "G4PIIDATA"         = "G4PII1.3"
    "G4RADIOACTIVEDATA" = "RadioactiveDecay6.1.2"
    "G4REALSURFACEDATA" = "RealSurface2.2"
    "G4SAIDXSDATA"      = "G4SAIDDATA2.0"
    "G4TENDLDATA"       = "G4TENDL1.4"
    "G4URRPTDATA"       = "G4URRPT1.1"
}

foreach ($item in $vars.GetEnumerator()) {
    $path = Join-Path $data $item.Value
    if (Test-Path $path) {
        [Environment]::SetEnvironmentVariable($item.Key, $path, "Machine")
    } else {
        Write-Host "Не найден каталог: $path"
    }
}
```

Если нужны переменные только для текущего пользователя, замените `"Machine"` на `"User"`.

После создания переменных закройте и снова откройте терминал.

#### 4.3.3. Добавление Geant4 в PATH

В `PATH` должен находиться каталог `bin` установленного Geant4, например:

```text
C:\Geant4\geant4_11.4.1\bin
```

Точный путь зависит от того, куда был установлен Geant4.

Проверка:

```powershell
where.exe geant4.bat
```

Если команда не находит файл, добавьте каталог `bin` установленного Geant4 в `PATH`:

1. Откройте «Параметры Windows» → «Система» → «О системе» → «Дополнительные параметры системы».
2. Нажмите «Переменные среды».
3. В разделе `Path` нажмите «Изменить».
4. Добавьте путь к каталогу `bin` Geant4, например:

```text
C:\Geant4\geant4_11.4.1\bin
```

5. Сохраните изменения и перезапустите терминал.

Альтернативно, в PowerShell от имени администратора:

```powershell
$G4 = "C:\Geant4\geant4_11.4.1"
$g4bin = Join-Path $G4 "bin"
$p = [Environment]::GetEnvironmentVariable("Path", "Machine")
if (($p -split ';') -notcontains $g4bin) {
    [Environment]::SetEnvironmentVariable("Path", "$p;$g4bin", "Machine")
}
```

### 4.4. Visual C++ Runtime

Для запуска `main.exe` нужны системные библиотеки Visual C++:

- `vcruntime140.dll`;
- `msvcp140.dll`;
- другие библиотеки семейства `vcruntime140`.

Обычно они уже установлены вместе с Visual Studio или Geant4. Если при запуске появляется ошибка о missing DLL, установите **Microsoft Visual C++ Redistributable x64** с официального сайта Microsoft.

### 4.5. Python и tkinter

1. Скачайте Python 3.12 или новее (x64) с официального сайта Python.
2. Запустите установщик.
3. Обязательно включите:

   - **Add python.exe to PATH**;
   - **py launcher** (обычно включён по умолчанию);
   - **tcl/tk and IDLE** (входит в стандартную установку).

4. Завершите установку.
5. Закройте и снова откройте терминал.

Проверка:

```powershell
py --version
py -c "import tkinter; print(tkinter.TkVersion)"
```

Ожидается версия Python 3.12 или новее и версия tkinter:

```text
3.13.7
8.6
```

Если команда `python` не работает, используйте `py`. Это нормально для конфигурации, где отключён псевдоним `python.exe`.

### 4.6. matplotlib (необязательно)

`matplotlib` нужен только для построения графиков на вкладке «Спектры» в `program_b.py`. Без него остальные функции программы работают.

Установка:

```powershell
py -m pip install matplotlib
```

Проверка:

```powershell
py -c "import matplotlib; print(matplotlib.__version__)"
```

---

## 5. Проверка зависимостей

Выполните команды в новом терминале после установки всех компонентов.

### 5.1. CMake

```powershell
cmake --version
```

### 5.2. Python и tkinter

```powershell
py --version
py -c "import tkinter; print('tkinter OK', tkinter.TkVersion)"
```

### 5.3. Geant4

Замените `$G4` на фактический путь установки Geant4:

```powershell
$G4 = "C:\Geant4\geant4_11.4.1"
Test-Path "$G4\lib\cmake\Geant4\Geant4Config.cmake"
Test-Path "$env:G4RADIOACTIVEDATA\z71.a177"
Test-Path "$env:G4LEVELGAMMADATA\z39.a90"
where.exe geant4.bat
```

Все команды `Test-Path` должны вернуть `True`.

### 5.4. Наборы данных

```powershell
Get-ChildItem Env: |
    Where-Object Name -like 'G4*DATA' |
    Sort-Object Name |
    Format-Table Name, Value -AutoSize
```

В списке должны присутствовать все 15 переменных из таблицы раздела 4.3.2.

### 5.5. Компилятор

Откройте «Developer Command Prompt for VS» и выполните:

```bat
cl
```

---

## 6. Установка geant4-autoradiolysis-beta

### 6.1. Получение файлов программы

Скачайте архив последнего релиза:

```text
https://github.com/timishavet/geant4-autoradiolysis-beta/releases/latest
```

Имя файла:

```text
geant4-autoradiolysis-beta-v0.1.0-win64.zip
```

Распакуйте архив в удобную папку, например:

```text
C:\geant4-autoradiolysis-beta
```

В архиве уже есть готовый `build\Release\main.exe`, поэтому Visual Studio и CMake для установки не нужны.

Если нужна сборка из исходников, клонируйте репозиторий:

```powershell
git clone <URL-репозитория> geant4-autoradiolysis-beta
```

Если Git не установлен, скачайте исходники на странице релиза.

Путь с пробелами, например `C:\Users\Имя\Desktop\OpenCode projects\geant4-autoradiolysis-beta`, тоже работает, но для первых запусков лучше использовать простой путь.

### 6.2. Вариант A. Готовая сборка из релиза

1. Установите Geant4 11.4.1 и добавьте его каталог `bin` в `PATH`.
2. Убедитесь, что заданы переменные `G4*DATA`.
3. Распакуйте архив релиза.
4. Откройте терминал в папке программы.
5. Проверьте наличие файла:

```powershell
Test-Path ".\build\Release\main.exe"
```

6. Перейдите к разделу 7 «Проверка запуска».

Готовая сборка рассчитана на Geant4 11.4.1 x64. Если установлена другая версия Geant4, соберите программу из исходников по варианту B.

### 6.3. Вариант B. Сборка из исходников

#### 6.3.1. Подготовка

1. Установите Visual Studio с рабочей нагрузкой C++.
2. Установите CMake.
3. Установите Geant4 11.4.1.
4. Убедитесь, что существует файл:

```text
<путь_Geant4>\lib\cmake\Geant4\Geant4Config.cmake
```

5. Откройте терминал в папке проекта:

```powershell
cd C:\geant4-autoradiolysis-beta
```

#### 6.3.2. Конфигурация CMake

Укажите фактический путь установки Geant4 в переменной `$G4` и выполните:

```powershell
$G4 = "C:\Geant4\geant4_11.4.1"
cmake -S . -B build -DGeant4_DIR="$G4/lib/cmake/Geant4"
```

Если Geant4 установлен в другой каталог, измените только значение `$G4`.

В пути можно использовать прямые слэши `/` или двойные обратные `\\`.

Если нужно явно выбрать генератор Visual Studio, добавьте параметр `-G`. Например, для Visual Studio 2022:

```powershell
$G4 = "C:\Geant4\geant4_11.4.1"
cmake -S . -B build -G "Visual Studio 17 2022" -A x64 -DGeant4_DIR="$G4/lib/cmake/Geant4"
```

Если генератор не указан, CMake выбирает установленный Visual Studio автоматически.

#### 6.3.3. Сборка

```powershell
cmake --build build --config Release --target main
```

После успешной сборки появится файл:

```text
build\Release\main.exe
```

Проверка:

```powershell
Test-Path ".\build\Release\main.exe"
```

Команда должна вернуть `True`.

#### 6.3.4. Повторная сборка

Если в проекте появились новые файлы `.cc` или `.hh`, выполните конфигурацию CMake заново, затем сборку:

```powershell
$G4 = "C:\Geant4\geant4_11.4.1"
cmake -S . -B build -DGeant4_DIR="$G4/lib/cmake/Geant4"
cmake --build build --config Release --target main
```

Это связано с тем, что список исходных файлов собирается через `file(GLOB ...)` и не обновляется автоматически.

#### 6.3.5. Если каталог `build` создан на другом компьютере

Удалите старый каталог сборки и создайте его заново:

```powershell
Remove-Item -Recurse -Force build
$G4 = "C:\Geant4\geant4_11.4.1"
cmake -S . -B build -DGeant4_DIR="$G4/lib/cmake/Geant4"
cmake --build build --config Release --target main
```

---

## 7. Проверка запуска

### 7.1. Подготовка терминала

1. Откройте новый терминал.
2. Перейдите в папку проекта:

```powershell
cd C:\geant4-autoradiolysis-beta
```

3. Проверьте, что Geant4 доступен:

```powershell
where.exe geant4.bat
```

4. Проверьте переменные данных:

```powershell
Test-Path "$env:G4RADIOACTIVEDATA\z39.a90"
```

### 7.2. Быстрый тест в пакетном режиме

Создайте в папке проекта файл `check_install.mac` со следующим содержимым:

```text
/myDetector/setGeometry cylinder
/myDetector/setRadius 2 cm
/myDetector/setHalfHeight 3 cm
/myDetector/setMaterial G4_WATER
/mySource/setRadionuclide 39 90
/run/initialize
/process/had/rdm/thresholdForVeryLongDecayTime 1.0e+60 year
/run/beamOn 10
```

Запустите:

```powershell
.\build\Release\main.exe check_install.mac
```

Признаки успешного запуска:

- программа завершилась без ошибок;
- в консоли появился блок `SIMULATION RESULTS`;
- в папке проекта создан или дополнен файл `results.txt`;
- в результатах есть `Number of decays: 10`;
- поглощённая доза `Absorbed dose (A)` больше нуля.

Пример ожидаемого вывода:

```text
========================================
  SIMULATION RESULTS
========================================
Radionuclide:        Y-90 (Z=39, A=90)
Number of decays:    10
Geometry variant:    cylinder
Volume:              75.3982 cm3
Material:            G4_WATER
...
Absorbed dose (A):   2.7646e-11 Gy
...
========================================
```

### 7.3. Запуск графического интерфейса

Из папки проекта:

```powershell
py program_b.py
```

Откроется окно «Программа Б — Генератор макросов».

Проверка установки:

1. Убедитесь, что вкладки открываются без ошибок.
2. На вкладке «Радионуклид» выберите, например, `Lu-177`.
3. Убедитесь, что отображается период полураспада и сообщение о наличии данных Geant4.
4. На вкладке «Запуск и вывод» нажмите «Запустить симуляцию».
5. Дождитесь завершения и проверьте блок результатов.

Если вкладка «Спектры» сообщает об отсутствии `matplotlib`, установите его:

```powershell
py -m pip install matplotlib
```

### 7.4. Интерактивный режим

Запуск без аргументов:

```powershell
.\build\Release\main.exe
```

или:

```powershell
.\interactive.bat
```

В интерактивном режиме загружаются `vis.mac` и `gui.mac`, открывается визуализация Geant4. Для этого нужна поддержка OpenGL.

---

## 8. Типовые ошибки установки

### 8.1. `CMake Error: Could not find Geant4`

Причина: неверный путь `Geant4_DIR`.

Решение:

```powershell
$G4 = "C:\Geant4\geant4_11.4.1"
Test-Path "$G4\lib\cmake\Geant4\Geant4Config.cmake"
```

Укажите правильный путь:

```powershell
$G4 = "C:\Geant4\geant4_11.4.1"
cmake -S . -B build -DGeant4_DIR="$G4/lib/cmake/Geant4"
```

### 8.2. `main.exe` не запускается, отсутствуют DLL

Причина: каталог `bin` установленного Geant4 отсутствует в `PATH`.

Решение:

1. Добавьте каталог в `PATH` по разделу 4.3.3.
2. Закройте и снова откройте терминал.
3. Проверьте:

```powershell
where.exe geant4.bat
```

### 8.3. `python` не является внутренней или внешней командой

Причина: команда `python` отсутствует в `PATH`.

Решение: используйте `py`:

```powershell
py program_b.py
```

### 8.4. `ModuleNotFoundError: No module named 'matplotlib'`

Причина: не установлен matplotlib.

Решение:

```powershell
py -m pip install matplotlib
```

Программа работает и без matplotlib, но вкладка «Спектры» не будет строить графики.

### 8.5. `ModuleNotFoundError: No module named 'tkinter'`

Причина: Python установлен без tcl/tk.

Решение: запустите установщик Python, выберите «Modify» и включите компонент **tcl/tk and IDLE**.

### 8.6. Ошибка компиляции, связанная с Geant4 11.3

Причина: используется неподходящая версия Geant4.

Решение: установите Geant4 11.4.x и заново выполните конфигурацию CMake:

```powershell
Remove-Item -Recurse -Force build
$G4 = "C:\Geant4\geant4_11.4.1"
cmake -S . -B build -DGeant4_DIR="$G4/lib/cmake/Geant4"
cmake --build build --config Release --target main
```

### 8.7. `Illegal application state`

Причина: команды настройки геометрии или радионуклида выполнены после `/run/initialize`.

Решение: в макросе команды `/myDetector/*` и `/mySource/setRadionuclide` должны находиться до `/run/initialize`.

### 8.8. Нулевая доза и сообщение об отсутствии вторичных частиц

Возможные причины:

- не задана переменная `G4RADIOACTIVEDATA`;
- отсутствуют файлы данных для выбранного нуклида;
- в макросе не снят порог очень долгого распада.

Проверьте:

```powershell
Test-Path "$env:G4RADIOACTIVEDATA\z71.a177"
```

В макросе должна быть строка:

```text
/process/had/rdm/thresholdForVeryLongDecayTime 1.0e+60 year
```

### 8.9. CMake не находит Visual Studio

Причина: не установлена рабочая нагрузка C++ или Windows SDK.

Решение: откройте Visual Studio Installer, нажмите «Изменить» и установите:

- «Разработка классических приложений на C++»;
- Windows 10/11 SDK;
- MSVC x64/x86.

### 8.10. Не строятся графики спектров

Причина: не установлен matplotlib.

Решение:

```powershell
py -m pip install matplotlib
```

---

## 9. Обновление и удаление

### 9.1. Обновление программы

1. Замените файлы проекта новой версией.
2. Если изменились исходники C++, пересоберите программу:

```powershell
$G4 = "C:\Geant4\geant4_11.4.1"
cmake -S . -B build -DGeant4_DIR="$G4/lib/cmake/Geant4"
cmake --build build --config Release --target main
```

3. Проверьте запуск по разделу 7.

### 9.2. Удаление программы

1. Закройте `program_b.py` и `main.exe`.
2. Удалите папку проекта.
3. При необходимости удалите `build` отдельно, если он находится вне папки проекта.

Зависимости (Geant4, Python, CMake, Visual Studio) удаляются стандартными средствами Windows, если они больше не нужны.

---

## 10. Краткий контрольный список

Перед первым запуском проверьте:

- [ ] Windows 10/11 x64.
- [ ] Visual Studio с рабочей нагрузкой C++.
- [ ] CMake 3.16 или новее.
- [ ] Geant4 11.4.1 установлен.
- [ ] Все 15 переменных `G4*DATA` заданы.
- [ ] Каталог `bin` установленного Geant4 добавлен в `PATH`.
- [ ] Python 3.12 или новее установлен.
- [ ] tkinter работает.
- [ ] matplotlib установлен, если нужны графики спектров.
- [ ] Проект скопирован на диск.
- [ ] `build\Release\main.exe` собран или присутствует.
- [ ] `check_install.mac` выполняется без ошибок.
- [ ] `py program_b.py` открывает окно программы.
