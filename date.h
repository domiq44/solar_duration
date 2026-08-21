// --- File: date.h ---

#ifndef DATE_H
#define DATE_H

#include <stdbool.h>

int is_leap_year(int annee);
int date_to_ordinal(int day, int month, int year);
void advance_day(int *jour, int *mois, int *annee);
bool is_date_valid(int day, int month, int year);

#endif // DATE_H
