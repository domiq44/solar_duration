import csv
import math
from datetime import datetime

import matplotlib.dates as mdates
import matplotlib.pyplot as plt
from matplotlib.ticker import MultipleLocator

CSV_FILE = "output/solar_model_comparison.csv"
LATITUDES = (0.0, 23.5, 45.0, 67.0, 80.0)
LATITUDE_STYLES = ("#333333", "#0072B2", "#D55E00", "#009E73", "#CC79A7")
FRENCH_MONTHS = (
    "janv.", "févr.", "mars", "avr.", "mai", "juin",
    "juil.", "août", "sept.", "oct.", "nov.", "déc.",
)


def format_month_tick(date_number, position):
    month = mdates.num2date(date_number).month
    return FRENCH_MONTHS[month - 1]


def load_latest_year(csv_path):
    declinations_by_year = {}
    solar_altitude_threshold = 0.0

    try:
        with open(csv_path, "r", newline="") as csv_file:
            reader = csv.DictReader(csv_file)
            for line_number, row in enumerate(reader, start=2):
                try:
                    date = datetime.strptime(row["Date"], "%d/%m/%Y")
                    declination = float(row["Delta_Spencer"])
                    row_threshold = float(
                        row.get("Solar_Altitude_Threshold_Degrees", "0")
                    )
                except (KeyError, TypeError, ValueError):
                    print(f"[AVERTISSEMENT] Ligne {line_number} invalide, ignorée.")
                    continue

                if not math.isfinite(declination) or not math.isfinite(row_threshold):
                    print(f"[AVERTISSEMENT] Déclinaison non finie ligne {line_number}, ignorée.")
                    continue

                declinations_by_year.setdefault(date.year, []).append(
                    (date, declination)
                )
                solar_altitude_threshold = row_threshold
    except OSError as error:
        print(f"[ERREUR] Impossible de lire '{csv_path}' : {error}")
        return None

    if not declinations_by_year:
        print(f"[ERREUR] Aucune donnée valide dans '{csv_path}'.")
        return None

    latest_year = max(declinations_by_year)
    return latest_year, declinations_by_year[latest_year], solar_altitude_threshold


def calculate_day_duration(declination, latitude, solar_altitude_threshold):
    latitude_radians = math.radians(latitude)
    declination_radians = math.radians(declination)
    threshold_radians = math.radians(solar_altitude_threshold)
    cosine_hour_angle = (
        math.sin(threshold_radians)
        - math.sin(latitude_radians) * math.sin(declination_radians)
    ) / (math.cos(latitude_radians) * math.cos(declination_radians))
    cosine_hour_angle = min(1.0, max(-1.0, cosine_hour_angle))
    return 24.0 / math.pi * math.acos(cosine_hour_angle)


def plot_duration_by_latitude(csv_path):
    data = load_latest_year(csv_path)
    if data is None:
        return

    year, daily_declinations, solar_altitude_threshold = data
    dates = [date for date, _ in daily_declinations]
    figure, axes = plt.subplots(figsize=(12, 6), constrained_layout=True)

    for latitude, color in zip(LATITUDES, LATITUDE_STYLES):
        durations = [
            calculate_day_duration(
                declination, latitude, solar_altitude_threshold
            )
            for _, declination in daily_declinations
        ]
        axes.plot(
            dates,
            durations,
            color=color,
            linewidth=1.7,
            label=f"{latitude:g}° N" if latitude else "Équateur",
        )

    axes.axhline(12, color="#777777", linestyle=":", linewidth=1.0)
    axes.set_xlim(dates[0], dates[-1])
    axes.xaxis.set_major_locator(mdates.MonthLocator())
    axes.xaxis.set_major_formatter(format_month_tick)
    axes.set_ylim(0, 24)
    axes.yaxis.set_major_locator(MultipleLocator(4))
    axes.set_title(
        f"Durée du jour selon la latitude ({year}, modèle Spencer, "
        f"seuil {solar_altitude_threshold:g}°)"
    )
    axes.set_xlabel("Mois")
    axes.set_ylabel("Durée du jour (heures)")
    axes.set_axisbelow(True)
    axes.grid(axis="y", linestyle=":", alpha=0.45)
    axes.legend(frameon=False, ncol=3)

    output_filename = "output/solar_latitudes.png"
    figure.savefig(output_filename, dpi=160)
    plt.close(figure)
    print(f"[OK] Graphique enregistré dans '{output_filename}'.")


if __name__ == "__main__":
    plot_duration_by_latitude(CSV_FILE)