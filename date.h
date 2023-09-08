#ifndef DATE_H
#define DATE_H
#include <stdbool.h>
// a set of functions regarding a date struct

// ALL FUNCTIONS REQUIRE VALID POINTERS

// a date struct that makes date comparisons easier
struct date;

extern const int DAYS_PER_YEAR;
extern const double YEARS_PER_DAY;
extern const int DATE_LENGTH;

// makes a date based on a day month year, and returns a pointer to the new date
// requires: 1 <= day <= 31
// 		1 <= month <= 12
// 		1900 <= year <= 2100
// effects: allocates heap memory, user must call date_destroy
// time: O(1)
struct date *date_create(int day, int month, int year);

// returns the pointer to a date sructure with a current date
// effects: allocates heap memory, user must call date_destroy
// time: O(1)
struct date *current_date();

// turns a date_string into a date struct
// requires: s is null-terminated in YYYY-MM-DD format (not asserted)
// effects: allocates heap memory, user must call date_destroy
// time: O(1)
struct date *string_to_date(const char *s);

// destroys d and all memory associated with it
// time: O(1)
void date_destroy(struct date *d);

// deep copies src to dest and returns the new pointer to dest
// NOTE: dest can be NULL
struct date *date_copy(struct date *dest, const struct date *src);

// returns the number of days between d1 and d2
// result will be negative if d1 is earlier than d2, in a sense it returns d1 -
// d2 time: O(1)
int date_compare(const struct date *d1, const struct date *d2);

// returns a string that stores the date in YYYY-MM-DD format
// requires: s has enough memory for DATE_LENGTH bytes (not asserted)
// time: O(1)
void date_string(const struct date *d, char *s);

// returns true if the given date is in the future, false if it is in the
// present or past
bool future(struct date *d);
#endif
