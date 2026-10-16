// --- Fichier : date.h ---

#ifndef DATE_H
#define DATE_H

#include <stdbool.h>

typedef struct {
	int jour;
	int mois;
	int annee;
} CalendarDate;

typedef struct {
	CalendarDate debut;
	CalendarDate fin;
} DateRange;

int is_leap_year(int annee);
int date_to_ordinal(CalendarDate date);
void advance_day(CalendarDate *date);
bool is_date_valid(CalendarDate date);
bool is_date_range_ordered(DateRange range);

#endif // DATE_H
