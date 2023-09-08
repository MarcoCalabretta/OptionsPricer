#include "date.h"
#include <assert.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

// documentation in date.h

const int DAYS_PER_YEAR = 365;
const double YEARS_PER_DAY = 1.0 / 365;
const int DATE_LENGTH = 11;

struct date {
  int total_day;
  int day;
  int month;
  int year;
};

struct date *date_create(int day, int month, int year) {
  assert(day >= 1);
  assert(day <= 31);
  assert(month >= 1);
  assert(month <= 12);
  assert(year >= 1900);
  assert(year <= 2100);
  struct date *d = malloc(sizeof(struct date));
  d->total_day = 0;
  d->day = day;
  d->month = month;
  d->year = year;
  d->total_day += year * DAYS_PER_YEAR;
  int months[12] = {31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31};
  for (int i = 1; i < month; i++) {
    d->total_day += months[i - 1];
  }
  d->total_day += day;

  // slighter additions
  // leap years
  d->total_day += year / 4;
  return d;
}

struct date *current_date() {
  time_t t;
  time(&t);
  struct tm *date = gmtime(&t);
  int day = date->tm_mday;
  int month = date->tm_mon + 1;
  int year = date->tm_year + 1900;
  return date_create(day, month, year);
}

struct date *string_to_date(const char *s) {
  assert(s);
  char new_s[DATE_LENGTH];
  strcpy(new_s, s);
  int day = atoi(new_s + 8);
  new_s[7] = '\0';
  int month = atoi(new_s + 5);
  new_s[4] = '\0';
  int year = atoi(new_s);
  return date_create(day, month, year);
}

void date_destroy(struct date *d) {
  assert(d);
  free(d);
}

struct date *date_copy(struct date *dest, const struct date *src) {
  assert(src);
  if (dest) {
    date_destroy(dest);
    dest = NULL;
  }
  dest = date_create(src->day, src->month, src->year);
  return dest;
}

int date_compare(const struct date *d1, const struct date *d2) {
  assert(d1);
  assert(d2);
  return d1->total_day - d2->total_day;
}

void date_string(const struct date *d, char *s) {
  assert(d);
  assert(s);
  int year = d->year;
  int month = d->month;
  int day = d->day;
  s[10] = '\0';
  s[9] = '0' + day % 10;
  day /= 10;
  s[8] = '0' + day;
  s[7] = '-';
  s[6] = '0' + month % 10;
  month /= 10;
  s[5] = '0' + month;
  s[4] = '-';
  s[3] = '0' + year % 10;
  year /= 10;
  s[2] = '0' + year % 10;
  year /= 10;
  s[1] = '0' + year % 10;
  year /= 10;
  s[0] = '0' + year;
}

bool future(struct date *d) {
  struct date *today = current_date();
  int diff = date_compare(d, today);
  free(today);
  return diff > 0;
}
