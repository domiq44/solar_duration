import csv
import math
from datetime import datetime

import matplotlib.dates as mdates
import matplotlib.pyplot as plt
from matplotlib.ticker import AutoMinorLocator, MultipleLocator

CSV_FILE = "output/solar_model_comparison.csv"


def load_model_data(csv_path):
    dates = []
    sinusoidal = []
    spencer = []
    meeus = []
    sinusoidal_duration = []
    spencer_duration = []
    meeus_duration = []
    latitude = None
    solar_altitude_threshold = 0.0

    try:
        with open(csv_path, "r", newline="") as csv_file:
            reader = csv.DictReader(csv_file)
            for line_number, row in enumerate(reader, start=2):
                try:
                    date = datetime.strptime(row["Date"], "%d/%m/%Y")
                    values = (
                        float(row["Delta_Sinusoidal"]),
                        float(row["Delta_Spencer"]),
                        float(row["Delta_Meeus"]),
                        float(row["Duration_Sinusoidal_Hours"]),
                        float(row["Duration_Spencer_Hours"]),
                        float(row["Duration_Meeus_Hours"]),
                        float(row["Latitude_Degrees"]),
                        float(row.get("Solar_Altitude_Threshold_Degrees", "0")),
                    )
                except (KeyError, TypeError, ValueError):
                    print(f"[AVERTISSEMENT] Ligne {line_number} invalide, ignorée.")
                    continue

                if not all(math.isfinite(value) for value in values):
                    print(f"[AVERTISSEMENT] Valeur non finie ligne {line_number}, ignorée.")
                    continue

                dates.append(date)
                sinusoidal.append(values[0])
                spencer.append(values[1])
                meeus.append(values[2])
                sinusoidal_duration.append(values[3])
                spencer_duration.append(values[4])
                meeus_duration.append(values[5])
                latitude = values[6]
                solar_altitude_threshold = values[7]
    except OSError as error:
        print(f"[ERREUR] Impossible de lire '{csv_path}' : {error}")
        return None

    if not dates:
        print(f"[ERREUR] Aucune donnée valide dans '{csv_path}'.")
        return None

    latest_year = max(date.year for date in dates)
    selected_indexes = [
        index for index, date in enumerate(dates) if date.year == latest_year
    ]
    dates = [dates[index] for index in selected_indexes]
    sinusoidal = [sinusoidal[index] for index in selected_indexes]
    spencer = [spencer[index] for index in selected_indexes]
    meeus = [meeus[index] for index in selected_indexes]
    sinusoidal_duration = [
        sinusoidal_duration[index] for index in selected_indexes
    ]
    spencer_duration = [spencer_duration[index] for index in selected_indexes]
    meeus_duration = [meeus_duration[index] for index in selected_indexes]

    return (
        latest_year,
        dates,
        sinusoidal,
        spencer,
        meeus,
        sinusoidal_duration,
        spencer_duration,
        meeus_duration,
        latitude,
        solar_altitude_threshold,
    )


def plot_model_comparison(csv_path):
    data = load_model_data(csv_path)
    if data is None:
        return

    (
        year,
        dates,
        sinusoidal,
        spencer,
        meeus,
        sinusoidal_duration,
        spencer_duration,
        meeus_duration,
        latitude,
        solar_altitude_threshold,
    ) = data
    figure, (
        declination_axes,
        declination_difference_axes,
        duration_difference_axes,
    ) = plt.subplots(
        3,
        1,
        figsize=(12, 10),
        sharex=True,
        gridspec_kw={"height_ratios": [2, 1, 1]},
        constrained_layout=True,
    )

    model_styles = {
        "sinusoidal": {"color": "#0072B2", "linestyle": "-"},
        "spencer": {"color": "#D55E00", "linestyle": "--"},
        "meeus": {"color": "#009E73", "linestyle": "-."},
    }
    declination_axes.plot(
        dates, sinusoidal, linewidth=1.5, label="Sinusoïdal", **model_styles["sinusoidal"]
    )
    declination_axes.plot(
        dates, spencer, linewidth=1.5, label="Spencer", **model_styles["spencer"]
    )
    declination_axes.plot(
        dates, meeus, linewidth=1.5, label="Meeus", **model_styles["meeus"]
    )
    declination_axes.set_title(f"Déclinaison solaire selon le modèle ({year})")
    declination_axes.set_ylabel("Déclinaison (°)")
    declination_axes.yaxis.set_major_locator(MultipleLocator(10))
    declination_axes.yaxis.set_minor_locator(AutoMinorLocator(10))
    declination_axes.tick_params(axis="y", which="minor", length=3)
    declination_axes.grid(axis="y", linestyle=":", alpha=0.45)
    declination_axes.legend(frameon=False, ncol=3, loc="upper right")

    declination_difference_axes.axhline(0, color="#444444", linewidth=0.9)
    declination_difference_axes.plot(
        dates,
        [value - baseline for value, baseline in zip(sinusoidal, spencer)],
        color=model_styles["sinusoidal"]["color"],
        linewidth=1.3,
        label="Sinusoïdal − Spencer",
    )
    declination_difference_axes.plot(
        dates,
        [value - baseline for value, baseline in zip(meeus, spencer)],
        color=model_styles["meeus"]["color"],
        linewidth=1.3,
        label="Meeus − Spencer",
    )
    declination_difference_axes.set_ylabel("Écart à Spencer (°)")
    declination_difference_axes.grid(axis="y", linestyle=":", alpha=0.45)
    declination_difference_axes.legend(frameon=False, ncol=2, loc="upper right")

    duration_difference_axes.axhline(0, color="#444444", linewidth=0.9)
    sinusoidal_duration_differences = [
        (value - baseline) * 60
        for value, baseline in zip(sinusoidal_duration, spencer_duration)
    ]
    meeus_duration_differences = [
        (value - baseline) * 60
        for value, baseline in zip(meeus_duration, spencer_duration)
    ]
    sinusoidal_max_difference = max(
        abs(value) for value in sinusoidal_duration_differences
    )
    meeus_max_difference = max(abs(value) for value in meeus_duration_differences)
    duration_difference_axes.plot(
        dates,
        sinusoidal_duration_differences,
        color=model_styles["sinusoidal"]["color"],
        linewidth=1.3,
        label=f"Sinusoïdal − Spencer (max. {sinusoidal_max_difference:.1f} min)",
    )
    duration_difference_axes.plot(
        dates,
        meeus_duration_differences,
        color=model_styles["meeus"]["color"],
        linewidth=1.3,
        label=f"Meeus − Spencer (max. {meeus_max_difference:.1f} min)",
    )
    hemisphere = "N" if latitude > 0 else "S"
    latitude_label = f"{abs(latitude):.2f}° {hemisphere}" if latitude else "l'équateur"
    duration_difference_axes.set_title(
        f"Impact sur la durée du jour à {latitude_label} "
        f"(seuil {solar_altitude_threshold:g}°)",
        loc="left",
        fontsize=10,
    )
    duration_difference_axes.set_ylabel("Écart à Spencer (min)")
    duration_difference_axes.set_xlabel("Date")
    duration_difference_axes.grid(axis="y", linestyle=":", alpha=0.45)
    duration_difference_axes.legend(frameon=False, ncol=2, loc="upper right")

    date_locator = mdates.AutoDateLocator(minticks=6, maxticks=10)
    duration_difference_axes.xaxis.set_major_locator(date_locator)
    duration_difference_axes.xaxis.set_major_formatter(
        mdates.ConciseDateFormatter(date_locator)
    )
    duration_difference_axes.xaxis.set_minor_locator(mdates.MonthLocator(bymonthday=1))
    duration_difference_axes.tick_params(axis="x", which="minor", length=3)

    output_filename = "output/solar_model_comparison.png"
    figure.savefig(output_filename, dpi=160)
    plt.close(figure)
    print(f"[OK] Graphique enregistré dans '{output_filename}'.")


if __name__ == "__main__":
    plot_model_comparison(CSV_FILE)
