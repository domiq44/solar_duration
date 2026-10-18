#ifndef WEB_API_H
#define WEB_API_H

/* Return NAN for invalid inputs or dates outside the supported year range. */
double solar_web_declination(int model, int year, int month, int day);
double solar_web_day_duration(int model, int year, int month, int day,
                              double latitude_degrees,
                              double altitude_threshold_degrees);

/* Return NAN unless hour is in [0, 24] and angles are finite and valid. */
double solar_web_altitude(double hour, double latitude_degrees,
                          double declination_degrees);

/* Return NAN for invalid inputs or when the threshold is never crossed. */
double solar_web_event_hour(int year, int month, int day,
                            double latitude_degrees, double longitude_degrees,
                            double declination_degrees,
                            double altitude_threshold_degrees,
                            double timezone_offset_minutes, int is_sunrise);

#endif