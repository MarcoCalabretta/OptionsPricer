#include "constants.h"
#include "date.h"
#include <assert.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// see options_chart.h for documentation
static const char *tempname = "optemp.txt";

// it's like fgetc(fp), but doesn't move the file pointer
static char fgetc_inplace(FILE *fp) {
  assert(fp);
  fpos_t p;
  fgetpos(fp, &p);
  char c = fgetc(fp);
  fsetpos(fp, &p);
  return c;
}

struct options_chart {
  int len;
  int max_len;
  int cur_option;
  const char *ticker;
  struct option **ops;
};

struct option {
  const char *ticker;
  struct date *expiry_date;
  double strike_price;
  double market_price;
  int volume;
  bool call;
  bool american;
};

struct options_chart *options_chart_create(const char *ticker) {
  assert(ticker);
  struct options_chart *oc = malloc(sizeof(struct options_chart));
  oc->len = 0;
  oc->max_len = 1;
  oc->cur_option = 0;
  oc->ticker = ticker;

  oc->ops = malloc(oc->max_len * sizeof(struct option *));

  // download shit into file
  bool american = true;
  char command[100];
  strcpy(command, "curl -s -o ");
  strcat(command, tempname);
  strcat(command, " https://www.marketwatch.com/investing/stock/");
  strcat(command, ticker);
  strcat(command, "/options");
  system((const char *)command);

  FILE *fp = fopen(tempname, "r");
  bool file_ok = true;
  if (!fp)
    file_ok = false;
  else {
    fgetc(fp);
    if (feof(fp))
      file_ok = false;
  }
  // if there's no file, we try building the url again with "fund" instead of
  // "stock"
  if (!file_ok) {
    // generally fund options are european, this is just a guess
    american = false;
    fclose(fp);
    remove(tempname);
    strcpy(command, "curl -s -o ");
    strcat(command, tempname);
    strcat(command, " https://www.marketwatch.com/investing/fund/");
    strcat(command, ticker);
    strcat(command, "/options");
    system(command);

    fp = fopen(tempname, "r");
    file_ok = true;
    if (!fp)
      file_ok = false;
    else {
      fgetc(fp);
      if (feof(fp))
        file_ok = false;
    }
  }
  if (!file_ok)
    return NULL;

  // searches for the line immediately before the options chart
  const int MAX_LINE_LEN = 200;
  char line[MAX_LINE_LEN];
  const char *start_key = "                <div class=\"overflow--table\">";
  int start_keylen = strlen(start_key);
  int linelen = 0;
  bool diff = 0;
  bool found = 0;
  while (!found && !feof(fp)) {
    fgets(line, MAX_LINE_LEN, fp);
    diff = 0;
    linelen = strlen((const char *)line);
    for (int i = 0; i < start_keylen && i < linelen; i++) {
      if (line[i] != start_key[i]) {
        diff = 1;
        break;
      }
    }
    if (!diff) {
      found = 1;
    }
  }

  // counts how many options there are
  const char *new_option_key = "                                               "
                               "         <div class=\"option__cell strike\">";
  const char *date_key = "                                        <th "
                         "colspan=\"3\"><div class=\"option__heading\"><span "
                         "class=\"text\">Expires ";
  // data to make the expiry date
  char mon[4] = "Aug";
  int month, day, year;
  fpos_t position;

  int new_option_keylen = strlen(new_option_key);
  int date_keylen = strlen(date_key);
  bool new_option_diff = 0;
  bool date_diff = 0;
  double strike_price;
  double market_price;
  int volume;
  while (!feof(fp)) {
    fgetpos(fp, &position);
    fgets(line, MAX_LINE_LEN, fp);
    new_option_diff = 0;
    date_diff = 0;
    linelen = strlen((const char *)line);
    for (int i = 0; i < new_option_keylen && i < linelen; i++) {
      if (line[i] != new_option_key[i]) {
        new_option_diff = 1;
        break;
      }
    }
    for (int i = 0; i < date_keylen && i < linelen; i++) {
      if (line[i] != date_key[i]) {
        date_diff = 1;
        break;
      }
    }
    if (!new_option_diff) {
      bool blank_option = false;
      struct option *o;
      oc->len += 2;
      while (oc->len >= oc->max_len) {
        oc->max_len *= 2;
        oc->ops = realloc(oc->ops, oc->max_len * sizeof(struct option *));
      }
      strike_price = strtod((const char *)(line + new_option_keylen), NULL);

      // fills in call option
      // NOTE: this is completely dependent on how marketwatch structures their
      // html
      for (int i = 0; i < 4; i++) {
        while (fgetc(fp) != '\n')
          ;
      }
      for (int i = 0; i < 2; i++) {
        while (fgetc(fp) != '>')
          ;
      }
      blank_option = (fgetc_inplace(fp) == '<');
      fscanf(fp, "%lf", &market_price);
      for (int i = 0; i < 1; i++) {
        while (fgetc(fp) != '\n')
          ;
      }
      for (int i = 0; i < 2; i++) {
        while (fgetc(fp) != '>')
          ;
      }
      fscanf(fp, "%d", &volume);
      if (!blank_option) {
        (oc->ops)[oc->len - 2] = malloc(sizeof(struct option));
        o = (oc->ops)[oc->len - 2];
        o->american = american;
        o->call = true;
        o->expiry_date = date_create(day, month, year);
        o->strike_price = strike_price;
        o->market_price = market_price;
        o->ticker = ticker;
        o->volume = volume;
      } else
        oc->len--;

      // fills in put option
      // NOTE: this is completely dependent on how marketwatch structures their
      // html
      for (int i = 0; i < 6; i++) {
        while (fgetc(fp) != '\n')
          ;
      }
      for (int i = 0; i < 2; i++) {
        while (fgetc(fp) != '>')
          ;
      }
      blank_option = (fgetc_inplace(fp) == '<');
      fscanf(fp, "%lf", &market_price);
      for (int i = 0; i < 1; i++) {
        while (fgetc(fp) != '\n')
          ;
      }
      for (int i = 0; i < 2; i++) {
        while (fgetc(fp) != '>')
          ;
      }
      fscanf(fp, "%d", &volume);
      if (!blank_option) {
        (oc->ops)[oc->len - 1] = malloc(sizeof(struct option));
        o = (oc->ops)[oc->len - 1];
        o->american = american;
        o->call = false;
        o->expiry_date = date_create(day, month, year);
        o->strike_price = strike_price;
        o->market_price = market_price;
        o->ticker = ticker;
        o->volume = volume;
      } else
        oc->len--;
    } else if (!date_diff) {
      fsetpos(fp, &position);
      for (int i = 0; i < 3; i++) {
        while (fgetc(fp) != '>')
          ;
      }
      while (fgetc(fp) != ' ')
        ;
      fscanf(fp, "%s", mon);
      fscanf(fp, " %d", &day);
      fscanf(fp, ", %d", &year);
      if (!strcmp((const char *)mon, "Jan"))
        month = 1;
      else if (!strcmp((const char *)mon, "Feb"))
        month = 2;
      else if (!strcmp((const char *)mon, "Mar"))
        month = 3;
      else if (!strcmp((const char *)mon, "Apr"))
        month = 4;
      else if (!strcmp((const char *)mon, "May"))
        month = 5;
      else if (!strcmp((const char *)mon, "Jun"))
        month = 6;
      else if (!strcmp((const char *)mon, "Jul"))
        month = 7;
      else if (!strcmp((const char *)mon, "Aug"))
        month = 8;
      else if (!strcmp((const char *)mon, "Sep"))
        month = 9;
      else if (!strcmp((const char *)mon, "Oct"))
        month = 10;
      else if (!strcmp((const char *)mon, "Nov"))
        month = 11;
      else if (!strcmp((const char *)mon, "Dec"))
        month = 12;
    }
  }
  fclose(fp);
  remove(tempname);
  return oc;
}

void options_chart_destroy(struct options_chart *oc) {
  assert(oc);
  for (int i = 0; i < oc->len; i++)
    free((oc->ops)[i]);
  free(oc->ops);
  free(oc);
}

struct option *option_create() {
  struct option *o = malloc(sizeof(struct option));
  o->ticker = NULL;
  o->expiry_date = current_date();
  o->strike_price = 0;
  o->market_price = 0;
  o->volume = 0;
  o->call = false;
  o->american = false;
  return o;
}

void option_destroy(struct option *o) {
  assert(o);
  date_destroy(o->expiry_date);
  free(o);
}

bool options_left(const struct options_chart *oc) {
  assert(oc);
  return (oc->cur_option < oc->len);
}

void options_next(struct options_chart *oc, struct option *o) {
  assert(oc);
  assert(o);
  struct option *cur = (oc->ops)[oc->cur_option];
  o->ticker = (cur)->ticker;
  o->strike_price = (cur)->strike_price;
  o->market_price = (cur)->market_price;
  o->volume = (cur)->volume;
  o->call = (cur)->call;
  o->american = (cur)->american;
  o->expiry_date = date_copy(o->expiry_date, cur->expiry_date);
  (oc->cur_option)++;
}

double get_market_price(const struct option *o) {
  assert(o);
  return o->market_price;
}

double get_strike_price(const struct option *o) {
  assert(o);
  return o->strike_price;
}

int get_volume(const struct option *o) {
  assert(o);
  return o->volume;
}

const char *get_ticker(const struct option *o) {
  assert(o);
  return o->ticker;
}

const struct date *get_expiry_date(const struct option *o) {
  assert(o);
  return o->expiry_date;
}

bool is_call(const struct option *o) {
  assert(o);
  return o->call;
}

bool is_american(const struct option *o) {
  assert(o);
  return o->american;
}
