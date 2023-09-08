#include "constants.h"
#include <assert.h>
#include <math.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
// see stock_prices.h for documentation

// the temporary file that we download the stock data into
static const char *tempname = "temp.txt";
static const char *pricename = "tempprice.txt";

// attempts to download the historical price data of a stock
static void ticker_download(const char *ticker) {
  assert(ticker);
  char command[1000];
  strcpy(command, "curl -o ");
  strcat(command, pricename);
  strcat(command, " \"https://query1.finance.yahoo.com/v7/finance/download/");
  strcat(command, ticker);
  strcat(command, "?period1=100000000&period2=1693872000&interval=1d&events="
                  "history&includeAdjustedClose=true\"");
  system((const char *)command);
}

// returns the current yield of ticker
static void cur_price_and_yield(const char *ticker, double *cur_yield,
                                double *cur_price) {
  assert(ticker);

  // builds url and downloads it
  remove(tempname);
  char command[150] = "curl -o ";
  strcat(command, tempname);
  strcat(command, " https://www.marketwatch.com/investing/stock/");
  strcat(command, ticker);
  system(command);

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
    fclose(fp);
    remove(tempname);
    strcpy(command, "curl -o ");
    strcat(command, tempname);
    strcat(command, " https://www.marketwatch.com/investing/fund/");
    strcat(command, ticker);
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
  // if it's not a fund or a stock we don't know what it is
  if (!file_ok)
    return;

  // searches for the line immediately before the dividend yield
  char line[100];
  const char *pkey = "        <meta name=\"price\" content=\"$";
  const char *ykey = "                    <small class=\"label\">Yield</small>";
  int keylen = strlen(pkey);
  int linelen = 0;
  bool diff = 0;
  bool found = 0;
  while (!found && !feof(fp)) {
    fgets(line, 100, fp);
    diff = 0;
    linelen = strlen((const char *)line);
    for (int i = 0; i < keylen && i < linelen; i++) {
      if (line[i] != pkey[i]) {
        diff = 1;
        break;
      }
    }
    if (!diff) {
      found = 1;
      *cur_price = strtod((const char *)(line + keylen), NULL);
    }
  }
  found = 0;
  keylen = strlen(ykey);
  while (!found && !feof(fp)) {
    fgets(line, 100, fp);
    diff = 0;
    linelen = strlen((const char *)line);
    for (int i = 0; i < keylen && i < linelen; i++) {
      if (line[i] != ykey[i]) {
        diff = 1;
        break;
      }
    }
    if (!diff) {
      found = 1;
      while (fgetc(fp) != '>')
        ;
      fscanf(fp, "%lf", cur_yield);
    }
  }

  fclose(fp);
  remove(tempname);
}

// a stock_prices struct that stores a lot of data about a stock's prices
struct stock_prices {
  const char *ticker;
  double div_yield;
  double cur_price;
  int len_prices;
  double *prices;
};

// creates a stock_prices struct with a particular ticker, returns the pointer
// to that struct ,or NULL if the ticker's data couldn't be downloaded ticker is
// lowercase, eg "aapl" for Apple
// effects: allocates heap memory, client must call stock_prices_destroy
struct stock_prices *stock_prices_create(const char *ticker) {
  assert(ticker);

  ticker_download(ticker);
  FILE *fp = fopen(pricename, "r");
  if (!fp) {
    return NULL;
  }

  // assigns the non-prices values to s
  struct stock_prices *s = malloc(sizeof(struct stock_prices));
  s->ticker = ticker;
  cur_price_and_yield(ticker, &(s->div_yield), &(s->cur_price));
  while (fgetc(fp) != '\n')
    ;
  fpos_t position;
  fgetpos(fp, &position);
  s->len_prices = 0;
  while (!feof(fp)) {
    if (fgetc(fp) == '\n')
      (s->len_prices)++;
  }
  fsetpos(fp, &position);

  // copies the daily close prices into the prices array
  s->prices = malloc(s->len_prices * sizeof(double));
  int comma_count = 0;
  for (int i = 0; i < s->len_prices; i++) {
    comma_count = 0;
    while (comma_count < 4) {
      if (fgetc(fp) == ',')
        comma_count++;
    }
    fscanf(fp, "%lf", (s->prices) + i);
    assert((s->prices)[i] > 0);
    while (fgetc(fp) != '\n')
      ;
  }
  fclose(fp);
  remove(pricename);
  return s;
}

// destroys s and all memory associated with it
void *stock_prices_destroy(struct stock_prices *s) {
  assert(s);
  free(s->prices);
  free(s);
}

// gives an estimate for the volatility of s, for the black_scholes model
double get_volatility(const struct stock_prices *s) {
  assert(s);
  double mean = 0;
  double cur;
  for (int i = 1; i < s->len_prices; i++) {
    cur = log((s->prices)[i] / (s->prices)[i - 1]);
    mean += cur;
  }
  mean /= (s->len_prices - 1);
  double var = 0;
  for (int i = 1; i < s->len_prices; i++) {
    var += pow(log((s->prices)[i] / (s->prices)[i - 1]) - mean, 2);
  }
  var /= (s->len_prices - 1);
  return sqrt(var);
}

// returns the dividend yield of s
double get_yield(const struct stock_prices *s) {
  assert(s);
  return s->div_yield;
}

// returns the current price of s
double get_price(const struct stock_prices *s) {
  assert(s);
  return s->cur_price;
}
