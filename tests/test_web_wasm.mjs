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

near(solar._solar_web_altitude(12, 0, 0), 90, 1e-9,
  "equatorial noon solar altitude");

const sunriseUtc = solar._solar_web_event_hour(
  2024, 3, 20, 0, 0, 0, 0, 0, 1);
const sunriseLocal = solar._solar_web_event_hour(
  2024, 3, 20, 0, 0, 0, 0, 120, 1);
near(sunriseLocal - sunriseUtc, 2, 1e-9, "timezone-adjusted sunrise");

assert.ok(Number.isNaN(solar._solar_web_declination(meeusMode, 2024, 2, 30)),
  "invalid calendar date is rejected");

console.log("WebAssembly solar API: 6 checks passed");