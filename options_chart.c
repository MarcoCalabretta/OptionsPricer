#include "constants.h"
#include "date.h"
#include <assert.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// see options_chart.h for documentation
static const char *tempname = "temp.txt";

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
  bool call;
  bool american;
};

// creates an options chart for ticker and returns a pointer to it
// effects: allocates heap memory, client must call
// options_destroy
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
  strcpy(command, "curl -o ");
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
    strcpy(command, "curl -o ");
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
  fpos_t position;
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
      fgetpos(fp, &position);
    }
  }

  // counts how many options there are
  const char *new_option_key = "                                               "
                               "         <div class=\"option__cell strike\">";
  const char *date_key = "                                        <th "
                         "colspan=\" 3 \"><div class=\"option__heading\"><span "
                         "class=\"text\">Expires ";
  // data to make the expiry date
  char mon[3];
  int month, day, year;

  int new_option_keylen = strlen(new_option_key);
  int date_keylen = strlen(date_key);
  bool new_option_diff = 0;
  bool date_diff = 0;
  double strike_price;
  double market_price;
  while (!feof(fp)) {
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
      oc->len += 2;
      while (oc->len >= oc->max_len) {
        oc->max_len *= 2;
        oc->ops = realloc(oc->ops, oc->max_len * sizeof(struct option *));
      }
      // TODO find data about options
      scanf("%lf", &strike_price);

      // fills in call option
      // NOTE: this is completely dependent on how marketwatch structures their
      // html
      for (int i = 0; i < 4; i++) {
        while (fgetc(fp) != '>')
          ;
      }
      scanf("%lf", &market_price);
      (oc->ops)[oc->len - 2] = malloc(sizeof(struct option));
      struct option *o = (oc->ops)[oc->len - 2];
      o->american = american;
      o->call = true;
      o->expiry_date = date_create(day, month, year);
      o->strike_price = strike_price;
      o->market_price = market_price;
      o->ticker = ticker;

      // fills in put option
      // NOTE: this is completely dependent on how marketwatch structures their
      // html
      for (int i = 0; i < 7; i++) {
        while (fgetc(fp) != '\n')
          ;
      }
      for (int i = 0; i < 2; i++) {
        while (fgetc(fp) != '>')
          ;
      }
      scanf("%lf", &market_price);
      (oc->ops)[oc->len - 1] = malloc(sizeof(struct option));
      o = (oc->ops)[oc->len - 2];
      o->american = american;
      o->call = false;
      o->expiry_date = date_create(day, month, year);
      o->strike_price = strike_price;
      o->market_price = market_price;
      o->ticker = ticker;

    } else if (!date_diff) {
      scanf("%s", mon);
      scanf(" %d", day);
      scanf(", %d", year);
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
  fsetpos(fp, &position);

  fclose(fp);

  return oc;
}

// destroys all memory associated with oc
void options_chart_destroy(struct options_chart *oc) {
  assert(oc);
  for (int i = 0; i < oc->len; i++)
    free((oc->ops)[i]);
  free(oc->ops);
  free(oc);
}

// creates an option and returns an option to it
// effects: allocates heap memory, client must call option_destroy
struct option *option_create() {
  struct option *o = malloc(sizeof(struct option));
  o->ticker = NULL;
  o->expiry_date = current_date();
  o->strike_price = 0;
  o->market_price = 0;
  o->call = false;
  o->american = false;
  return o;
}

// destroys all memory associated with 0
void option_destroy(struct option *o) {
  assert(o);
  date_destroy(o->expiry_date);
  free(o);
}

// returns true if there are still options left in the chart, false otherwise
bool options_left(struct options_chart *oc) {
  assert(oc);
  return (oc->cur_option < oc->len);
}

// reads the next option from oc and assigns the value to o
void options_next(struct options_chart *oc, struct option *o) {
  assert(oc);
  assert(o);
  struct option *cur = (oc->ops)[oc->cur_option];
  o->ticker = (cur)->ticker;
  o->strike_price = (cur)->strike_price;
  o->market_price = (cur)->market_price;
  o->call = (cur)->call;
  o->american = (cur)->american;
  date_copy(o->expiry_date, cur->expiry_date);
}

double get_market_price(struct option *o) {
  assert(o);
  return o->market_price;
}

// returns the strike price of o
double get_strike_price(struct option *o) {
  assert(o);
  return o->strike_price;
}

// returns the ticker name of o
const char *get_ticker(struct option *o) {
  assert(o);
  return o->ticker;
}

// returns the expiry date of o
const struct date *get_expiry_date(struct option *o) {
  assert(o);
  return o->expiry_date;
}

// returns true if o is a call
bool is_call(struct option *o) {
  assert(o);
  return o->call;
}

// returns true if o is an american option
bool is_american(struct option *o) {
  assert(o);
  return o->american;
}
