import tkinter as tk
from tkinter import ttk, messagebox, filedialog, scrolledtext
import math, os, datetime, re, subprocess, threading, time
from collections import deque

# ============================================================
# DATA TABLES
# ============================================================

RADIONUCLIDES = [
    {"Z":71, "A":177, "name":"Lu-177", "desc":"Лютеций-177. β⁻-излучатель (E_βmax ≈ 497 кэВ). Таргетная радиотерапия нейроэндокринных опухолей (Lutathera), радиосиновэктомия."},
    {"Z":53, "A":131, "name":"I-131", "desc":"Йод-131. β⁻-излучатель (E_βmax ≈ 606 кэВ) + γ. Терапия гипертиреоза и рака щитовидной железы."},
    {"Z":39, "A":90,  "name":"Y-90", "desc":"Иттрий-90. Чистый β⁻-излучатель (E_βmax ≈ 2.28 МэВ). Радиоэмболизация печени (SIR-Spheres, TheraSphere)."},
    {"Z":88, "A":223, "name":"Ra-223", "desc":"Радий-223. α-излучатель (5–7.5 МэВ). Лечение метастазов в костях (Xofigo)."},
    {"Z":89, "A":225, "name":"Ac-225", "desc":"Актиний-225. α-каскад из 4 α-частиц. Перспективная α-терапия (PSMA-TAT)."},
    {"Z":43, "A":99,  "name":"Tc-99m", "desc":"Технеций-99m. γ-излучатель (140 кэВ). Основной диагностический изотоп в ОФЭКТ.", "exc": 142.6836},
    {"Z":9,  "A":18,  "name":"F-18", "desc":"Фтор-18. β⁺-излучатель (0.633 МэВ). ПЭТ-диагностика (FDG)."},
    {"Z":53, "A":125, "name":"I-125", "desc":"Йод-125. γ + AE (27–35 кэВ). Брахитерапия предстательной железы, офтальмоаппликаторы."},
    {"Z":62, "A":153, "name":"Sm-153", "desc":"Самарий-153. β⁻ (E_βmax ≈ 810 кэВ). Паллиативная терапия костных метастазов (Quadramet)."},
    {"Z":75, "A":186, "name":"Re-186", "desc":"Рений-186. β⁻ (1.07 МэВ). Брахитерапия, радиосиновэктомия."},
    {"Z":75, "A":188, "name":"Re-188", "desc":"Рений-188. β⁻ (2.12 МэВ). Брахитерапия, радиоэмболизация."},
    {"Z":27, "A":60,  "name":"Co-60", "desc":"Кобальт-60. β⁻ + γ (1.17, 1.33 МэВ). Телетерапия (Рокус), стерилизация."},
    {"Z":55, "A":137, "name":"Cs-137", "desc":"Цезий-137. β⁻ + γ (662 кэВ). Брахитерапия, дозиметрическая калибровка."},
    {"Z":11, "A":22,  "name":"Na-22", "desc":"Натрий-22. β⁺ + γ (1.275 МэВ). Позитронный источник, калибровка."},
    {"Z":63, "A":152, "name":"Eu-152", "desc":"Европий-152. γ-многолинейный спектр. Калибровочный γ-источник."},
]

MATERIALS = [
    {"name":"G4_WATER",                "label":"Вода (H₂O)",                       "density":"1.000 г/см³", "desc":"Эталонная модель биологической ткани. Стандарт для дозиметрии."},
]

GLASS_MATERIALS = [
    {"name":"G4_SILICON_DIOXIDE","label":"Стекло (SiO₂)",   "density":"2.320 г/см³", "desc":"Плавленый кварц / кварцевое стекло. Стандартный материал ампул и пробирок."},
]

GEOMETRY_VARIANTS = [
    {"token":"cylinder","label":"Цилиндр"},
    {"token":"beaker",  "label":"Стакан"},
    {"token":"tube",    "label":"Пробирка"},
]

OUTPUT_FIELDS = [
    {"id":"material_name",      "label":"Наименование среды",                         "group":"medium",      "def":True},
    {"id":"material_density",   "label":"Плотность среды (г/см³)",                    "group":"medium",      "def":True},
    {"id":"mass",               "label":"Масса (г)",                                  "group":"medium",      "def":True},
    {"id":"volume",             "label":"Объём (см³)",                                "group":"medium",      "def":True},
    {"id":"dimensions",         "label":"Размеры цилиндра (см)",                      "group":"medium",      "def":False},
    {"id":"geometry_variant",   "label":"Вариант геометрии",                          "group":"geometry",    "def":True},
    {"id":"wall_thickness",     "label":"Толщина стенки оболочки (см)",               "group":"geometry",    "def":True},
    {"id":"rim",                "label":"Сухой бортик над жидкостью (см)",            "group":"geometry",    "def":True},
    {"id":"glass_material",     "label":"Материал оболочки",                          "group":"geometry",    "def":True},
    {"id":"radionuclide",       "label":"Радионуклид (Z, A, имя)",                    "group":"radionuclide","def":True},
    {"id":"activity",           "label":"Активность источника (Бк)",                  "group":"radionuclide","def":True},
    {"id":"irr_time",           "label":"Время облучения (с)",                        "group":"radionuclide","def":True},
    {"id":"num_events",         "label":"Число распадов",                             "group":"results",     "def":True},
    {"id":"edep_energy",        "label":"Поглощённая энергия (МэВ) — Path A",         "group":"results",     "def":True},
    {"id":"edep_dose",          "label":"Поглощённая доза (Гр) — Path A",             "group":"results",     "def":True},
    {"id":"decay_energy",       "label":"Энергия распада (МэВ) — Path B in",          "group":"results",     "def":True},
    {"id":"escaped_energy",     "label":"Убежавшая энергия (МэВ) — Path B out",       "group":"results",     "def":True},
    {"id":"transferred_energy", "label":"Переданная энергия (МэВ) — Path B net",      "group":"results",     "def":True},
    {"id":"path_b_dose",        "label":"Доза Path B (Гр)",                           "group":"results",     "def":True},
    {"id":"cross_check",        "label":"Cross-check A−B (МэВ)",                      "group":"results",     "def":True},
    {"id":"energy_per_decay",   "label":"Энергия на распад (МэВ)",                    "group":"results",     "def":False},
    {"id":"timestamp",          "label":"Дата и время",                               "group":"simconfig",   "def":True},
    {"id":"threading_mode",     "label":"Режим поточности",                           "group":"simconfig",   "def":True},
    {"id":"num_threads",        "label":"Число потоков",                              "group":"simconfig",   "def":True},
    {"id":"sim_wall_time",      "label":"Время симуляции (с)",                        "group":"simconfig",   "def":False},
    {"id":"avg_time_per_event", "label":"Среднее время на событие (мс)",              "group":"simconfig",   "def":False},
]

GROUP_TITLES = {
    "medium":      "Информация о среде",
    "geometry":    "Информация о геометрии",
    "radionuclide":"Информация о радионуклиде",
    "results":     "Информация о результатах эксперимента",
    "simconfig":   "Информация о настройке и протекании симуляции",
}

UNIT_TO_CM = {"мм":0.1, "см":1, "м":100}

# ============================================================
# HELPER FUNCTIONS
# ============================================================

def to_cm(value, unit):
    try:
        n = float(value)
    except (ValueError, TypeError):
        return None
    k = UNIT_TO_CM.get(unit)
    return None if k is None else n * k

def format_number(value):
    try:
        n = float(value)
    except (ValueError, TypeError):
        return None
    if n == int(n) and abs(n) < 1e15:
        return str(int(n))
    s = f"{n:.6f}".rstrip("0").rstrip(".")
    return s

# ============================================================
# GEANT4 DECAY DATA (half-life + decay scheme)
# Paths ONLY from environment (per-machine). No hardcoded fallbacks.
# ============================================================

NMAX_EVENTS = 2147483647
ACTIVITY_UNITS = {"Бк":1.0, "кБк":1e3, "МБк":1e6, "ГБк":1e9, "мКи":3.7e7, "Ки":3.7e10}
TIME_UNITS = {"с":1.0, "мин":60.0, "ч":3600.0, "сут":86400.0}
DECAY_MODES = ["По числу распадов", "По активности и времени"]
EXC_TOL_KEV = 1.0

_MODE_TARGETS = {
    "BetaMinus": lambda z, a: (z + 1, a),
    "BetaPlus":  lambda z, a: (z - 1, a),
    "KshellEC":  lambda z, a: (z - 1, a),
    "LshellEC":  lambda z, a: (z - 1, a),
    "Alpha":     lambda z, a: (z - 2, a - 4),
    "IT":        lambda z, a: (z, a),
}

_RD_AVAIL_CACHE = {"dir": None, "set": None}
_PE_AVAIL_CACHE = {"dir": None, "set": None}

def get_geant4_data_dirs():
    rd = os.environ.get("G4RADIOACTIVEDATA", "")
    pe = os.environ.get("G4LEVELGAMMADATA", "")
    rd = rd if rd and os.path.isdir(rd) else None
    pe = pe if pe and os.path.isdir(pe) else None
    return rd, pe

def _list_isotopes_cached(path, cache):
    if path is None:
        return set()
    if cache["dir"] == path and cache["set"] is not None:
        return cache["set"]
    names = set()
    try:
        for fn in os.listdir(path):
            if len(fn) > 1 and fn[0] == "z" and ".a" in fn:
                try:
                    zpart, apart = fn[1:].split(".a")
                    names.add((int(zpart), int(apart)))
                except ValueError:
                    continue
    except OSError:
        return set()
    cache["dir"] = path
    cache["set"] = names
    return names

def parse_halflife_seconds(z, a, exc_kev=0.0):
    """Returns (state, halflife_s). state: ok | no_base | no_nuclide."""
    rd_dir, _ = get_geant4_data_dirs()
    if rd_dir is None:
        return ("no_base", None)
    path = os.path.join(rd_dir, f"z{z}.a{a}")
    if not os.path.isfile(path):
        return ("no_nuclide", None)
    best = None
    try:
        f = open(path, encoding="utf-8", errors="replace")
    except OSError:
        return ("no_nuclide", None)
    with f:
        for line in f:
            parts = line.split()
            if len(parts) < 4 or parts[0] != "P":
                continue
            try:
                exc = float(parts[1])
                hl = float(parts[3])
            except ValueError:
                continue
            if abs(exc - exc_kev) <= EXC_TOL_KEV:
                return ("ok", hl)
            if best is None:
                best = (exc, hl)
    return ("no_nuclide", None)

def parse_daughters(z, a):
    rd_dir, _ = get_geant4_data_dirs()
    if rd_dir is None:
        return set()
    path = os.path.join(rd_dir, f"z{z}.a{a}")
    daughters = set()
    try:
        f = open(path, encoding="utf-8", errors="replace")
    except OSError:
        return daughters
    with f:
        for line in f:
            s = line.strip()
            if not s or s[0] in ("#", "P", "N"):
                continue
            mode = s.split()[0]
            if mode in _MODE_TARGETS:
                daughters.add(_MODE_TARGETS[mode](z, a))
    return daughters

def check_nuclide(z, a, exc_kev=0.0):
    """Full check. Returns dict with keys: state, halflife, daughters, missing_pe."""
    rd_dir, pe_dir = get_geant4_data_dirs()
    if rd_dir is None:
        return {"state": "no_base", "halflife": None, "daughters": set(), "missing_pe": []}
    rd_avail = _list_isotopes_cached(rd_dir, _RD_AVAIL_CACHE)
    if (z, a) not in rd_avail:
        return {"state": "no_nuclide", "halflife": None, "daughters": set(), "missing_pe": []}
    state, hl = parse_halflife_seconds(z, a, exc_kev)
    if state != "ok":
        return {"state": "no_nuclide", "halflife": None, "daughters": set(), "missing_pe": []}
    daughters = parse_daughters(z, a)
    missing = []
    if pe_dir is not None:
        pe_avail = _list_isotopes_cached(pe_dir, _PE_AVAIL_CACHE)
        missing = sorted((dz, da) for dz, da in daughters
                         if (dz, da) not in pe_avail and (dz, da) != (z, a))
    return {"state": "ok", "halflife": hl, "daughters": daughters, "missing_pe": missing}

def format_halflife_ru(hl_seconds):
    if hl_seconds is None:
        return "—"
    units = [("лет", 365.25*86400.0), ("сут", 86400.0), ("ч", 3600.0), ("мин", 60.0), ("с", 1.0)]
    for name, k in units:
        if hl_seconds >= k:
            return f"{hl_seconds/k:.3f} {name} ({hl_seconds:.1f} с)"
    return f"{hl_seconds:.3f} с"

def compute_decays_from_activity(a_bq, t_s, hl_s):
    lam = math.log(2.0) / hl_s
    x = lam * t_s
    if x <= 0:
        return 0
    n = (a_bq / lam) * (-math.expm1(-x))
    if n < 0:
        return 0
    return int(n + 0.5)

# ============================================================
# MAIN APPLICATION
# ============================================================

class ProgramB(tk.Tk):
    def __init__(self):
        super().__init__()
        self.title("Программа Б — Генератор макросов")
        self.geometry("1120x800")
        self.minsize(960, 680)

        self.output_checkvars = {}
        self.rdm_threshold_var = tk.BooleanVar(value=True)

        self._apply_visual_style()
        self._build_ui()
        self._populate_materials()
        self._populate_radionuclides()
        self._populate_output_checks()
        self._update_geometry()
        self._update_decay_visibility()
        self._refresh_macro_preview()

    # --------------------------------------------------------
    # VISUAL STYLE (fonts, colors). Behavior is unchanged.
    # --------------------------------------------------------
    def _apply_visual_style(self):
        self.font_base = ("Segoe UI", 11)
        self.font_bold = ("Segoe UI", 11, "bold")
        self.font_mono = ("Consolas", 12)

        self.option_add("*TCombobox*Listbox.font", self.font_base)

        style = ttk.Style(self)
        style.configure("TLabel", font=self.font_base)
        style.configure("TButton", font=self.font_base, padding=(10, 5))
        style.configure("TCheckbutton", font=self.font_base)
        style.configure("TEntry", font=self.font_base)
        style.configure("TCombobox", font=self.font_base)
        style.configure("TLabelframe", font=self.font_base)
        style.configure("TLabelframe.Label", font=self.font_bold)
        style.configure("TNotebook.Tab", font=self.font_bold, padding=(10, 4))

        style.configure("Hint.TLabel", foreground="#555555")
        style.configure("Error.TLabel", foreground="#B00020")
        style.configure("Ok.TLabel", foreground="#1E7E34")
        style.configure("Warn.TLabel", foreground="#B26A00")
        style.configure("Mono.TLabel", font=self.font_mono)

    # --------------------------------------------------------
    # UI CONSTRUCTION
    # --------------------------------------------------------
    def _build_ui(self):
        self.notebook = ttk.Notebook(self)
        self.notebook.pack(fill="both", expand=True, padx=8, pady=(8,0))

        self.page_geometry = ttk.Frame(self.notebook)
        self.page_medium   = ttk.Frame(self.notebook)
        self.page_radionuclide = ttk.Frame(self.notebook)
        self.page_decay    = ttk.Frame(self.notebook)
        self.page_output   = ttk.Frame(self.notebook)
        self.page_misc     = ttk.Frame(self.notebook)

        self.notebook.add(self.page_geometry,       text="  Геометрия  ")
        self.notebook.add(self.page_medium,         text="  Среда и корпус  ")
        self.notebook.add(self.page_radionuclide,   text="  Радионуклид  ")
        self.notebook.add(self.page_decay,          text="  Распад  ")
        self.notebook.add(self.page_output,         text="  Получаемые значения  ")
        self.notebook.add(self.page_misc,           text="  Запуск и вывод  ")

        self._build_geometry_page()
        self._build_medium_page()
        self._build_radionuclide_page()
        self._build_decay_page()
        self._build_output_page()
        self._build_misc_page()

        bottom = ttk.Frame(self)
        bottom.pack(fill="x", padx=8, pady=8)
        self.gen_status = ttk.Label(bottom, text="")
        self.gen_status.pack(side="left", padx=10)

    def _row(self, parent, row, label_text, widget_factory, **grid_kw):
        lbl = ttk.Label(parent, text=label_text)
        lbl.grid(row=row, column=0, sticky="w", padx=(0,8), pady=4)
        w = widget_factory(parent)
        w.grid(row=row, column=1, sticky="ew", pady=4, **grid_kw)
        parent.columnconfigure(1, weight=1)
        return w

    # --- ГЕОМЕТРИЯ ---
    def _build_geometry_page(self):
        f = self.page_geometry
        lf = ttk.LabelFrame(f, text="Форма и размеры")
        lf.pack(fill="both", expand=True, padx=8, pady=8)

        ttk.Label(lf, text="Вариант геометрии").grid(row=0, column=0, sticky="w", padx=4, pady=4)
        self.geometry_combo = ttk.Combobox(lf, state="readonly")
        self.geometry_combo["values"] = [v["label"] for v in GEOMETRY_VARIANTS]
        self.geometry_combo.current(0)
        self.geometry_combo.grid(row=0, column=1, sticky="ew", padx=4, pady=4)
        self.geometry_combo.bind("<<ComboboxSelected>>", lambda _: (self._update_geometry(), self._refresh_macro_preview()))

        ttk.Label(lf, text="Высота").grid(row=1, column=0, sticky="w", padx=4, pady=4)
        hf = ttk.Frame(lf)
        hf.grid(row=1, column=1, sticky="ew", padx=4, pady=4)
        self.height_var = tk.StringVar(value="6.0")
        ttk.Entry(hf, textvariable=self.height_var, width=12).pack(side="left", fill="x", expand=True)
        self.height_unit = ttk.Combobox(hf, values=["мм","см","м"], state="readonly", width=5)
        self.height_unit.set("см")
        self.height_unit.pack(side="left", padx=(4,0))

        ttk.Label(lf, text="Радиус").grid(row=2, column=0, sticky="w", padx=4, pady=4)
        rf = ttk.Frame(lf)
        rf.grid(row=2, column=1, sticky="ew", padx=4, pady=4)
        self.radius_var = tk.StringVar(value="2.0")
        ttk.Entry(rf, textvariable=self.radius_var, width=12).pack(side="left", fill="x", expand=True)
        self.radius_unit = ttk.Combobox(rf, values=["мм","см","м"], state="readonly", width=5)
        self.radius_unit.set("см")
        self.radius_unit.pack(side="left", padx=(4,0))

        self.shell_frame = ttk.LabelFrame(lf, text="Стеклянная оболочка (стакан / пробирка)")
        self.shell_frame.grid(row=3, column=0, columnspan=2, sticky="ew", padx=4, pady=8)
        self.shell_frame.columnconfigure(1, weight=1)

        ttk.Label(self.shell_frame, text="Толщина стенки").grid(row=0, column=0, sticky="w", padx=4, pady=4)
        wf = ttk.Frame(self.shell_frame)
        wf.grid(row=0, column=1, sticky="ew", padx=4, pady=4)
        self.wall_var = tk.StringVar(value="1.5")
        ttk.Entry(wf, textvariable=self.wall_var, width=12).pack(side="left", fill="x", expand=True)
        self.wall_unit = ttk.Combobox(wf, values=["мм","см","м"], state="readonly", width=5)
        self.wall_unit.set("мм")
        self.wall_unit.pack(side="left", padx=(4,0))

        ttk.Label(self.shell_frame, text="Сухой бортик выше уровня").grid(row=1, column=0, sticky="w", padx=4, pady=4)
        rimf = ttk.Frame(self.shell_frame)
        rimf.grid(row=1, column=1, sticky="ew", padx=4, pady=4)
        self.rim_var = tk.StringVar(value="0")
        ttk.Entry(rimf, textvariable=self.rim_var, width=12).pack(side="left", fill="x", expand=True)
        self.rim_unit = ttk.Combobox(rimf, values=["мм","см","м"], state="readonly", width=5)
        self.rim_unit.set("мм")
        self.rim_unit.pack(side="left", padx=(4,0))

        ttk.Label(
            self.shell_frame,
            text="Бортик — сухое (пустое) продолжение цилиндрической стенки вверх от уровня жидкости.\n"
                 "Полусферическое дно пробирки всегда заполнено средой. Толщина задаётся на все стенки и дно.",
            wraplength=620, justify="left",
        ).grid(row=2, column=0, columnspan=2, sticky="w", padx=4, pady=(4, 6))

        ttk.Label(lf, text="В макрос передаётся половина высоты: /myDetector/setHalfHeight").grid(row=4, column=0, columnspan=2, sticky="w", padx=4, pady=(8,4))

        lf.columnconfigure(1, weight=1)

        lf2 = ttk.LabelFrame(f, text="Расчётные параметры")
        lf2.pack(fill="both", expand=True, padx=8, pady=8)
        ttk.Label(lf2, text="Объём среды (см³)").grid(row=0, column=0, sticky="w", padx=4, pady=4)
        self.volume_var = tk.StringVar()
        ttk.Entry(lf2, textvariable=self.volume_var, state="readonly").grid(row=0, column=1, sticky="ew", padx=4, pady=4)
        lf2.columnconfigure(1, weight=1)

        self.height_var.trace_add("write", lambda *_: (self._update_geometry(), self._refresh_macro_preview()))
        self.height_unit.bind("<<ComboboxSelected>>", lambda _: (self._update_geometry(), self._refresh_macro_preview()))
        self.radius_var.trace_add("write", lambda *_: (self._update_geometry(), self._refresh_macro_preview()))
        self.radius_unit.bind("<<ComboboxSelected>>", lambda _: (self._update_geometry(), self._refresh_macro_preview()))
        self.wall_var.trace_add("write", lambda *_: (self._update_geometry(), self._refresh_macro_preview()))
        self.wall_unit.bind("<<ComboboxSelected>>", lambda _: (self._update_geometry(), self._refresh_macro_preview()))
        self.rim_var.trace_add("write", lambda *_: (self._update_geometry(), self._refresh_macro_preview()))
        self.rim_unit.bind("<<ComboboxSelected>>", lambda _: (self._update_geometry(), self._refresh_macro_preview()))

    # --- СРЕДА ---
    def _build_medium_page(self):
        f = self.page_medium
        f.columnconfigure(0, weight=1)
        lf = ttk.LabelFrame(f, text="Материал среды")
        lf.grid(row=0, column=0, sticky="nsew", padx=8, pady=8)

        ttk.Label(lf, text="Выбор материала").grid(row=0, column=0, sticky="w", padx=4, pady=4)
        self.material_var = tk.StringVar()
        self.material_combo = ttk.Combobox(lf, textvariable=self.material_var, state="readonly")
        self.material_combo.grid(row=0, column=1, sticky="ew", padx=4, pady=4)
        self.material_combo.bind("<<ComboboxSelected>>", lambda _: (self._update_material_info(), self._refresh_macro_preview()))

        ttk.Label(lf, text="NIST-имя").grid(row=1, column=0, sticky="w", padx=4, pady=4)
        self.material_name_var = tk.StringVar()
        ttk.Entry(lf, textvariable=self.material_name_var, state="readonly").grid(row=1, column=1, sticky="ew", padx=4, pady=4)

        ttk.Label(lf, text="Плотность").grid(row=2, column=0, sticky="w", padx=4, pady=4)
        self.material_density_var = tk.StringVar()
        ttk.Entry(lf, textvariable=self.material_density_var, state="readonly").grid(row=2, column=1, sticky="ew", padx=4, pady=4)

        self.material_desc_var = tk.StringVar()
        ttk.Label(lf, textvariable=self.material_desc_var, wraplength=500, justify="left").grid(row=3, column=0, columnspan=2, sticky="w", padx=4, pady=8)
        lf.columnconfigure(1, weight=1)

        self.custom_mat_frame = ttk.LabelFrame(f, text="Пользовательский материал")
        self.custom_mat_frame.grid(row=1, column=0, sticky="ew", padx=8, pady=8)
        self.custom_mat_frame.columnconfigure(1, weight=1)

        ttk.Label(self.custom_mat_frame, text="NIST-имя материала").grid(row=0, column=0, sticky="w", padx=4, pady=4)
        self.custom_mat_name = tk.StringVar(value="G4_WATER")
        ttk.Entry(self.custom_mat_frame, textvariable=self.custom_mat_name).grid(row=0, column=1, sticky="ew", padx=4, pady=4)

        ttk.Label(self.custom_mat_frame, text="Плотность (г/см³)").grid(row=1, column=0, sticky="w", padx=4, pady=4)
        self.custom_mat_density = tk.StringVar(value="1.0")
        ttk.Entry(self.custom_mat_frame, textvariable=self.custom_mat_density).grid(row=1, column=1, sticky="ew", padx=4, pady=4)

        ttk.Label(self.custom_mat_frame, text="Укажите любое NIST-имя и переопределите плотность.\nВ макрос добавятся /myDetector/setMaterial и /myDetector/setMaterialDensity", wraplength=500, justify="left").grid(row=2, column=0, columnspan=2, sticky="w", padx=4, pady=8)

        self.custom_mat_name.trace_add("write", lambda *_: self._refresh_macro_preview())
        self.custom_mat_density.trace_add("write", lambda *_: self._refresh_macro_preview())

        # --- Материал оболочки (стекло), виден только для стакана/пробирки ---
        self.glass_frame = ttk.LabelFrame(f, text="Материал оболочки (стекло)")
        self.glass_frame.grid(row=2, column=0, sticky="ew", padx=8, pady=8)
        self.glass_frame.columnconfigure(1, weight=1)

        ttk.Label(self.glass_frame, text="Выбор стекла").grid(row=0, column=0, sticky="w", padx=4, pady=4)
        self.glass_material_var = tk.StringVar()
        self.glass_material_combo = ttk.Combobox(self.glass_frame, textvariable=self.glass_material_var, state="readonly")
        self.glass_material_combo["values"] = ["— Custom —"] + [g["label"] for g in GLASS_MATERIALS]
        self.glass_material_combo.grid(row=0, column=1, sticky="ew", padx=4, pady=4)
        self.glass_material_combo.bind("<<ComboboxSelected>>", lambda _: (self._update_glass_material_info(), self._refresh_macro_preview()))

        ttk.Label(self.glass_frame, text="NIST-имя").grid(row=1, column=0, sticky="w", padx=4, pady=4)
        self.glass_name_var = tk.StringVar()
        ttk.Entry(self.glass_frame, textvariable=self.glass_name_var, state="readonly").grid(row=1, column=1, sticky="ew", padx=4, pady=4)

        ttk.Label(self.glass_frame, text="Плотность").grid(row=2, column=0, sticky="w", padx=4, pady=4)
        self.glass_density_var = tk.StringVar()
        ttk.Entry(self.glass_frame, textvariable=self.glass_density_var, state="readonly").grid(row=2, column=1, sticky="ew", padx=4, pady=4)

        self.glass_desc_var = tk.StringVar()
        ttk.Label(self.glass_frame, textvariable=self.glass_desc_var, wraplength=500, justify="left").grid(row=3, column=0, columnspan=2, sticky="w", padx=4, pady=4)

        self.custom_glass_frame = ttk.LabelFrame(self.glass_frame, text="Пользовательское стекло")
        self.custom_glass_frame.grid(row=4, column=0, columnspan=2, sticky="ew", padx=4, pady=4)
        self.custom_glass_frame.columnconfigure(1, weight=1)

        ttk.Label(self.custom_glass_frame, text="NIST-имя материала").grid(row=0, column=0, sticky="w", padx=4, pady=4)
        self.custom_glass_name = tk.StringVar(value="G4_SILICON_DIOXIDE")
        ttk.Entry(self.custom_glass_frame, textvariable=self.custom_glass_name).grid(row=0, column=1, sticky="ew", padx=4, pady=4)

        ttk.Label(self.custom_glass_frame, text="Плотность (г/см³)").grid(row=1, column=0, sticky="w", padx=4, pady=4)
        self.custom_glass_density = tk.StringVar(value="2.32")
        ttk.Entry(self.custom_glass_frame, textvariable=self.custom_glass_density).grid(row=1, column=1, sticky="ew", padx=4, pady=4)

        ttk.Label(self.custom_glass_frame, text="В макрос добавятся /myDetector/setGlassMaterial и /myDetector/setGlassMaterialDensity", wraplength=500, justify="left").grid(row=2, column=0, columnspan=2, sticky="w", padx=4, pady=8)

        self.custom_glass_name.trace_add("write", lambda *_: self._refresh_macro_preview())
        self.custom_glass_density.trace_add("write", lambda *_: self._refresh_macro_preview())

    def _update_glass_material_info(self):
        idx = self.glass_material_combo.current()
        if idx <= 0:
            self.custom_glass_frame.grid()
            self.glass_name_var.set("custom")
            self.glass_density_var.set("")
            self.glass_desc_var.set("Укажите NIST-имя и плотность стекла вручную.")
            return
        self.custom_glass_frame.grid_remove()
        g = GLASS_MATERIALS[idx - 1]
        self.glass_name_var.set(g["name"])
        self.glass_density_var.set(g["density"])
        self.glass_desc_var.set(g["desc"])

    # --- РАДИОНУКЛИД ---
    def _build_radionuclide_page(self):
        f = self.page_radionuclide
        f.columnconfigure(0, weight=1)
        lf = ttk.LabelFrame(f, text="Источник")
        lf.grid(row=0, column=0, sticky="nsew", padx=8, pady=8)

        ttk.Label(lf, text="Выбор радионуклида").grid(row=0, column=0, sticky="w", padx=4, pady=4)
        self.rn_var = tk.StringVar()
        self.rn_combo = ttk.Combobox(lf, textvariable=self.rn_var, state="readonly")
        self.rn_combo.grid(row=0, column=1, sticky="ew", padx=4, pady=4)
        self.rn_combo.bind("<<ComboboxSelected>>", lambda _: (self._update_radionuclide_info(), self._refresh_macro_preview()))

        ttk.Label(lf, text="Z (атомный номер)").grid(row=1, column=0, sticky="w", padx=4, pady=4)
        self.rn_z_var = tk.StringVar()
        ttk.Entry(lf, textvariable=self.rn_z_var, state="readonly").grid(row=1, column=1, sticky="ew", padx=4, pady=4)

        ttk.Label(lf, text="A (массовое число)").grid(row=2, column=0, sticky="w", padx=4, pady=4)
        self.rn_a_var = tk.StringVar()
        ttk.Entry(lf, textvariable=self.rn_a_var, state="readonly").grid(row=2, column=1, sticky="ew", padx=4, pady=4)

        ttk.Label(lf, text="Имя в Geant4").grid(row=3, column=0, sticky="w", padx=4, pady=4)
        self.rn_name_var = tk.StringVar()
        ttk.Entry(lf, textvariable=self.rn_name_var, state="readonly").grid(row=3, column=1, sticky="ew", padx=4, pady=4)
        lf.columnconfigure(1, weight=1)

        self.custom_rn_frame = ttk.LabelFrame(f, text="Пользовательский радионуклид")
        self.custom_rn_frame.grid(row=1, column=0, sticky="ew", padx=8, pady=8)
        self.custom_rn_frame.columnconfigure(1, weight=1)

        ttk.Label(self.custom_rn_frame, text="Z (атомный номер)").grid(row=0, column=0, sticky="w", padx=4, pady=4)
        self.custom_rn_z = tk.StringVar(value="27")
        ttk.Entry(self.custom_rn_frame, textvariable=self.custom_rn_z).grid(row=0, column=1, sticky="ew", padx=4, pady=4)

        ttk.Label(self.custom_rn_frame, text="A (массовое число)").grid(row=1, column=0, sticky="w", padx=4, pady=4)
        self.custom_rn_a = tk.StringVar(value="60")
        ttk.Entry(self.custom_rn_frame, textvariable=self.custom_rn_a).grid(row=1, column=1, sticky="ew", padx=4, pady=4)

        ttk.Button(self.custom_rn_frame, text="Проверить", command=self._check_custom_nuclide).grid(row=2, column=0, columnspan=2, sticky="ew", padx=4, pady=4)
        ttk.Label(self.custom_rn_frame, text="Проверка выполняется по файлам Geant4 на этом компьютере.", wraplength=500, justify="left").grid(row=3, column=0, columnspan=2, sticky="w", padx=4, pady=(0,8))

        self.custom_rn_z.trace_add("write", lambda *_: self._on_custom_rn_typed())
        self.custom_rn_a.trace_add("write", lambda *_: self._on_custom_rn_typed())

        self.rn_desc_frame = ttk.LabelFrame(f, text="Описание")
        self.rn_desc_frame.grid(row=2, column=0, sticky="ew", padx=8, pady=8)
        self.rn_desc_var = tk.StringVar()
        ttk.Label(self.rn_desc_frame, textvariable=self.rn_desc_var, wraplength=500, justify="left").pack(fill="x", padx=4, pady=4)
        ttk.Label(self.rn_desc_frame, text="В макрос передаётся: /mySource/setRadionuclide Z A").pack(fill="x", padx=4, pady=(4,8))

        self.rn_data_frame = ttk.LabelFrame(f, text="Данные Geant4")
        self.rn_data_frame.grid(row=3, column=0, sticky="ew", padx=8, pady=8)
        self.rn_halflife_var = tk.StringVar(value="T1/2: —")
        ttk.Label(self.rn_data_frame, textvariable=self.rn_halflife_var, wraplength=600, justify="left", style="Mono.TLabel").pack(fill="x", padx=4, pady=(4,2))
        self.rn_status_var = tk.StringVar(value="")
        self.rn_status_label = ttk.Label(self.rn_data_frame, textvariable=self.rn_status_var, wraplength=600, justify="left")
        self.rn_status_label.pack(fill="x", padx=4, pady=(2,4))

    # --- РАСПАД ---
    def _build_decay_page(self):
        f = self.page_decay
        f.columnconfigure(0, weight=1)

        mode_frame = ttk.LabelFrame(f, text="Режим задания")
        mode_frame.grid(row=0, column=0, sticky="ew", padx=8, pady=8)
        mode_frame.columnconfigure(1, weight=1)
        ttk.Label(mode_frame, text="Режим").grid(row=0, column=0, sticky="w", padx=4, pady=4)
        self.decay_mode_var = tk.StringVar(value=DECAY_MODES[0])
        self.decay_mode_combo = ttk.Combobox(mode_frame, textvariable=self.decay_mode_var,
                                             values=DECAY_MODES, state="readonly")
        self.decay_mode_combo.grid(row=0, column=1, sticky="ew", padx=4, pady=4)
        self.decay_mode_combo.bind("<<ComboboxSelected>>", lambda _: self._on_decay_mode_selected())

        self.count_frame = ttk.LabelFrame(f, text="Количество распадов")
        self.count_frame.grid(row=1, column=0, sticky="ew", padx=8, pady=8)
        ttk.Label(self.count_frame, text="Количество распадов").grid(row=0, column=0, sticky="w", padx=4, pady=4)
        self.decay_count_var = tk.StringVar(value="1000000")
        ttk.Entry(self.count_frame, textvariable=self.decay_count_var).grid(row=0, column=1, sticky="ew", padx=4, pady=4)
        ttk.Label(self.count_frame, text="Программа выполнит ровно N событий: /run/beamOn N").grid(row=1, column=0, columnspan=2, sticky="w", padx=4, pady=4)
        self.count_warn_var = tk.StringVar(value="")
        ttk.Label(self.count_frame, textvariable=self.count_warn_var, foreground="#B00020", wraplength=600, justify="left").grid(row=2, column=0, columnspan=2, sticky="w", padx=4, pady=4)
        self.count_frame.columnconfigure(1, weight=1)

        self.act_frame = ttk.LabelFrame(f, text="По активности и времени")
        self.act_frame.grid(row=2, column=0, sticky="ew", padx=8, pady=8)
        self.act_frame.columnconfigure(1, weight=1)

        ttk.Label(self.act_frame, text="Активность").grid(row=0, column=0, sticky="w", padx=4, pady=4)
        af = ttk.Frame(self.act_frame)
        af.grid(row=0, column=1, sticky="ew", padx=4, pady=4)
        af.columnconfigure(0, weight=1)
        self.activity_var = tk.StringVar(value="1")
        ttk.Entry(af, textvariable=self.activity_var).grid(row=0, column=0, sticky="ew")
        self.activity_unit_var = tk.StringVar(value="МБк")
        self.activity_unit_combo = ttk.Combobox(af, textvariable=self.activity_unit_var,
                                               values=list(ACTIVITY_UNITS.keys()),
                                               state="readonly", width=7)
        self.activity_unit_combo.grid(row=0, column=1, padx=(4, 0))

        ttk.Label(self.act_frame, text="Время облучения").grid(row=1, column=0, sticky="w", padx=4, pady=4)
        tf = ttk.Frame(self.act_frame)
        tf.grid(row=1, column=1, sticky="ew", padx=4, pady=4)
        tf.columnconfigure(0, weight=1)
        self.irrtime_var = tk.StringVar(value="1")
        ttk.Entry(tf, textvariable=self.irrtime_var).grid(row=0, column=0, sticky="ew")
        self.irrtime_unit_var = tk.StringVar(value="ч")
        self.irrtime_unit_combo = ttk.Combobox(tf, textvariable=self.irrtime_unit_var,
                                              values=list(TIME_UNITS.keys()),
                                              state="readonly", width=7)
        self.irrtime_unit_combo.grid(row=0, column=1, padx=(4, 0))

        self.decay_computed_var = tk.StringVar(value="N = —")
        ttk.Label(self.act_frame, textvariable=self.decay_computed_var, wraplength=600, justify="left", style="Mono.TLabel").grid(row=2, column=0, columnspan=2, sticky="w", padx=4, pady=4)
        self.act_halflife_var = tk.StringVar(value="T1/2: —")
        ttk.Label(self.act_frame, textvariable=self.act_halflife_var, wraplength=600, justify="left", style="Mono.TLabel").grid(row=3, column=0, columnspan=2, sticky="w", padx=4, pady=4)
        self.act_warn_var = tk.StringVar(value="")
        ttk.Label(self.act_frame, textvariable=self.act_warn_var, foreground="#B00020", wraplength=600, justify="left").grid(row=4, column=0, columnspan=2, sticky="w", padx=4, pady=4)
        ttk.Label(self.act_frame, text="Число событий N вычисляется с учетом распада источника за время облучения.", wraplength=600, justify="left").grid(row=5, column=0, columnspan=2, sticky="w", padx=4, pady=4)

        self.decay_count_var.trace_add("write", lambda *_: (self._update_decay_warnings(), self._refresh_macro_preview()))
        self.activity_var.trace_add("write", lambda *_: (self._update_decay_warnings(), self._refresh_macro_preview()))
        self.irrtime_var.trace_add("write", lambda *_: (self._update_decay_warnings(), self._refresh_macro_preview()))
        self.activity_unit_combo.bind("<<ComboboxSelected>>", lambda _: (self._update_decay_warnings(), self._refresh_macro_preview()))
        self.irrtime_unit_combo.bind("<<ComboboxSelected>>", lambda _: (self._update_decay_warnings(), self._refresh_macro_preview()))

    # --- ПОЛУЧАЕМЫЕ ЗНАЧЕНИЯ ---
    def _build_output_page(self):
        f = self.page_output

        lf_threads = ttk.LabelFrame(f, text="Параллелизм симуляции")
        lf_threads.pack(fill="x", padx=8, pady=8)

        ttk.Label(lf_threads, text="Количество потоков").grid(row=0, column=0, sticky="w", padx=4, pady=4)
        self.num_threads_var = tk.StringVar(value="0")
        ttk.Entry(lf_threads, textvariable=self.num_threads_var, width=10).grid(row=0, column=1, sticky="w", padx=4, pady=4)
        ttk.Button(lf_threads, text="Макс.", command=self._set_max_threads).grid(row=0, column=2, padx=4, pady=4)
        ttk.Label(lf_threads, text="0 = использовать максимальное число потоков. В макрос добавляется /run/numberOfThreads N").grid(row=1, column=0, columnspan=3, sticky="w", padx=4, pady=4)
        self.num_threads_var.trace_add("write", lambda *_: self._refresh_macro_preview())

        lf_out = ttk.LabelFrame(f, text="Поля для вывода в results.txt и консоль")
        lf_out.pack(fill="both", expand=True, padx=8, pady=8)

        btn_frame = ttk.Frame(lf_out)
        btn_frame.pack(fill="x", padx=4, pady=4)
        ttk.Button(btn_frame, text="Отметить все", command=lambda: self._check_all(True)).pack(side="left", padx=2)
        ttk.Button(btn_frame, text="Снять все", command=lambda: self._check_all(False)).pack(side="left", padx=2)
        ttk.Button(btn_frame, text="По умолчанию", command=self._check_defaults).pack(side="left", padx=2)

        self.checks_canvas = tk.Canvas(lf_out)
        scrollbar = ttk.Scrollbar(lf_out, orient="vertical", command=self.checks_canvas.yview)
        self.checks_inner = ttk.Frame(self.checks_canvas)
        self.checks_inner.bind("<Configure>", lambda e: self.checks_canvas.configure(scrollregion=self.checks_canvas.bbox("all")))
        self.checks_canvas.create_window((0,0), window=self.checks_inner, anchor="nw")
        self.checks_canvas.configure(yscrollcommand=scrollbar.set)
        self.checks_canvas.pack(side="left", fill="both", expand=True)
        scrollbar.pack(side="right", fill="y")
        self.checks_canvas.bind("<Enter>", lambda _: self._wheel_on())
        self.checks_canvas.bind("<Leave>", lambda _: self._wheel_off())

        ttk.Label(lf_out, text="Снимите галочку с поля, чтобы оно не попало в вывод.\nВ макрос добавляются команды /myOutput/disable <id> для снятых полей.", wraplength=600, justify="left").pack(fill="x", padx=4, pady=8)

    def _wheel_on(self):
        self.checks_canvas.bind_all("<MouseWheel>", self._scroll_checks)
        self.checks_canvas.bind_all("<Button-4>", self._scroll_checks)
        self.checks_canvas.bind_all("<Button-5>", self._scroll_checks)

    def _wheel_off(self):
        self.checks_canvas.unbind_all("<MouseWheel>")
        self.checks_canvas.unbind_all("<Button-4>")
        self.checks_canvas.unbind_all("<Button-5>")

    def _scroll_checks(self, event):
        if getattr(event, "num", 0) == 4:
            self.checks_canvas.yview_scroll(-1, "units")
        elif getattr(event, "num", 0) == 5:
            self.checks_canvas.yview_scroll(1, "units")
        else:
            delta = getattr(event, "delta", 0)
            self.checks_canvas.yview_scroll(-1 * int(delta / 120), "units")
        return "break"

    # --- ЗАПУСК И ВЫВОД ---
    def _build_misc_page(self):
        f = self.page_misc

        lf_macro = ttk.LabelFrame(f, text="Макрос (обновляется автоматически)")
        lf_macro.pack(fill="both", expand=True, padx=8, pady=8)

        btn_frame = ttk.Frame(lf_macro)
        btn_frame.pack(fill="x", padx=4, pady=(4, 2))
        self.run_button = ttk.Button(btn_frame, text="Запустить симуляцию", command=self._run_simulation)
        self.run_button.pack(side="left", padx=2)
        self.abort_button = ttk.Button(btn_frame, text="Прервать симуляцию", command=self._abort_simulation, state="disabled")
        self.abort_button.pack(side="left", padx=2)
        ttk.Button(btn_frame, text="Сохранить .mac", command=self._save_macro_dialog).pack(side="left", padx=2)

        progress_frame = ttk.Frame(lf_macro)
        progress_frame.pack(fill="x", padx=4, pady=(0, 4))
        self.progress_bar = ttk.Progressbar(progress_frame, orient="horizontal", mode="determinate", maximum=100.0)
        self.progress_bar.pack(fill="x")
        self.progress_label = ttk.Label(progress_frame, text="Прогресс появится после запуска симуляции.", foreground="#555555")
        self.progress_label.pack(anchor="w", pady=(2, 0))

        self.macro_text = scrolledtext.ScrolledText(lf_macro, height=8, font=self.font_mono)
        self.macro_text.pack(fill="both", expand=True, padx=4, pady=4)
        ttk.Label(
            lf_macro,
            text="Порог времени распада снимается автоматически (/process/had/rdm/thresholdForVeryLongDecayTime 1e+60 year).\nЗапуск: main.exe run.mac в папке программы.",
            foreground="#555555",
            justify="left",
        ).pack(anchor="w", padx=4, pady=(0, 4))

        lf_result = ttk.LabelFrame(f, text="Результат симуляции")
        lf_result.pack(fill="both", expand=True, padx=8, pady=8)
        btn_frame2 = ttk.Frame(lf_result)
        btn_frame2.pack(fill="x", padx=4, pady=4)
        ttk.Button(btn_frame2, text="Прочитать results.txt", command=self._read_result).pack(side="left", padx=2)
        ttk.Button(btn_frame2, text="Выбрать results.txt", command=self._load_result_file).pack(side="left", padx=2)
        self.result_text = scrolledtext.ScrolledText(lf_result, height=14, font=self.font_mono)
        self.result_text.pack(fill="both", expand=True, padx=4, pady=4)

    # --------------------------------------------------------
    # LOGIC
    # --------------------------------------------------------

    def _populate_materials(self):
        labels = [m["label"] for m in MATERIALS]
        self.material_combo["values"] = ["— Custom —"] + labels
        self.material_combo.current(1)
        self._update_material_info()
        if hasattr(self, "glass_material_combo"):
            self.glass_material_combo.current(1)
            self._update_glass_material_info()

    def _populate_radionuclides(self):
        labels = [f'{r["name"]} (Z={r["Z"]}, A={r["A"]})' for r in RADIONUCLIDES]
        self.rn_combo["values"] = ["— Custom —"] + labels
        self.rn_combo.current(1)
        self._update_radionuclide_info()

    def _populate_output_checks(self):
        self.output_check_widgets = {}
        for w in self.checks_inner.winfo_children():
            w.destroy()
        col = 0
        row = 0
        for gk, gtitle in GROUP_TITLES.items():
            group_frame = ttk.LabelFrame(self.checks_inner, text=gtitle)
            group_frame.grid(row=row, column=0, columnspan=2, sticky="ew", padx=4, pady=6, ipadx=4, ipady=4)
            fields = [f for f in OUTPUT_FIELDS if f["group"] == gk]
            inner = ttk.Frame(group_frame)
            inner.pack(fill="x", padx=4, pady=4)
            for i, f in enumerate(fields):
                var = tk.BooleanVar(value=f["def"])
                self.output_checkvars[f["id"]] = var
                cb = ttk.Checkbutton(inner, text=f["label"], variable=var,
                                command=self._refresh_macro_preview)
                cb.grid(row=i//2, column=i%2, sticky="w", padx=4, pady=2)
                self.output_check_widgets[f["id"]] = cb
            row += 1
        self.checks_inner.columnconfigure(0, weight=1)
        self._update_output_visibility()

    def _check_all(self, state):
        for var in self.output_checkvars.values():
            var.set(state)
        self._refresh_macro_preview()

    def _check_defaults(self):
        for f in OUTPUT_FIELDS:
            if f["id"] in self.output_checkvars:
                self.output_checkvars[f["id"]].set(f["def"])
        self._refresh_macro_preview()

    def _geometry_token(self):
        idx = self.geometry_combo.current()
        if idx < 0 or idx >= len(GEOMETRY_VARIANTS):
            idx = 0
        return GEOMETRY_VARIANTS[idx]["token"]

    def _geometry_label(self):
        idx = self.geometry_combo.current()
        if idx < 0 or idx >= len(GEOMETRY_VARIANTS):
            idx = 0
        return GEOMETRY_VARIANTS[idx]["label"]

    def _show_shell(self):
        return self._geometry_token() != "cylinder"

    def _update_geometry(self):
        r = to_cm(self.radius_var.get(), self.radius_unit.get())
        h = to_cm(self.height_var.get(), self.height_unit.get())
        v = None
        if r is not None and h is not None and r > 0 and h > 0:
            v = math.pi * r * r * h
            if self._geometry_token() == "tube":
                v += (2.0 / 3.0) * math.pi * r ** 3
        self.volume_var.set(f"{v:.4f}" if v is not None else "")
        shell = self._show_shell()
        if hasattr(self, "shell_frame"):
            if shell:
                self.shell_frame.grid()
            else:
                self.shell_frame.grid_remove()
        if hasattr(self, "glass_frame"):
            if shell:
                self.glass_frame.grid()
            else:
                self.glass_frame.grid_remove()

    def _update_material_info(self):
        idx = self.material_combo.current()
        if idx <= 0:
            self.custom_mat_frame.grid()
            self.material_name_var.set("custom")
            self.material_density_var.set("")
            self.material_desc_var.set("Укажите NIST-имя и плотность вручную.")
            return
        self.custom_mat_frame.grid_remove()
        m = MATERIALS[idx - 1]
        self.material_name_var.set(m["name"])
        self.material_density_var.set(m["density"])
        self.material_desc_var.set(m["desc"])

    def _update_radionuclide_info(self):
        idx = self.rn_combo.current()
        if idx <= 0:
            self.custom_rn_frame.grid()
            self.rn_desc_frame.grid_remove()
            self.rn_z_var.set("")
            self.rn_a_var.set("")
            self.rn_name_var.set("custom")
            self._set_nuclide_status(None)
            return
        self.custom_rn_frame.grid_remove()
        self.rn_desc_frame.grid()
        r = RADIONUCLIDES[idx - 1]
        self.rn_z_var.set(str(r["Z"]))
        self.rn_a_var.set(str(r["A"]))
        self.rn_name_var.set(r["name"])
        self.rn_desc_var.set(r["desc"])
        self._set_nuclide_status(check_nuclide(r["Z"], r["A"], r.get("exc", 0.0) or 0.0))
        self._update_decay_warnings()

    def _on_custom_rn_typed(self):
        self._set_nuclide_status(None, typed=True)
        self._update_decay_warnings()
        self._refresh_macro_preview()

    def _check_custom_nuclide(self):
        try:
            z = int(self.custom_rn_z.get())
            a = int(self.custom_rn_a.get())
        except ValueError:
            self._set_nuclide_status({"state": "bad_input"})
            self._update_decay_warnings()
            self._refresh_macro_preview()
            return
        self._set_nuclide_status(check_nuclide(z, a, 0.0))
        self._update_decay_warnings()
        self._refresh_macro_preview()

    def _current_nuclide(self):
        idx = self.rn_combo.current() if hasattr(self, "rn_combo") else 1
        if idx <= 0:
            try:
                z = int(self.custom_rn_z.get())
                a = int(self.custom_rn_a.get())
            except ValueError:
                return None
            if z < 1 or z > 119 or a < 1:
                return None
            return (z, a, 0.0)
        r = RADIONUCLIDES[idx - 1]
        return (r["Z"], r["A"], r.get("exc", 0.0) or 0.0)

    def _set_nuclide_status(self, res, typed=False):
        if not hasattr(self, "rn_status_var"):
            return
        if res is None:
            if typed:
                self.rn_halflife_var.set("T1/2: —")
                self.rn_status_var.set("Измените Z/A и нажмите Проверить.")
            else:
                self.rn_halflife_var.set("T1/2: —")
                self.rn_status_var.set("")
            self.rn_status_label.configure(style="TLabel")
            return
        st = res.get("state")
        if st == "no_base":
            self.rn_halflife_var.set("T1/2: —")
            self.rn_status_var.set("База Geant4 недоступна (нет G4RADIOACTIVEDATA).")
            self.rn_status_label.configure(style="Warn.TLabel")
        elif st == "bad_input":
            self.rn_halflife_var.set("T1/2: —")
            self.rn_status_var.set("Z должен быть 1..119, A положительным целым.")
            self.rn_status_label.configure(style="Error.TLabel")
        elif st == "no_nuclide":
            self.rn_halflife_var.set("T1/2: —")
            self.rn_status_var.set("Данные по радионуклиду не найдены — симуляция недоступна")
            self.rn_status_label.configure(style="Error.TLabel")
        else:
            self.rn_halflife_var.set("T1/2: " + format_halflife_ru(res.get("halflife")))
            miss = res.get("missing_pe") or []
            if miss:
                self.rn_status_var.set(f"Схема распада есть. Уровни дочерних отсутствуют: {miss}")
            else:
                self.rn_status_var.set("Схема распада есть. Уровни дочерних есть.")
            self.rn_status_label.configure(style="Ok.TLabel")

    def _decay_mode(self):
        try:
            return self.decay_mode_var.get()
        except AttributeError:
            return DECAY_MODES[0]

    def _on_decay_mode_selected(self):
        mode = self._decay_mode()
        if mode == DECAY_MODES[1]:
            if "activity" in self.output_checkvars:
                self.output_checkvars["activity"].set(True)
            if "irr_time" in self.output_checkvars:
                self.output_checkvars["irr_time"].set(True)
            if "num_events" in self.output_checkvars:
                self.output_checkvars["num_events"].set(False)
        else:
            if "num_events" in self.output_checkvars:
                self.output_checkvars["num_events"].set(True)
        self._update_decay_visibility()
        self._refresh_macro_preview()

    def _update_output_visibility(self):
        if not hasattr(self, "output_check_widgets"):
            return
        show_source_fields = (self._decay_mode() == DECAY_MODES[1])
        for fid in ("activity", "irr_time"):
            cb = self.output_check_widgets.get(fid)
            if cb is None:
                continue
            if show_source_fields:
                cb.grid()
            else:
                cb.grid_remove()

    def _update_decay_visibility(self):
        self._update_output_visibility()
        mode = self._decay_mode()
        is_count = (mode == DECAY_MODES[0])
        if hasattr(self, "count_frame"):
            if is_count:
                self.count_frame.grid()
            else:
                self.count_frame.grid_remove()
        if hasattr(self, "act_frame"):
            if is_count:
                self.act_frame.grid_remove()
            else:
                self.act_frame.grid()
        self._update_decay_warnings()

    def _parse_activity_time(self):
        try:
            a_val = float(self.activity_var.get())
        except (ValueError, TypeError):
            return (None, None, "Укажите положительную активность.")
        try:
            t_val = float(self.irrtime_var.get())
        except (ValueError, TypeError):
            return (None, None, "Укажите положительное время облучения.")
        if a_val <= 0:
            return (None, None, "Укажите положительную активность.")
        if t_val <= 0:
            return (None, None, "Укажите положительное время облучения.")
        a_bq = a_val * ACTIVITY_UNITS.get(self.activity_unit_var.get(), 1.0)
        t_s = t_val * TIME_UNITS.get(self.irrtime_unit_var.get(), 1.0)
        return (a_bq, t_s, None)

    def _effective_decays(self):
        """Returns (N_int, info_dict). Raises ValueError on bad input."""
        if self._decay_mode() == DECAY_MODES[0]:
            s = format_number(self.decay_count_var.get())
            if s is None:
                raise ValueError("Укажите целое число распадов не меньше 1.")
            try:
                n = int(float(s))
            except ValueError:
                raise ValueError("Укажите целое число распадов не меньше 1.")
            if n < 1:
                raise ValueError("Укажите целое число распадов не меньше 1.")
            if n > NMAX_EVENTS:
                raise ValueError(f"Число распадов {n} превышает лимит {NMAX_EVENTS}. Уменьшите число распадов.")
            return (n, {"mode": "count"})
        nuc = self._current_nuclide()
        if nuc is None:
            raise ValueError("Z должен быть целым числом от 1 до 119, A положительным.")
        z, a, exc = nuc
        state, hl = parse_halflife_seconds(z, a, exc)
        if state == "no_base":
            raise ValueError("База Geant4 недоступна (нет G4RADIOACTIVEDATA).")
        if state != "ok" or not hl or hl <= 0:
            raise ValueError("Данные по радионуклиду не найдены — симуляция недоступна")
        a_bq, t_s, err = self._parse_activity_time()
        if err:
            raise ValueError(err)
        n = compute_decays_from_activity(a_bq, t_s, hl)
        if n < 1:
            raise ValueError("При таких активности и времени число распадов меньше 1. Увеличьте активность или время.")
        if n > NMAX_EVENTS:
            raise ValueError(f"Число распадов {n} превышает лимит {NMAX_EVENTS}. Уменьшите активность или время.")
        return (n, {"mode": "activity", "a_bq": a_bq, "t_s": t_s, "hl": hl})

    def _update_decay_warnings(self):
        if not hasattr(self, "count_warn_var"):
            return
        try:
            s = format_number(self.decay_count_var.get())
            n = int(float(s)) if s is not None else 0
        except (ValueError, TypeError):
            n = 0
        if n > NMAX_EVENTS:
            self.count_warn_var.set(f"Число распадов {n} превышает лимит {NMAX_EVENTS}. Уменьшите число распадов.")
        else:
            self.count_warn_var.set("")
        if not hasattr(self, "act_warn_var"):
            return
        raw = self._raw_activity_n()
        if raw is None:
            n2info = None
            try:
                n2, info = self._effective_decays_for_display()
                self.decay_computed_var.set(f"N = {n2}")
                self.act_halflife_var.set("T1/2: " + format_halflife_ru(info.get("hl")))
                self.act_warn_var.set("")
            except Exception as e:
                self.decay_computed_var.set("N = —")
                msg = str(e)
                if "превышает лимит" in msg:
                    self.act_warn_var.set(msg)
                else:
                    self.act_warn_var.set("")
                hl_txt = self._halflife_for_display()
                self.act_halflife_var.set("T1/2: " + hl_txt)
            return
        n_raw, hl_raw, err_raw = raw
        if err_raw is not None:
            self.decay_computed_var.set("N = —")
            self.act_warn_var.set("")
            self.act_halflife_var.set("T1/2: " + self._halflife_for_display())
            return
        self.decay_computed_var.set(f"N = {n_raw}")
        self.act_halflife_var.set("T1/2: " + format_halflife_ru(hl_raw))
        if n_raw > NMAX_EVENTS:
            self.act_warn_var.set(f"Число распадов {n_raw} превышает лимит {NMAX_EVENTS}. Уменьшите активность или время.")
        elif n_raw < 1:
            self.act_warn_var.set("")
            self.decay_computed_var.set("N = — (меньше 1: увеличьте активность или время)")
        else:
            self.act_warn_var.set("")

    def _raw_activity_n(self):
        if self._decay_mode() == DECAY_MODES[0]:
            return None
        nuc = self._current_nuclide()
        if nuc is None:
            return None
        z, a, exc = nuc
        state, hl = parse_halflife_seconds(z, a, exc)
        if state != "ok" or not hl or hl <= 0:
            return None
        a_bq, t_s, err = self._parse_activity_time()
        if err:
            return (None, hl, err)
        n = compute_decays_from_activity(a_bq, t_s, hl)
        return (n, hl, None)

    def _effective_decays_for_display(self):
        if self._decay_mode() == DECAY_MODES[0]:
            raise ValueError("count mode")
        nuc = self._current_nuclide()
        if nuc is None:
            raise ValueError("bad nuclide")
        z, a, exc = nuc
        state, hl = parse_halflife_seconds(z, a, exc)
        if state == "no_base":
            raise ValueError("База Geant4 недоступна (нет G4RADIOACTIVEDATA).")
        if state != "ok" or not hl or hl <= 0:
            raise ValueError("Данные по радионуклиду не найдены — симуляция недоступна")
        a_bq, t_s, err = self._parse_activity_time()
        if err:
            raise ValueError(err)
        n = compute_decays_from_activity(a_bq, t_s, hl)
        if n > NMAX_EVENTS:
            raise ValueError(f"Число распадов {n} превышает лимит {NMAX_EVENTS}. Уменьшите активность или время.")
        return (n, {"hl": hl})

    def _halflife_for_display(self):
        nuc = self._current_nuclide()
        if nuc is None:
            return "—"
        z, a, exc = nuc
        state, hl = parse_halflife_seconds(z, a, exc)
        if state == "no_base":
            return "база недоступна"
        if state != "ok" or not hl:
            return "не найден"
        return format_halflife_ru(hl)

    def _set_max_threads(self):
        import multiprocessing
        n = multiprocessing.cpu_count()
        self.num_threads_var.set(str(n))
        self.gen_status.config(text=f"Установлено макс. потоков: {n}")

    # --- MACRO BUILDING ---

    def _build_macro(self):
        radius_cm = to_cm(self.radius_var.get(), self.radius_unit.get())
        height_cm = to_cm(self.height_var.get(), self.height_unit.get())
        if radius_cm is None or radius_cm <= 0:
            raise ValueError("Укажите положительный радиус.")
        if height_cm is None or height_cm <= 0:
            raise ValueError("Укажите положительную высоту.")

        idx_rn = self.rn_combo.current()
        if idx_rn <= 0:
            try:
                rn_z = int(self.custom_rn_z.get())
            except ValueError:
                rn_z = None
            try:
                rn_a = int(self.custom_rn_a.get())
            except ValueError:
                rn_a = None
            if rn_z is None or rn_z < 1 or rn_z > 119:
                raise ValueError("Z должен быть целым числом от 1 до 119.")
            if rn_a is None or rn_a < 1:
                raise ValueError("A должен быть положительным целым числом.")
            rn_name = "Custom"
            rn_exc = None
        else:
            rn = RADIONUCLIDES[idx_rn - 1]
            rn_z, rn_a, rn_name = rn["Z"], rn["A"], rn["name"]
            rn_exc = rn.get("exc")
        if rn_name != "Custom":
            rn_label = rn_name + (f" (метастабильное, E*={rn_exc} кэВ)" if rn_exc else "")
        else:
            rn_label = "Custom"

        idx_mat = self.material_combo.current()
        mat_density_line = ""
        if idx_mat <= 0:
            mat_name = self.custom_mat_name.get().strip()
            if not mat_name:
                raise ValueError("Укажите NIST-имя пользовательского материала.")
            try:
                d = float(self.custom_mat_density.get())
                if d > 0:
                    mat_density_line = f"/myDetector/setMaterialDensity {format_number(d)} g/cm3"
            except ValueError:
                pass
        else:
            mat_name = MATERIALS[idx_mat - 1]["name"]

        token = self._geometry_token()
        shell = token != "cylinder"
        shell_lines = []
        if shell:
            wall_cm = to_cm(self.wall_var.get(), self.wall_unit.get())
            rim_cm = to_cm(self.rim_var.get(), self.rim_unit.get())
            if wall_cm is None or wall_cm <= 0:
                raise ValueError("Укажите положительную толщину стенки оболочки.")
            if rim_cm is None or rim_cm < 0:
                raise ValueError("Укажите неотрицательную высоту бортика.")
            idx_glass = self.glass_material_combo.current()
            glass_density_line = ""
            if idx_glass <= 0:
                glass_name = self.custom_glass_name.get().strip()
                if not glass_name:
                    raise ValueError("Укажите NIST-имя пользовательского стекла.")
                try:
                    gd = float(self.custom_glass_density.get())
                    if gd > 0:
                        glass_density_line = f"/myDetector/setGlassMaterialDensity {format_number(gd)} g/cm3"
                except ValueError:
                    pass
            else:
                glass_name = GLASS_MATERIALS[idx_glass - 1]["name"]
            shell_lines = [
                f"/myDetector/setGlassWall {format_number(wall_cm)} cm",
                f"/myDetector/setRim {format_number(rim_cm)} cm",
                f"/myDetector/setGlassMaterial {glass_name}",
            ]
            if glass_density_line:
                shell_lines.append(glass_density_line)

        n_events, decay_info = self._effective_decays()
        beam_on_n = str(n_events)
        activity_line = ""
        irrtime_line = ""
        decay_comment = ""
        if decay_info.get("mode") == "activity":
            activity_line = f"/mySource/setActivity {format_number(decay_info['a_bq'])} Bq"
            irrtime_line = f"/mySource/setIrrTime {format_number(decay_info['t_s'])} s"
            decay_comment = (f"# Активность: {self.activity_var.get()} {self.activity_unit_var.get()}, "
                             f"Время: {self.irrtime_var.get()} {self.irrtime_unit_var.get()}, "
                             f"T1/2: {format_halflife_ru(decay_info['hl'])}")

        thread_line = ""
        try:
            n_threads = int(self.num_threads_var.get())
            if n_threads > 0:
                thread_line = f"/run/numberOfThreads {n_threads}"
        except ValueError:
            pass

        unchecked = [fid for fid, var in self.output_checkvars.items() if not var.get()]
        if self._decay_mode() == DECAY_MODES[0]:
            for fid in ("activity", "irr_time"):
                if fid in self.output_checkvars and fid not in unchecked:
                    unchecked.append(fid)
        all_ids = list(self.output_checkvars.keys())
        output_lines = []
        if unchecked and len(unchecked) < len(all_ids):
            output_lines.append("/myOutput/enableAll")
            for fid in unchecked:
                output_lines.append(f"/myOutput/disable {fid}")

        stamp = datetime.datetime.now().strftime("%d.%m.%Y %H:%M:%S")

        lines = [
            "# Макрос симуляции авторадиолиза",
            f"# Радионуклид: {rn_label} (Z={rn_z}, A={rn_a})",
            f"# Материал:    {mat_name}",
            f"# Вариант:     {self._geometry_label()}",
            "# Сгенерировано: program_b.py",
            "# Дата: " + stamp,
        ]
        if decay_comment:
            lines.append(decay_comment)
        lines.extend([
            "#",
            "# Запуск:",
            "#   main.exe run.mac",
            "",
            f"/myDetector/setGeometry {token}",
            f"/myDetector/setRadius {format_number(radius_cm)} cm",
            f"/myDetector/setHalfHeight {format_number(height_cm / 2)} cm",
            f"/myDetector/setMaterial {mat_name}",
        ])
        if mat_density_line:
            lines.append(mat_density_line)
        lines.extend(shell_lines)
        rn_cmd = f"/mySource/setRadionuclide {rn_z} {rn_a}"
        if rn_exc:
            rn_cmd += f" {format_number(rn_exc)}"
        lines.append(rn_cmd)
        if activity_line:
            lines.append(activity_line)
        if irrtime_line:
            lines.append(irrtime_line)
        if thread_line:
            lines.append(thread_line)
        lines.extend(output_lines)
        lines.append("/run/initialize")
        lines.append("/process/had/rdm/thresholdForVeryLongDecayTime 1.0e+60 year")
        lines.append(f"/run/beamOn {beam_on_n}")
        lines.append("")
        return "\n".join(lines)

    def _refresh_macro_preview(self):
        try:
            text = self._build_macro()
        except Exception as e:
            text = "# Ошибка построения макроса:\n# " + str(e)
        self.macro_text.delete("1.0", "end")
        self.macro_text.insert("1.0", text)

    def _generate_and_save(self):
        try:
            text = self._build_macro()
            self.macro_text.delete("1.0", "end")
            self.macro_text.insert("1.0", text)
            script_dir = os.path.dirname(os.path.abspath(__file__))
            path = os.path.join(script_dir, "run.mac")
            with open(path, "w", encoding="utf-8") as f:
                f.write(text)
            self.gen_status.config(text=f"Сохранено: {path}")
        except Exception as e:
            messagebox.showerror("Ошибка", str(e))

    def _save_macro_dialog(self):
        try:
            text = self._build_macro()
            path = filedialog.asksaveasfilename(
                initialdir=os.path.dirname(os.path.abspath(__file__)),
                defaultextension=".mac",
                filetypes=[("Macro files", "*.mac"), ("All files", "*.*")],
                initialfile="run.mac",
            )
            if path:
                with open(path, "w", encoding="utf-8") as f:
                    f.write(text)
                self.gen_status.config(text=f"Сохранено: {path}")
        except Exception as e:
            messagebox.showerror("Ошибка", str(e))

    def _collect_state(self):
        idx_mat = self.material_combo.current()
        idx_rn = self.rn_combo.current()
        geo = {
            "shape": self._geometry_label(),
            "variant": self._geometry_token(),
            "height": self.height_var.get(),
            "heightUnit": self.height_unit.get(),
            "radius": self.radius_var.get(),
            "radiusUnit": self.radius_unit.get(),
            "volume": self.volume_var.get(),
            "wall": self.wall_var.get(),
            "wallUnit": self.wall_unit.get(),
            "rim": self.rim_var.get(),
            "rimUnit": self.rim_unit.get(),
        }
        idx_glass = self.glass_material_combo.current() if hasattr(self, "glass_material_combo") else 1
        return {
            "geo": geo,
            "material": idx_mat,
            "material_idx": idx_mat - 1 if idx_mat > 0 else -1,
            "customMaterialName": self.custom_mat_name.get(),
            "customMaterialDensity": self.custom_mat_density.get(),
            "glassMaterial": idx_glass,
            "glassMaterialIdx": idx_glass - 1 if idx_glass > 0 else -1,
            "customGlassName": self.custom_glass_name.get(),
            "customGlassDensity": self.custom_glass_density.get(),
            "radionuclide": idx_rn,
            "rn_idx": idx_rn - 1 if idx_rn > 0 else -1,
            "customRnZ": self.custom_rn_z.get(),
            "customRnA": self.custom_rn_a.get(),
            "numThreads": self.num_threads_var.get(),
            "decay": {"count": self.decay_count_var.get(),
                      "mode": self._decay_mode() if hasattr(self, "decay_mode_var") else DECAY_MODES[0],
                      "activity": self.activity_var.get() if hasattr(self, "activity_var") else "1",
                      "activityUnit": self.activity_unit_var.get() if hasattr(self, "activity_unit_var") else "МБк",
                      "irrTime": self.irrtime_var.get() if hasattr(self, "irrtime_var") else "1",
                      "irrTimeUnit": self.irrtime_unit_var.get() if hasattr(self, "irrtime_unit_var") else "ч"},
            "output": {fid: var.get() for fid, var in self.output_checkvars.items()},
            "rdmThreshold": True,
        }

    def _load_state(self, s):
        geo = s.get("geo", {})
        variant = geo.get("variant", "cylinder")
        idx_v = -1
        for i, v in enumerate(GEOMETRY_VARIANTS):
            if v["token"] == variant:
                idx_v = i
                break
        if idx_v < 0:
            idx_v = 0
        self.geometry_combo.current(idx_v)
        self.height_var.set(geo.get("height", "6.0"))
        self.height_unit.set(geo.get("heightUnit", "см"))
        self.radius_var.set(geo.get("radius", "2.0"))
        self.radius_unit.set(geo.get("radiusUnit", "см"))
        self.wall_var.set(geo.get("wall", "1.5"))
        self.wall_unit.set(geo.get("wallUnit", "мм"))
        self.rim_var.set(geo.get("rim", "0"))
        self.rim_unit.set(geo.get("rimUnit", "мм"))

        mi = s.get("material", 0)
        if isinstance(mi, int) and 0 <= mi <= len(MATERIALS):
            self.material_combo.current(mi)
        elif mi == "custom" or s.get("radionuclide") == "custom":
            self.material_combo.current(0)
        self._update_material_info()
        self.custom_mat_name.set(s.get("customMaterialName", "G4_WATER"))
        self.custom_mat_density.set(s.get("customMaterialDensity", "1.0"))

        gi = s.get("glassMaterial", 1)
        if isinstance(gi, int) and 0 <= gi <= len(GLASS_MATERIALS):
            self.glass_material_combo.current(gi)
        else:
            self.glass_material_combo.current(0)
        self._update_glass_material_info()
        self.custom_glass_name.set(s.get("customGlassName", "G4_SILICON_DIOXIDE"))
        self.custom_glass_density.set(s.get("customGlassDensity", "2.32"))

        ri = s.get("radionuclide", 0)
        if isinstance(ri, int) and 0 <= ri <= len(RADIONUCLIDES):
            self.rn_combo.current(ri)
        else:
            self.rn_combo.current(0)
        self._update_radionuclide_info()
        self.custom_rn_z.set(s.get("customRnZ", "27"))
        self.custom_rn_a.set(s.get("customRnA", "60"))

        self.num_threads_var.set(s.get("numThreads", "0"))

        decay = s.get("decay", {})
        self.decay_count_var.set(decay.get("count", "1000000"))
        if hasattr(self, "decay_mode_var"):
            mode = decay.get("mode", DECAY_MODES[0])
            if mode not in DECAY_MODES:
                mode = DECAY_MODES[0]
            self.decay_mode_var.set(mode)
            self.decay_mode_combo.set(mode)
        if hasattr(self, "activity_var"):
            self.activity_var.set(decay.get("activity", "1"))
            au = decay.get("activityUnit", "МБк")
            self.activity_unit_var.set(au if au in ACTIVITY_UNITS else "МБк")
            self.irrtime_var.set(decay.get("irrTime", "1"))
            tu = decay.get("irrTimeUnit", "ч")
            self.irrtime_unit_var.set(tu if tu in TIME_UNITS else "ч")
        self._update_decay_visibility()

        output = s.get("output", {})
        if isinstance(output, dict):
            for fid, val in output.items():
                if fid in self.output_checkvars:
                    self.output_checkvars[fid].set(val)
        elif isinstance(output, list):
            for item in output:
                if isinstance(item, dict) and item.get("id") in self.output_checkvars:
                    self.output_checkvars[item["id"]].set(item.get("checked", False))

        misc = s.get("misc", {})
        self.rdm_threshold_var.set(s.get("rdmThreshold", True))
        self.macro_text.delete("1.0", "end")
        self.macro_text.insert("1.0", misc.get("macro", ""))

        self._update_geometry()
        self._refresh_macro_preview()

    # --- READ RESULTS ---

    @staticmethod
    def _reverse_result_blocks(txt):
        parts = re.split(r"(?m)(?=^=== (?:Run|WARNING) @)", txt)
        if len(parts) <= 1:
            return txt
        head, rest = parts[0], parts[1:]
        rest.reverse()
        return head + "".join(rest)

    def _show_results(self, txt):
        self.result_text.delete("1.0", "end")
        self.result_text.insert("1.0", self._reverse_result_blocks(txt))

    def _read_result(self):
        script_dir = os.path.dirname(os.path.abspath(__file__))
        path = os.path.join(script_dir, "results.txt")
        if os.path.isfile(path):
            with open(path, "r", encoding="utf-8") as f:
                self._show_results(f.read())
        else:
            messagebox.showwarning("Внимание", "Файл results.txt не найден в папке программы.\nВыберите файл вручную.")
            self._load_result_file()

    def _load_result_file(self):
        path = filedialog.askopenfilename(
            initialdir=os.path.dirname(os.path.abspath(__file__)),
            filetypes=[("Text files", "*.txt"), ("All files", "*.*")],
        )
        if path:
            with open(path, "r", encoding="utf-8") as f:
                self._show_results(f.read())

    # --- RUN SIMULATION ---

    def _run_simulation(self):
        script_dir = os.path.dirname(os.path.abspath(__file__))
        try:
            text = self._build_macro()
        except Exception as e:
            messagebox.showerror("Ошибка", str(e))
            return
        with open(os.path.join(script_dir, "run.mac"), "w", encoding="utf-8") as f:
            f.write(text)
        try:
            n_eff, _ = self._effective_decays()
            self._sim_total = n_eff
        except Exception:
            self._sim_total = 1
        exe = None
        for cand in [
            os.path.join(script_dir, "build", "Release", "main.exe"),
            os.path.join(script_dir, "build", "x64", "Release", "main.exe"),
            os.path.join(script_dir, "build", "main.exe"),
        ]:
            if os.path.isfile(cand):
                exe = cand
                break
        if exe is None:
            messagebox.showerror("Ошибка", "Не найден main.exe в папке build программы.\nСоберите проект и повторите запуск.")
            return
        self._sim_max_seen = 0
        self._sim_start = time.monotonic()
        self._sim_running = True
        self._sim_aborted = False
        self._sim_proc = None
        self._sim_rc = None
        self._sim_out = ""
        self._sim_finalized = False
        self.progress_bar["value"] = 0.0
        self.progress_label.config(text="Подготовка симуляции…")
        self.run_button.config(state="disabled", text="Идёт симуляция…")
        self.abort_button.config(state="normal")
        threading.Thread(target=self._sim_worker, args=(exe, script_dir), daemon=True).start()
        self._poll_progress()

    def _abort_simulation(self):
        if not getattr(self, "_sim_running", False):
            return
        self._sim_aborted = True
        proc = getattr(self, "_sim_proc", None)
        if proc is not None:
            try:
                if proc.poll() is None:
                    proc.terminate()
            except Exception:
                pass
        self.progress_label.config(text="Прерывание симуляции…")
        self.abort_button.config(state="disabled")

    def _sim_worker(self, exe, script_dir):
        tail = deque(maxlen=30)
        try:
            proc = subprocess.Popen(
                [exe, "run.mac"],
                cwd=script_dir,
                stdout=subprocess.PIPE,
                stderr=subprocess.STDOUT,
                bufsize=1,
                text=True,
                encoding="utf-8",
                errors="replace",
            )
            self._sim_proc = proc
            for line in iter(proc.stdout.readline, ""):
                tail.append(line.rstrip("\r\n"))
                m = re.search(r"Event\s+(\d+)\s+starts", line)
                if m:
                    n = int(m.group(1))
                    if n > self._sim_max_seen:
                        self._sim_max_seen = n
            proc.stdout.close()
            code = proc.wait()
            out = "\n".join(tail)
        except FileNotFoundError:
            code, out = -1, "Файл main.exe не найден."
        except Exception as e:
            code, out = -1, str(e)
        finally:
            self._sim_proc = None
        self._sim_rc = code
        self._sim_out = out
        self._sim_running = False

    def _poll_progress(self):
        if self._sim_running:
            if getattr(self, "_sim_aborted", False):
                self.progress_label.config(text="Прерывание симуляции…")
                self.after(300, self._poll_progress)
                return
            total = self._sim_total
            seen = self._sim_max_seen
            pct = min(100.0, seen * 100.0 / total) if total > 0 else 0.0
            if seen == 0:
                # Still initializing (physics tables, geometry, first event):
                # show a "working" label instead of a frozen-looking 0%.
                dots = "." * ((int(time.monotonic() * 2) % 3) + 1)
                self.progress_bar["value"] = 0.0
                self.progress_label.config(
                    text=f"Инициализация{dots}   (запуск симуляции)")
                self.after(300, self._poll_progress)
                return
            elapsed = time.monotonic() - self._sim_start
            eta_label = ""
            if seen > 0 and total > 0:
                remaining = elapsed * (total - seen) / seen
                mm, ss = divmod(int(remaining), 60)
                eta_label = f"  |  осталось ~{mm:02d}:{ss:02d}"
            self.progress_bar["value"] = pct
            self.progress_label.config(
                text=f"{pct:.1f}%  |  распад {seen:,} из {total:,}{eta_label}")
            self.after(300, self._poll_progress)
        elif not self._sim_finalized and self._sim_rc is not None:
            self._sim_finalized = True
            self._sim_finished(self._sim_rc, self._sim_out)

    def _sim_finished(self, code, out):
        self._sim_running = False
        self._sim_proc = None
        self.run_button.config(state="normal", text="Запустить симуляцию")
        self.abort_button.config(state="disabled")
        if getattr(self, "_sim_aborted", False):
            self.progress_bar["value"] = 0.0
            self.progress_label.config(text="Симуляция прервана пользователем.")
            return
        self.progress_bar["value"] = 100.0
        self.progress_label.config(text="Симуляция завершена.")
        script_dir = os.path.dirname(os.path.abspath(__file__))
        path = os.path.join(script_dir, "results.txt")
        if os.path.isfile(path):
            with open(path, "r", encoding="utf-8") as f:
                self._show_results(f.read())
        if code != 0:
            tail = "\n".join(out.splitlines()[-20:])
            messagebox.showwarning("Внимание",
                                   f"Симуляция завершилась с кодом {code}.\n\nПоследние строки вывода:\n{tail}")


# ============================================================
# MAIN
# ============================================================
if __name__ == "__main__":
    app = ProgramB()
    app.mainloop()
