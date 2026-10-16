import csv
import math
from datetime import datetime

import matplotlib.pyplot as plt

CSV_FILE = "output/solar_model_comparison.csv"
OUTPUT_FILE = "output/solar_duration_heatmap.png"
FRENCH_MONTHS = (
    "janv.", "févr.", "mars", "avr.", "mai", "juin",
    "juil.", "août", "sept.", "oct.", "nov.", "déc.",
)


def load_duration_data(csv_path):
    data_by_year = {}
    latitude = None
    solar_altitude_threshold = 0.0

    try:
        with open(csv_path, "r", newline="") as csv_file:
            reader = csv.DictReader(csv_file)
            for line_number, row in enumerate(reader, start=2):
                try:
                    date = datetime.strptime(row["Date"], "%d/%m/%Y")
                    row_latitude = float(row["Latitude_Degrees"])
                    duration = float(row["Duration_Spencer_Hours"])
                    row_threshold = float(
                        row.get("Solar_Altitude_Threshold_Degrees", "0")
                    )
                except (KeyError, TypeError, ValueError):
                    print(f"[AVERTISSEMENT] Ligne {line_number} invalide, ignorée.")
                    continue

                if (
                    not math.isfinite(row_latitude)
                    or not math.isfinite(duration)
                    or not math.isfinite(row_threshold)
                ):
                    print(f"[AVERTISSEMENT] Valeur non finie ligne {line_number}, ignorée.")
                    continue

                latitude = row_latitude
                solar_altitude_threshold = row_threshold
                data_by_year.setdefault(date.year, []).append(
                    (date.timetuple().tm_yday - 1, duration)
                )
    except OSError as error:
        print(f"[ERREUR] Impossible de lire '{csv_path}' : {error}")
        return None

    if not data_by_year:
        print(f"[ERREUR] Aucune donnée valide dans '{csv_path}'.")
        return None

    return latitude, data_by_year, solar_altitude_threshold


def latitude_label(latitude):
    if latitude == 0:
        return "l'équateur"
    hemisphere = "N" if latitude > 0 else "S"
    return f"{abs(latitude):.2f}° {hemisphere}"


def plot_duration_heatmap(csv_path):
    data = load_duration_data(csv_path)
    if data is None:
        return

    latitude, data_by_year, solar_altitude_threshold = data
    years = sorted(data_by_year)
    year_indexes = {year: index for index, year in enumerate(years)}
    matrix = [[math.nan] * 366 for _ in years]
    for year, entries in data_by_year.items():
        for day_index, duration in entries:
            matrix[year_indexes[year]][day_index] = duration

    figure, axes = plt.subplots(figsize=(12, 4.5), constrained_layout=True)
    image = axes.imshow(
        matrix,
        aspect="auto",
        origin="lower",
        interpolation="nearest",
        vmin=0,
        vmax=24,
        cmap="YlGnBu",
    )
    if len(years) > 1:
        all_durations = [
            duration for entries in data_by_year.values() for _, duration in entries
        ]
        contour_min = math.ceil(min(all_durations) / 2) * 2
        contour_max = math.floor(max(all_durations) / 2) * 2
        contour_levels = list(range(contour_min, contour_max + 1, 2))
        if contour_levels:
            contours = axes.contour(
                range(366),
                range(len(years)),
                matrix,
                levels=contour_levels,
                colors="#263238",
                linewidths=0.7,
                alpha=0.8,
            )
            contour_labels = axes.clabel(
                contours,
                inline=True,
                fontsize=7,
                fmt=lambda value: f"{value:.0f} h",
            )
            for contour_label in contour_labels:
                contour_label.set_rotation(0)
    month_positions = [
        datetime(2025, month, 1).timetuple().tm_yday - 1
        for month in range(1, 13)
    ]
    axes.set_xticks(month_positions, FRENCH_MONTHS)
    axes.set_yticks(range(len(years)), years)
    axes.set_xlim(-0.5, 365.5)
    axes.set_xlabel("Jour de l'année")
    axes.set_ylabel("Année")
    axes.set_title(
        f"Durée du jour à {latitude_label(latitude)} (modèle Spencer, "
        f"seuil {solar_altitude_threshold:g}°)"
    )
    colorbar = figure.colorbar(image, ax=axes)
    colorbar.set_label("Durée du jour (heures)")

    figure.savefig(OUTPUT_FILE, dpi=160)
    plt.close(figure)
    print(f"[OK] Carte thermique enregistrée dans '{OUTPUT_FILE}'.")


if __name__ == "__main__":
    plot_duration_heatmap(CSV_FILE)