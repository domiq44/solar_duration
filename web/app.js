"use strict";

const radians = Math.PI / 180;
const degrees = 180 / Math.PI;
const elements = Object.fromEntries([
  "date-input", "today-button", "latitude-input", "latitude-range", "location-button",
  "model-select", "threshold-input", "threshold-reset", "status-message", "result-date",
  "duration-value", "sunrise-value", "sunset-value", "declination-value", "model-chip",
  "latitude-display", "solar-chart"
].map((id) => [id, document.getElementById(id)]));

function localDateString(date) {
  return `${date.getFullYear()}-${String(date.getMonth() + 1).padStart(2, "0")}-${String(date.getDate()).padStart(2, "0")}`;
}

function isLeapYear(year) {
  return year % 4 === 0 && (year % 100 !== 0 || year % 400 === 0);
}

function dayOfYear(year, month, day) {
  const monthLengths = [31, isLeapYear(year) ? 29 : 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31];
  return monthLengths.slice(0, month - 1).reduce((total, days) => total + days, day);
}

function parseDate(value) {
  const match = /^(\d{4})-(\d{2})-(\d{2})$/.exec(value);
  if (!match) return null;
  const [, yearText, monthText, dayText] = match;
  const year = Number(yearText);
  const month = Number(monthText);
  const day = Number(dayText);
  const date = new Date(year, month - 1, day, 12);
  if (date.getFullYear() !== year || date.getMonth() !== month - 1 || date.getDate() !== day) return null;
  return { year, month, day, ordinal: dayOfYear(year, month, day), date };
}

function sinusoidalDeclination({ year, ordinal }) {
  const yearLength = isLeapYear(year) ? 366 : 365;
  const phase = (ordinal / yearLength - 81 / yearLength) * 2 * Math.PI;
  return 23.45 * Math.sin(phase);
}

function spencerDeclination({ ordinal }) {
  const phase = (ordinal - 1) * 2 * Math.PI / 365;
  const delta = 0.006918 - 0.399912 * Math.cos(phase) + 0.070257 * Math.sin(phase)
    - 0.006758 * Math.cos(2 * phase) + 0.000907 * Math.sin(2 * phase)
    - 0.002697 * Math.cos(3 * phase) + 0.00148 * Math.sin(3 * phase);
  return delta * degrees;
}

function julianDay({ year, month, day }) {
  let adjustedYear = year;
  let adjustedMonth = month;
  if (adjustedMonth <= 2) {
    adjustedYear -= 1;
    adjustedMonth += 12;
  }
  const century = Math.trunc(adjustedYear / 100);
  const correction = 2 - century + Math.trunc(century / 4);
  return Math.floor(365.25 * (adjustedYear + 4716))
    + Math.floor(30.6001 * (adjustedMonth + 1)) + day + correction - 1524.5 + 0.5;
}

function meeusDeclination(date) {
  const julianDate = julianDay({ year: date.year, month: 1, day: 1 }) + date.ordinal - 1;
  const centuries = (julianDate - 2451545) / 36525;
  const meanLongitude = (280.46646 + 36000.76983 * centuries + 0.0003032 * centuries ** 2) % 360;
  const meanAnomaly = ((357.52911 + 35999.05029 * centuries - 0.0001537 * centuries ** 2) % 360) * radians;
  const equationOfCenter = (1.914602 - 0.004817 * centuries - 0.000014 * centuries ** 2) * Math.sin(meanAnomaly)
    + (0.019993 - 0.000101 * centuries) * Math.sin(2 * meanAnomaly)
    + 0.000289 * Math.sin(3 * meanAnomaly);
  const trueLongitude = meanLongitude + equationOfCenter;
  const omega = (125.04 - 1934.136 * centuries) * radians;
  const apparentLongitude = (trueLongitude - 0.00569 - 0.00478 * Math.sin(omega)) * radians;
  const meanObliquity = 23.43929111 - 0.013004167 * centuries - 0.000000164 * centuries ** 2 + 0.000000504 * centuries ** 3;
  const obliquity = (meanObliquity + 0.00256 * Math.cos(omega)) * radians;
  return Math.asin(Math.sin(obliquity) * Math.sin(apparentLongitude)) * degrees;
}

function declinationFor(date, model) {
  if (model === "spencer") return spencerDeclination(date);
  if (model === "sinusoidal") return sinusoidalDeclination(date);
  return meeusDeclination(date);
}

function durationHours(latitude, declination, threshold) {
  if (Math.abs(latitude) === 90) {
    const altitudeAtPole = latitude > 0 ? declination : -declination;
    return altitudeAtPole > threshold ? 24 : 0;
  }
  const phi = latitude * radians;
  const delta = declination * radians;
  const altitude = threshold * radians;
  const cosHourAngle = (Math.sin(altitude) - Math.sin(phi) * Math.sin(delta))
    / (Math.cos(phi) * Math.cos(delta));
  const hourAngle = Math.acos(Math.max(-1, Math.min(1, cosHourAngle)));
  return 2 * hourAngle * 12 / Math.PI;
}

function solarAltitude(hour, latitude, declination) {
  const hourAngle = (hour - 12) * 15 * radians;
  const sineAltitude = Math.sin(latitude * radians) * Math.sin(declination * radians)
    + Math.cos(latitude * radians) * Math.cos(declination * radians) * Math.cos(hourAngle);
  return Math.asin(Math.max(-1, Math.min(1, sineAltitude))) * degrees;
}

function formatDuration(hours) {
  const minutesTotal = Math.round(hours * 60);
  return `${Math.floor(minutesTotal / 60)} h ${String(minutesTotal % 60).padStart(2, "0")} min`;
}

function formatSolarTime(hour) {
  const minutesTotal = Math.round((((hour % 24) + 24) % 24) * 60) % 1440;
  return `${String(Math.floor(minutesTotal / 60)).padStart(2, "0")}:${String(minutesTotal % 60).padStart(2, "0")}`;
}

function formatLatitude(latitude) {
  const hemisphere = latitude < 0 ? "S" : "N";
  const absolute = Math.abs(latitude).toLocaleString("fr-FR", { minimumFractionDigits: 2, maximumFractionDigits: 4 });
  return `${absolute}° ${hemisphere}`;
}

function setStatus(message, error = false) {
  elements["status-message"].textContent = message;
  elements["status-message"].classList.toggle("error", error);
}

function drawChart(latitude, declination, threshold) {
  const canvas = elements["solar-chart"];
  const bounds = canvas.getBoundingClientRect();
  if (!bounds.width || !bounds.height) return;
  const scale = window.devicePixelRatio || 1;
  canvas.width = Math.round(bounds.width * scale);
  canvas.height = Math.round(bounds.height * scale);
  const context = canvas.getContext("2d");
  context.scale(scale, scale);
  const width = bounds.width;
  const height = bounds.height;
  const plot = { left: 38, right: width - 9, top: 12, bottom: height - 26 };
  const x = (hour) => plot.left + hour / 24 * (plot.right - plot.left);
  const y = (altitude) => plot.top + (90 - altitude) / 180 * (plot.bottom - plot.top);

  context.clearRect(0, 0, width, height);
  context.font = "10px Segoe UI, sans-serif";
  context.textBaseline = "middle";
  context.fillStyle = "#647169";
  context.strokeStyle = "#e5ebe6";
  context.lineWidth = 1;
  for (const altitude of [-60, 0, 60]) {
    const position = y(altitude);
    context.beginPath();
    context.moveTo(plot.left, position);
    context.lineTo(plot.right, position);
    context.stroke();
    context.textAlign = "right";
    context.fillText(`${altitude}°`, plot.left - 8, position);
  }

  context.setLineDash([4, 4]);
  context.strokeStyle = "#c75b42";
  context.save();
  context.beginPath();
  context.rect(plot.left, plot.top, plot.right - plot.left, y(threshold) - plot.top);
  context.clip();
  context.beginPath();
  context.moveTo(plot.left, y(threshold));
  context.lineTo(plot.right, y(threshold));
  context.stroke();
  context.setLineDash([]);

  context.beginPath();
  for (let step = 0; step <= 144; step += 1) {
    const hour = step / 6;
    const pointX = x(hour);
    const pointY = y(solarAltitude(hour, latitude, declination));
    if (step === 0) context.moveTo(pointX, pointY);
    else context.lineTo(pointX, pointY);
  }
  context.lineTo(plot.right, y(-90));
  context.lineTo(plot.left, y(-90));
  context.closePath();
  context.fillStyle = "rgba(231, 168, 55, 0.15)";
  context.fill();
  context.restore();

  context.beginPath();
  for (let step = 0; step <= 144; step += 1) {
    const hour = step / 6;
    const pointX = x(hour);
    const pointY = y(solarAltitude(hour, latitude, declination));
    if (step === 0) context.moveTo(pointX, pointY);
    else context.lineTo(pointX, pointY);
  }
  context.strokeStyle = "#d99422";
  context.lineWidth = 2.5;
  context.lineJoin = "round";
  context.stroke();
  context.fillStyle = "#647169";
  context.textAlign = "center";
  for (const hour of [0, 6, 12, 18, 24]) context.fillText(`${String(hour).padStart(2, "0")} h`, x(hour), height - 10);
  canvas.setAttribute("aria-label", `Hauteur solaire en heure locale. Latitude ${formatLatitude(latitude)}, déclinaison ${declination.toFixed(2)} degrés, seuil ${threshold.toFixed(3)} degrés.`);
}

function update() {
  const date = parseDate(elements["date-input"].value);
  const latitudeText = elements["latitude-input"].value.trim();
  const thresholdText = elements["threshold-input"].value.trim();
  const latitude = Number(latitudeText);
  const threshold = Number(thresholdText);
  if (!date || date.year < 1900 || date.year > 3000) {
    elements["date-input"].setAttribute("aria-invalid", "true");
    setStatus("Choisissez une date comprise entre 1900 et 3000.", true);
    return;
  }
  elements["date-input"].removeAttribute("aria-invalid");
  if (!latitudeText || !Number.isFinite(latitude) || latitude < -90 || latitude > 90) {
    elements["latitude-input"].setAttribute("aria-invalid", "true");
    setStatus("La latitude doit être comprise entre −90° et 90°.", true);
    return;
  }
  elements["latitude-input"].removeAttribute("aria-invalid");
  if (!thresholdText || !Number.isFinite(threshold) || threshold < -90 || threshold > 90) {
    elements["threshold-input"].setAttribute("aria-invalid", "true");
    setStatus("Le seuil doit être compris entre −90° et 90°.", true);
    return;
  }
  elements["threshold-input"].removeAttribute("aria-invalid");

  const model = elements["model-select"].value;
  const declination = declinationFor(date, model);
  const duration = durationHours(latitude, declination, threshold);
  const dateLabel = new Intl.DateTimeFormat("fr-FR", { dateStyle: "full" }).format(date.date);
  elements["result-date"].textContent = dateLabel;
  elements["duration-value"].textContent = formatDuration(duration);
  elements["declination-value"].textContent = `${declination >= 0 ? "+" : "−"}${Math.abs(declination).toFixed(2)}°`;
  elements["model-chip"].textContent = model === "meeus" ? "MEEUS" : model === "spencer" ? "SPENCER" : "SINUSOÏDAL";
  elements["latitude-display"].textContent = `Latitude : ${formatLatitude(latitude)}`;
  elements["sunrise-value"].textContent = duration === 0 ? "Nuit polaire" : duration === 24 ? "Jour polaire" : formatSolarTime(12 - duration / 2);
  elements["sunset-value"].textContent = duration === 0 ? "Nuit polaire" : duration === 24 ? "Jour polaire" : formatSolarTime(12 + duration / 2);
  setStatus("");
  drawChart(latitude, declination, threshold);
}

elements["date-input"].value = localDateString(new Date());
elements["today-button"].addEventListener("click", () => {
  elements["date-input"].value = localDateString(new Date());
  update();
});
elements["date-input"].addEventListener("input", update);
elements["model-select"].addEventListener("change", update);
elements["threshold-input"].addEventListener("input", update);
elements["threshold-reset"].addEventListener("click", () => {
  elements["threshold-input"].value = "-0.833";
  update();
});
elements["latitude-input"].addEventListener("input", () => {
  const text = elements["latitude-input"].value.trim();
  const latitude = Number(text);
  if (text && Number.isFinite(latitude) && latitude >= -90 && latitude <= 90) {
    elements["latitude-range"].value = String(latitude);
  }
  update();
});
elements["latitude-range"].addEventListener("input", () => {
  elements["latitude-input"].value = Number(elements["latitude-range"].value).toFixed(2);
  update();
});
elements["location-button"].addEventListener("click", () => {
  if (!navigator.geolocation) {
    setStatus("La géolocalisation n’est pas disponible dans ce navigateur.", true);
    return;
  }
  setStatus("Recherche de la position…");
  navigator.geolocation.getCurrentPosition(
    (position) => {
      const latitude = position.coords.latitude;
      elements["latitude-input"].value = latitude.toFixed(4);
      elements["latitude-range"].value = String(Math.max(-90, Math.min(90, latitude)));
      update();
    },
    () => setStatus("Position indisponible. Saisissez la latitude ou réglez le curseur.", true),
    { enableHighAccuracy: false, timeout: 8000, maximumAge: 300000 }
  );
});
new ResizeObserver(update).observe(elements["solar-chart"]);
update();