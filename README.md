# geant4-autoradiolysis-beta

Симуляция радиолиза (радионуклидной дозовой нагрузки) в цилиндрическом резервуаре на Geant4.

## Возможности

- Три варианта геометрии: цилиндр, стакан (беакер) с дном и стенкой, пробирка со скруглённым дном.
- Объём среды «Цилиндр» зарегистрирован как чувствительный объём; рассчитывается поглощённая доза (Path A — энерговыделение, Path B — энергия распада/утечки, cross-check A-B).
- Выбор радионуклида (Z, A, энергия возбуждения), активности, плотности среды и стекла, толщины стенки и бортика.
- Поддержка многопоточности (MT), RadioactiveDecay, выход результатов в консоль и в `results.txt`.

## Установка

Скачайте готовый архив со страницы релизов:

https://github.com/timishavet/geant4-autoradiolysis-beta/releases/latest

Требования:

- Windows 10/11 x64.
- Geant4 11.4.1 с поддержкой UI Win32 и OpenGL Win32.
- Каталог `bin` Geant4 добавлен в `PATH`.
- Заданы переменные `G4*DATA`.
- Python 3.12 или новее x64 с tkinter.
- Visual C++ Redistributable x64.

Запуск GUI:

```powershell
py program_b.py
```

Подробная установка и проверка описаны в `MANUAL.md` и `MANUAL.docx`.

## Сборка из исходников

Требуется Geant4 11.4.1 (с геометрией, физикой, визуализацией), CMake и Visual Studio с рабочей нагрузкой C++. Укажите путь к установке Geant4 через `Geant4_DIR`:

```powershell
$G4 = "C:\Geant4\geant4_11.4.1"
cmake -S . -B build -DGeant4_DIR="$G4/lib/cmake/Geant4"
cmake --build build --config Release --target main
```

## Запуск

- Интерактивный режим с GUI: `interactive.bat` (или `./build/Release/main.exe` без аргументов).
- Пакетный режим: `./build/Release/main.exe run.mac`.
- Графический интерфейс построения макроса и запуска: `py program_b.py`.

## Структура

- `main.cc` — точка входа, настройка RunManager(MT), физика, UI.
- `src/include` — пользовательские классы Geant4 (детектор, генератор, действия, конфигурации и мессенджеры).
- `vis.mac`, `gui.mac` — макросы визуализации и GUI-меню.
- `program_b.py` — GUI-обёртка (tkinter) для выбора параметров и запуска симуляции.