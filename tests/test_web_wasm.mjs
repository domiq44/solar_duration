import assert from "node:assert/strict";
import createSolarCore from "../web/solar_core.mjs";

const solar = await createSolarCore();
const near = (actual, expected, tolerance, label) => {
  assert.ok(Math.abs(actual - expected) <= tolerance,
    `${label}: expected ${expected}, received ${actual}`);
};

const meeusMode = 3;
const referenceDeclination = solar._solar_web_declination(meeusMode, 1992, 10, 13);
near(referenceDeclination, -7.97, 0.02, "Meeus reference declination");

const equatorialDuration = solar._solar_web_day_duration(
  meeusMode, 2024, 3, 20, 0, 0);
near(equatorialDuration, 12, 1e-9, "equatorial equinox duration");

const polarDay = solar._solar_web_day_duration(meeusMode, 2025, 6, 21, 80, -0.833);
assert.equal(polarDay, 24, "polar day duration");
const polarDeclination = solar._solar_web_declination(meeusMode, 2025, 6, 21);
const missingPolarSunrise = solar._solar_web_event_hour(
  2025, 6, 21, 80, 0, polarDeclination, -0.833, 0, 1);
assert.ok(Number.isNaN(missingPolarSunrise),
  "polar day has no fabricated sunrise time");

near(solar._solar_web_altitude(12, 0, 0), 90, 1e-9,
  "equatorial noon solar altitude");
assert.ok(Number.isNaN(solar._solar_web_altitude(25, 0, 0)),
  "invalid solar altitude hour is rejected");

const sunriseUtc = solar._solar_web_event_hour(
  2024, 3, 20, 0, 0, 0, 0, 0, 1);
const sunriseLocal = solar._solar_web_event_hour(
  2024, 3, 20, 0, 0, 0, 0, 120, 1);
near(sunriseLocal - sunriseUtc, 2, 1e-9, "timezone-adjusted sunrise");

assert.ok(Number.isNaN(solar._solar_web_declination(meeusMode, 2024, 2, 30)),
  "invalid calendar date is rejected");
assert.ok(Number.isNaN(solar._solar_web_event_hour(
  2024, 3, 20, 48, 181, 0, -0.833, 60, 1)),
  "invalid event longitude is rejected");
assert.ok(Number.isNaN(solar._solar_web_event_hour(
  2024, 3, 20, 48, 2, 0, -0.833, Number.NaN, 1)),
  "non-finite timezone offset is rejected");

console.log("WebAssembly solar API: 10 checks passed");