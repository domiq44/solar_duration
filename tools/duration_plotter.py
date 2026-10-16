# --- Fichier : duration_plotter.py ---
import csv
import math
import re
from datetime import datetime

import matplotlib.dates as mdates
import matplotlib.pyplot as plt
from matplotlib.ticker import AutoMinorLocator, MultipleLocator

CSV_FILE = "output/solar_duration.csv"
FRENCH_MONTHS = (
    "janv.", "févr.", "mars", "avr.", "mai", "juin",
    "juil.", "août", "sept.", "oct.", "nov.", "déc.",
)
MODEL_NAMES = {1: "Sinusoïdal", 2: "Spencer", 3: "Meeus"}
MODEL_DURATION_FIELDS = {
    1: "Duration_Sinusoidal_Hours",
    2: "Duration_Spencer_Hours",
    3: "Duration_Meeus_Hours",
}
MODEL_COLORS = {1: "#0072B2", 2: "#D55E00", 3: "#009E73"}
LATITUDE_PATTERN = re.compile(
    r"(\d+)\s*deg\s+(\d+)\s*min\s+([\d.]+)\s*sec\s+(Nord|Sud)",
    re.IGNORECASE,
)


def load_plot_metadata(config_path):
    config_values = {}
    try:
        with open(config_path, "r") as config_file:
            for line in config_file:
                line = line.split("//", 1)[0].split("#", 1)[0].strip()
                if "=" in line:
                    key, value = line.split("=", 1)
                    config_values[key.strip().lower()] = value.strip()
    except OSError:
        return "Latitude inconnue", "modèle inconnu", 0, 0.0

    latitude_match = LATITUDE_PATTERN.fullmatch(config_values.get("latitude", ""))
    if latitude_match:
        degrees, minutes, seconds, hemisphere = latitude_match.groups()
        latitude = int(degrees) + int(minutes) / 60 + float(seconds) / 3600
        if hemisphere.lower() == "sud":
            latitude = -latitude
        hemisphere_label = "N" if latitude >= 0 else "S"
        latitude_label = f"{abs(latitude):.2f}° {hemisphere_label}"
    else:
        latitude_label = "latitude inconnue"

    try:
        mode = int(config_values.get("mode_solaire", ""))
    except ValueError:
        mode = 0
    model_name = MODEL_NAMES.get(mode, "modèle inconnu")
    try:
        solar_altitude_threshold = float(
            config_values.get("altitude_solaire_seuil", "-0.833")
        )
    except ValueError:
        solar_altitude_threshold = -0.833
    return latitude_label, model_name, mode, solar_altitude_threshold


def format_month_tick(date_number, position):
    date = mdates.num2date(date_number)
    month_label = FRENCH_MONTHS[date.month - 1]
    if date.month == 1 or position == 0:
        return f"{month_label}\n{date.year}"
    return month_label


def plot_duration_vs_date(csv_path):
    dates = []
    durations = []
    durations_by_model = {mode: [] for mode in MODEL_NAMES}
    model_data_available = True

    try:
        with open(csv_path, "r", newline="") as csv_file:
            reader = csv.DictReader(csv_file)
            for line_number, row in enumerate(reader, start=2):
                try:
                    date = datetime.strptime(row["Date"], "%d/%m/%Y")
                    duration = float(row["Duration_Hours"])
                except (KeyError, TypeError, ValueError):
                    print(f"[AVERTISSEMENT] Ligne {line_number} invalide, ignorée.")
                    continue

                if not math.isfinite(duration):
                    print(f"[AVERTISSEMENT] Durée non finie à la ligne {line_number}, ignorée.")
                    continue

                try:
                    model_values = {
                        mode: float(row[field])
                        for mode, field in MODEL_DURATION_FIELDS.items()
                    }
                except (KeyError, TypeError, ValueError):
                    model_values = None
                    model_data_available = False

                if model_values is not None:
                    if not all(math.isfinite(value) for value in model_values.values()):
                        model_data_available = False
                    else:
                        for mode, value in model_values.items():
                            durations_by_model[mode].append(value)

                dates.append(date)
                durations.append(duration)
    except OSError as error:
        print(f"[ERREUR] Impossible de lire '{csv_path}' : {error}")
        return

    if not dates:
        print(f"[ERREUR] Aucune donnée valide dans '{csv_path}'.")
        return

    (
        latitude_label,
        model_name,
        selected_mode,
        solar_altitude_threshold,
    ) = load_plot_metadata("config/solar_duration.dat")
    figure, axes = plt.subplots(figsize=(12, 6), constrained_layout=True)
    if model_data_available:
        for mode, curve_model_name in MODEL_NAMES.items():
            selected = mode == selected_mode
            axes.plot(
                dates,
                durations_by_model[mode],
                color=MODEL_COLORS[mode],
                linewidth=2.4 if selected else 1.5,
                linestyle="-" if selected else "--",
                alpha=1.0 if selected else 0.8,
                label=(
                    f"{curve_model_name} (configuré)"
                    if selected
                    else curve_model_name
                ),
                zorder=3 if selected else 2,
            )
    else:
        mean_duration = sum(durations) / len(durations)
        axes.plot(
            dates, durations, color="#0072B2", linewidth=1.8,
            label="Durée quotidienne",
        )
        axes.axhline(
            mean_duration,
            color="#D55E00",
            linestyle="--",
            linewidth=1.5,
            label=f"Moyenne de la période ({mean_duration:.2f} h)",
        )

    axes.set_xlim(dates[0], dates[-1])
    axes.xaxis.set_major_locator(mdates.MonthLocator(bymonthday=1, interval=1))
    axes.xaxis.set_major_formatter(format_month_tick)
    axes.tick_params(axis="x", labelsize=8)
    axes.set_ylim(0, 24)
    axes.yaxis.set_major_locator(MultipleLocator(5))
    axes.yaxis.set_minor_locator(AutoMinorLocator(5))
    axes.tick_params(axis="y", which="minor", length=3)
    axes.set_title(
        f"Durée du jour à {latitude_label} — comparaison des modèles\n"
        f"Modèle configuré : {model_name}, seuil {solar_altitude_threshold:g}°\n"
        f"{dates[0]:%d/%m/%Y} au {dates[-1]:%d/%m/%Y}"
    )
    axes.set_xlabel("Date")
    axes.set_ylabel("Durée du jour (heures)")
    axes.set_axisbelow(True)
    axes.grid(axis="y", linestyle=":", alpha=0.45)
    axes.legend(frameon=False)

    output_filename = "output/solar_duration.png"
    figure.savefig(output_filename, dpi=160)
    plt.close(figure)
    print(f"[OK] Graphique enregistré dans '{output_filename}'.")


if __name__ == "__main__":
    plot_duration_vs_date(CSV_FILE)
