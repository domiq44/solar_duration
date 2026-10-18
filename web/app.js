"use strict";

const radians = Math.PI / 180;
const degrees = 180 / Math.PI;
const SETTINGS_STORAGE_KEY = "solar-duration-settings-v1";
const elements = Object.fromEntries([
  "date-input", "today-button", "city-input", "city-suggestions", "latitude-input",
  "latitude-range", "longitude-input", "location-button", "model-select", "timezone-select",
  "threshold-input", "threshold-reset", "status-message", "result-date", "duration-value",
  "sunrise-value", "sunset-value", "declination-value", "model-chip", "latitude-display",
  "settings-reset", "solar-chart"
].map((id) => [id, document.getElementById(id)]));

const cities = [
  { name: "Ajaccio", latitude: 41.9192, longitude: 8.7386, timezone: "Europe/Paris" },
  { name: "Aix-en-Provence", latitude: 43.5297, longitude: 5.4474, timezone: "Europe/Paris" },
  { name: "Amiens", latitude: 49.8941, longitude: 2.2958, timezone: "Europe/Paris" },
  { name: "Amsterdam", latitude: 52.3676, longitude: 4.9041, timezone: "Europe/Amsterdam" },
  { name: "Angers", latitude: 47.4784, longitude: -0.5632, timezone: "Europe/Paris" },
  { name: "Annecy", latitude: 45.8992, longitude: 6.1294, timezone: "Europe/Paris" },
  { name: "Berlin", latitude: 52.52, longitude: 13.405, timezone: "Europe/Berlin" },
  { name: "Bordeaux", latitude: 44.8378, longitude: -0.5792, timezone: "Europe/Paris" },
  { name: "Brest", latitude: 48.3904, longitude: -4.4861, timezone: "Europe/Paris" },
  { name: "Bruxelles", latitude: 50.8503, longitude: 4.3517, timezone: "Europe/Paris" },
  { name: "Clermont-Ferrand", latitude: 45.7772, longitude: 3.087, timezone: "Europe/Paris" },
  { name: "Dijon", latitude: 47.322, longitude: 5.0415, timezone: "Europe/Paris" },
  { name: "Genève", latitude: 46.2044, longitude: 6.1432, timezone: "Europe/Paris" },
  { name: "Grenoble", latitude: 45.1885, longitude: 5.7245, timezone: "Europe/Paris" },
  { name: "La Rochelle", latitude: 46.1603, longitude: -1.1511, timezone: "Europe/Paris" },
  { name: "Lille", latitude: 50.6292, longitude: 3.0573, timezone: "Europe/Paris" },
  { name: "Limoges", latitude: 45.8336, longitude: 1.2611, timezone: "Europe/Paris" },
  { name: "Lisbonne", latitude: 38.7223, longitude: -9.1393, timezone: "Europe/Lisbon" },
  { name: "Lyon", latitude: 45.764, longitude: 4.8357, timezone: "Europe/Paris" },
  { name: "Madrid", latitude: 40.4168, longitude: -3.7038, timezone: "Europe/Madrid" },
  { name: "Marseille", latitude: 43.2965, longitude: 5.3698, timezone: "Europe/Paris" },
  { name: "Metz", latitude: 49.1193, longitude: 6.1757, timezone: "Europe/Paris" },
  { name: "Milan", latitude: 45.4642, longitude: 9.19, timezone: "Europe/Rome" },
  { name: "Montpellier", latitude: 43.6108, longitude: 3.8767, timezone: "Europe/Paris" },
  { name: "Nancy", latitude: 48.6921, longitude: 6.1844, timezone: "Europe/Paris" },
  { name: "Nantes", latitude: 47.2184, longitude: -1.5536, timezone: "Europe/Paris" },
  { name: "Nice", latitude: 43.7102, longitude: 7.262, timezone: "Europe/Paris" },
  { name: "Oslo", latitude: 59.9139, longitude: 10.7522, timezone: "Europe/Oslo" },
  { name: "Paris", latitude: 48.8566, longitude: 2.3522, timezone: "Europe/Paris" },
  { name: "Prague", latitude: 50.0755, longitude: 14.4378, timezone: "Europe/Prague" },
  { name: "Reims", latitude: 49.2583, longitude: 4.0317, timezone: "Europe/Paris" },
  { name: "Rennes", latitude: 48.1173, longitude: -1.6778, timezone: "Europe/Paris" },
  { name: "Rome", latitude: 41.9028, longitude: 12.4964, timezone: "Europe/Rome" },
  { name: "Rouen", latitude: 49.4431, longitude: 1.0993, timezone: "Europe/Paris" },
  { name: "Strasbourg", latitude: 48.5734, longitude: 7.7521, timezone: "Europe/Paris" },
  { name: "Stockholm", latitude: 59.3293, longitude: 18.0686, timezone: "Europe/Stockholm" },
  { name: "Toulouse", latitude: 43.6047, longitude: 1.4442, timezone: "Europe/Paris" },
  { name: "Londres", latitude: 51.5072, longitude: -0.1276, timezone: "Europe/London" },
  { name: "Vienne", latitude: 48.2082, longitude: 16.3738, timezone: "Europe/Vienna" },
  { name: "Varsovie", latitude: 52.2297, longitude: 21.0122, timezone: "Europe/Warsaw" },
  { name: "Zurich", latitude: 47.3769, longitude: 8.5417, timezone: "Europe/Zurich" },
  { name: "New York", latitude: 40.7128, longitude: -74.006, timezone: "America/New_York" },
  { name: "Los Angeles", latitude: 34.0522, longitude: -118.2437, timezone: "America/Los_Angeles" },
  { name: "Tokyo", latitude: 35.6762, longitude: 139.6503, timezone: "Asia/Tokyo" },
  { name: "Sydney", latitude: -33.8688, longitude: 151.2093, timezone: "Australia/Sydney" }
];
let activeCityIndex = -1;
let visibleCities = [];
let chosenCityName = "";

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

function equationOfTimeMinutes(date) {
  const centuries = (julianDay(date) - 2451545) / 36525;
  const meanLongitude = (280.46646 + 36000.76983 * centuries + 0.0003032 * centuries ** 2) * radians;
  const meanAnomaly = (357.52911 + 35999.05029 * centuries - 0.0001537 * centuries ** 2) * radians;
  const eccentricity = 0.016708634 - 0.000042037 * centuries - 0.0000001267 * centuries ** 2;
  const omega = (125.04 - 1934.136 * centuries) * radians;
  const obliquity = (23.43929111 - 0.013004167 * centuries - 0.000000164 * centuries ** 2
    + 0.000000504 * centuries ** 3 + 0.00256 * Math.cos(omega)) * radians;
  const y = Math.tan(obliquity / 2) ** 2;
  const equation = y * Math.sin(2 * meanLongitude) - 2 * eccentricity * Math.sin(meanAnomaly)
    + 4 * eccentricity * y * Math.sin(meanAnomaly) * Math.cos(2 * meanLongitude)
    - 0.5 * y ** 2 * Math.sin(4 * meanLongitude)
    - 1.25 * eccentricity ** 2 * Math.sin(2 * meanAnomaly);
  return equation * degrees * 4;
}

function timezoneOffsetMinutes(date, timezone) {
  const instant = new Date(Date.UTC(date.year, date.month - 1, date.day, 12));
  const parts = new Intl.DateTimeFormat("en", {
    timeZone: timezone,
    year: "numeric",
    month: "2-digit",
    day: "2-digit",
    hour: "2-digit",
    minute: "2-digit",
    second: "2-digit",
    hourCycle: "h23"
  }).formatToParts(instant).reduce((values, part) => {
    if (part.type !== "literal") values[part.type] = Number(part.value);
    return values;
  }, {});
  const localAsUtc = Date.UTC(parts.year, parts.month - 1, parts.day, parts.hour, parts.minute, parts.second);
  return (localAsUtc - instant.getTime()) / 60000;
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

function formatClockTime(hour) {
  const minutesTotal = Math.round((((hour % 24) + 24) % 24) * 60) % 1440;
  return `${String(Math.floor(minutesTotal / 60)).padStart(2, "0")}:${String(minutesTotal % 60).padStart(2, "0")}`;
}

function estimateSolarEvents(date, latitude, longitude, declination, threshold, timezone) {
  const phi = latitude * radians;
  const delta = declination * radians;
  const altitude = threshold * radians;
  const cosHourAngle = (Math.sin(altitude) - Math.sin(phi) * Math.sin(delta))
    / (Math.cos(phi) * Math.cos(delta));
  const hourAngleDegrees = Math.acos(Math.max(-1, Math.min(1, cosHourAngle))) * degrees;
  const civilSolarNoonMinutes = 720 - equationOfTimeMinutes(date) - 4 * longitude
    + timezoneOffsetMinutes(date, timezone);
  return {
    sunrise: (civilSolarNoonMinutes - 4 * hourAngleDegrees) / 60,
    sunset: (civilSolarNoonMinutes + 4 * hourAngleDegrees) / 60
  };
}

function formatLatitude(latitude) {
  const hemisphere = latitude < 0 ? "S" : "N";
  const absolute = Math.abs(latitude).toLocaleString("fr-FR", { minimumFractionDigits: 2, maximumFractionDigits: 4 });
  return `${absolute}° ${hemisphere}`;
}

function normalizeText(text) {
  return text.normalize("NFD").replace(/[\u0300-\u036f]/g, "").toLocaleLowerCase("fr");
}

function closeCitySuggestions() {
  elements["city-suggestions"].hidden = true;
  elements["city-input"].setAttribute("aria-expanded", "false");
  elements["city-input"].removeAttribute("aria-activedescendant");
  activeCityIndex = -1;
}

function renderCitySuggestions() {
  const query = normalizeText(elements["city-input"].value.trim());
  visibleCities = cities.filter((city) => normalizeText(city.name).includes(query)).slice(0, 8);
  const list = elements["city-suggestions"];
  list.replaceChildren();
  visibleCities.forEach((city, index) => {
    const option = document.createElement("li");
    option.id = `city-option-${index}`;
    option.setAttribute("role", "option");
    option.setAttribute("aria-selected", String(index === activeCityIndex));
    const button = document.createElement("button");
    button.type = "button";
    button.textContent = city.name;
    option.append(button);
    list.append(option);
  });
  list.hidden = visibleCities.length === 0;
  elements["city-input"].setAttribute("aria-expanded", String(!list.hidden));
}

function selectCity(city) {
  chosenCityName = city.name;
  elements["city-input"].value = city.name;
  elements["latitude-input"].value = city.latitude.toFixed(4);
  elements["latitude-range"].value = String(city.latitude);
  elements["longitude-input"].value = city.longitude.toFixed(4);
  elements["timezone-select"].value = city.timezone;
  closeCitySuggestions();
  update();
}

function switchToCustomLocation() {
  chosenCityName = "";
  elements["city-input"].value = "";
}

function timezoneLabel(timezone) {
  return timezone === "Europe/Paris" ? "CET/CEST" : timezone;
}

function persistSettings() {
  const settings = {
    date: elements["date-input"].value,
    city: chosenCityName,
    latitude: elements["latitude-input"].value,
    longitude: elements["longitude-input"].value,
    model: elements["model-select"].value,
    timezone: elements["timezone-select"].value,
    threshold: elements["threshold-input"].value
  };
  try {
    localStorage.setItem(SETTINGS_STORAGE_KEY, JSON.stringify(settings));
  } catch {
    setStatus("Le navigateur ne permet pas de mémoriser les réglages.", true);
  }
}

function restoreSettings() {
  let settings;
  try {
    settings = JSON.parse(localStorage.getItem(SETTINGS_STORAGE_KEY));
  } catch {
    return;
  }
  if (!settings || typeof settings !== "object") return;

  const savedDate = parseDate(settings.date);
  if (savedDate && savedDate.year >= 1900 && savedDate.year <= 3000) {
    elements["date-input"].value = settings.date;
  }
  if (Number.isFinite(Number(settings.latitude)) && Number(settings.latitude) >= -90 && Number(settings.latitude) <= 90) {
    elements["latitude-input"].value = String(settings.latitude);
    elements["latitude-range"].value = String(settings.latitude);
  }
  if (Number.isFinite(Number(settings.longitude)) && Number(settings.longitude) >= -180 && Number(settings.longitude) <= 180) {
    elements["longitude-input"].value = String(settings.longitude);
  }
  if (["meeus", "spencer", "sinusoidal"].includes(settings.model)) {
    elements["model-select"].value = settings.model;
  }
  if ([...elements["timezone-select"].options].some((option) => option.value === settings.timezone)) {
    elements["timezone-select"].value = settings.timezone;
  }
  if (Number.isFinite(Number(settings.threshold)) && Number(settings.threshold) >= -90 && Number(settings.threshold) <= 90) {
    elements["threshold-input"].value = String(settings.threshold);
  }
  if (cities.some((city) => city.name === settings.city)) {
    chosenCityName = settings.city;
    elements["city-input"].value = settings.city;
  }
}

function resetSettings() {
  try {
    localStorage.removeItem(SETTINGS_STORAGE_KEY);
  } catch {
    setStatus("Le navigateur ne permet pas de supprimer les réglages mémorisés.", true);
  }
  chosenCityName = "";
  elements["date-input"].value = localDateString(new Date());
  elements["city-input"].value = "";
  elements["latitude-input"].value = "47.2967";
  elements["latitude-range"].value = "47.3";
  elements["longitude-input"].value = "0";
  elements["model-select"].value = "meeus";
  elements["timezone-select"].value = "Europe/Paris";
  elements["threshold-input"].value = "-0.833";
  update();
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
  const longitudeText = elements["longitude-input"].value.trim();
  const thresholdText = elements["threshold-input"].value.trim();
  const latitude = Number(latitudeText);
  const longitude = Number(longitudeText);
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
  if (!longitudeText || !Number.isFinite(longitude) || longitude < -180 || longitude > 180) {
    elements["longitude-input"].setAttribute("aria-invalid", "true");
    setStatus("La longitude doit être comprise entre −180° et 180°.", true);
    return;
  }
  elements["longitude-input"].removeAttribute("aria-invalid");
  if (!thresholdText || !Number.isFinite(threshold) || threshold < -90 || threshold > 90) {
    elements["threshold-input"].setAttribute("aria-invalid", "true");
    setStatus("Le seuil doit être compris entre −90° et 90°.", true);
    return;
  }
  elements["threshold-input"].removeAttribute("aria-invalid");

  const model = elements["model-select"].value;
  const timezone = elements["timezone-select"].value;
  const declination = declinationFor(date, model);
  const duration = durationHours(latitude, declination, threshold);
  const events = duration === 0 || duration === 24
    ? null
    : estimateSolarEvents(date, latitude, longitude, declination, threshold, timezone);
  const dateLabel = new Intl.DateTimeFormat("fr-FR", { dateStyle: "full" }).format(date.date);
  elements["result-date"].textContent = dateLabel;
  elements["duration-value"].textContent = formatDuration(duration);
  elements["declination-value"].textContent = `${declination >= 0 ? "+" : "−"}${Math.abs(declination).toFixed(2)}°`;
  elements["model-chip"].textContent = model === "meeus" ? "MEEUS" : model === "spencer" ? "SPENCER" : "SINUSOÏDAL";
  elements["latitude-display"].textContent = `${chosenCityName || "Position personnalisée"} · ${timezoneLabel(timezone)}`;
  elements["sunrise-value"].textContent = duration === 0 ? "Nuit polaire" : duration === 24 ? "Jour polaire" : formatClockTime(events.sunrise);
  elements["sunset-value"].textContent = duration === 0 ? "Nuit polaire" : duration === 24 ? "Jour polaire" : formatClockTime(events.sunset);
  setStatus("");
  persistSettings();
  drawChart(latitude, declination, threshold);
}

elements["date-input"].value = localDateString(new Date());
restoreSettings();
update();
elements["today-button"].addEventListener("click", () => {
  elements["date-input"].value = localDateString(new Date());
  update();
});
elements["date-input"].addEventListener("input", update);
elements["settings-reset"].addEventListener("click", resetSettings);
elements["model-select"].addEventListener("change", update);
elements["timezone-select"].addEventListener("change", update);
elements["city-input"].addEventListener("focus", renderCitySuggestions);
elements["city-input"].addEventListener("input", () => {
  chosenCityName = "";
  activeCityIndex = -1;
  renderCitySuggestions();
  update();
});
elements["city-input"].addEventListener("keydown", (event) => {
  if (event.key === "Escape") {
    closeCitySuggestions();
  } else if (event.key === "ArrowDown" && !elements["city-suggestions"].hidden) {
    event.preventDefault();
    activeCityIndex = (activeCityIndex + 1) % visibleCities.length;
    renderCitySuggestions();
    elements["city-input"].setAttribute("aria-activedescendant", `city-option-${activeCityIndex}`);
  } else if (event.key === "ArrowUp" && !elements["city-suggestions"].hidden) {
    event.preventDefault();
    activeCityIndex = (activeCityIndex - 1 + visibleCities.length) % visibleCities.length;
    renderCitySuggestions();
    elements["city-input"].setAttribute("aria-activedescendant", `city-option-${activeCityIndex}`);
  } else if (event.key === "Enter" && !elements["city-suggestions"].hidden && visibleCities.length) {
    event.preventDefault();
    selectCity(visibleCities[Math.max(activeCityIndex, 0)]);
  }
});
elements["city-suggestions"].addEventListener("pointerdown", (event) => event.preventDefault());
elements["city-suggestions"].addEventListener("click", (event) => {
  const option = event.target.closest('[role="option"]');
  if (option) selectCity(visibleCities[Number(option.id.replace("city-option-", ""))]);
});
elements["threshold-input"].addEventListener("input", update);
elements["threshold-reset"].addEventListener("click", () => {
  elements["threshold-input"].value = "-0.833";
  update();
});
elements["latitude-input"].addEventListener("input", () => {
  switchToCustomLocation();
  const text = elements["latitude-input"].value.trim();
  const latitude = Number(text);
  if (text && Number.isFinite(latitude) && latitude >= -90 && latitude <= 90) {
    elements["latitude-range"].value = String(latitude);
  }
  update();
});
elements["longitude-input"].addEventListener("input", () => {
  switchToCustomLocation();
  update();
});
elements["latitude-range"].addEventListener("input", () => {
  switchToCustomLocation();
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
      switchToCustomLocation();
      elements["latitude-input"].value = latitude.toFixed(4);
      elements["latitude-range"].value = String(Math.max(-90, Math.min(90, latitude)));
      elements["longitude-input"].value = position.coords.longitude.toFixed(4);
      elements["city-input"].value = "";
      update();
    },
    () => setStatus("Position indisponible. Saisissez la latitude ou réglez le curseur.", true),
    { enableHighAccuracy: false, timeout: 8000, maximumAge: 300000 }
  );
});
new ResizeObserver(update).observe(elements["solar-chart"]);