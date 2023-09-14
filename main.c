#include "binomial_tree.h"
#include "date.h"
#include "options_chart.h"
#include "stock_prices.h"
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

const char *quit = "quit";
const char *filename = "options.csv";
const char *tempname = "temp_main.csv";
const char *firstline = "Ticker,Expiry Date,Type,Strike Price,Market "
                        "Price,Option Price,Model Price,Volume\n";

static bool next_eof(FILE *fp) {
  if (feof(fp))
    return true;
  fpos_t position;
  fgetpos(fp, &position);
  fgetc(fp);
  if (feof(fp))
    return true;
  fsetpos(fp, &position);
  return false;
}

int main() {
  int mode = 0;
  char ticker[20];
  char date[DATE_LENGTH];
  printf("input 1 to update prices, 2 to update options: ");
  scanf("%d", &mode);
  if (mode == 1) {
    char prev_ticker[20];
    prev_ticker[0] = '\0';
    double cur_price = 0;
    struct stock_prices *s = NULL;
    FILE *fp = fopen(filename, "r");
    FILE *temp = fopen(tempname, "w");
    struct date *d = current_date();
    char today[DATE_LENGTH];
    date_string(d, today);
    date_destroy(d);

    // skips first line
    char c = 0;
    while (!feof(fp) && c != '\n') {
      c = fgetc(fp);
      fputc(c, temp);
    }

    while (!next_eof(fp)) {
      c = fgetc(fp);
      ;
      int count = 0;
      while (c != ',') {
        ticker[count] = c;
        count++;
        fputc(c, temp);
        c = fgetc(fp);
      }
      fputc(',', temp);
      ticker[count] = '\0';
      c = fgetc(fp);
      count = 0;
      while (c != ',') {
        date[count] = c;
        count++;
        fputc(c, temp);
        c = fgetc(fp);
      }
      fputc(',', temp);
      date[count] = '\0';
      if (strcmp(ticker, prev_ticker)) {
        s = stock_prices_create(ticker);
        cur_price = get_price(s);
        stock_prices_destroy(s);
        s = NULL;
        strcpy(prev_ticker, ticker);
        printf("%s\n", ticker);
      }
      count = 0;
      while (count < 2) {
        c = fgetc(fp);
        fputc(c, temp);
        if (c == ',')
          count++;
      }
      fprintf(temp, "%.2lf,", cur_price);
      while (fgetc(fp) != ',')
        ;
      while (c != '\n') {
        c = fgetc(fp);
        fputc(c, temp);
      }
    }

    fclose(fp);
    fclose(temp);
    remove(filename);
    rename(tempname, filename);
  }
  if (mode == 2) {
    FILE *fp = fopen(filename, "w");
    fprintf(fp, firstline);
    while (true) {
      printf("input ticker (enter %s to stop): ", quit);
      scanf("%s", ticker);
      printf("ticker is %s\n", ticker);
      if (strcmp((const char *)ticker, quit) == 0)
        break;
      double model_price;
      struct stock_prices *s = stock_prices_create(ticker);
      struct options_chart *oc = options_chart_create((const char *)ticker);
      struct option *o = option_create();
      while (options_left(oc)) {
        options_next(oc, o);

        // some options will show as a $0 market price because there's no
        // trades. We don't want to look at those
        double moneyness = get_strike_price(o) / get_price(s);
        if (get_market_price(o) && get_volume(o) > 10 && moneyness < 1.05 &&
            moneyness > 0.95) {
          model_price = binomial_tree_expected_price(s, o);
          // here, insert filters to decide which options you want to see.
          if ((model_price - get_market_price(o)) > 0.05) {
            date_string(get_expiry_date(o), date);
            fprintf(fp, "%s,%s,", get_ticker(o), (const char *)date);
            if (is_american(o))
              printf("American ");
            else
              printf("European ");
            if (is_call(o)) {
              printf("call ");
              fprintf(fp, "Call,");
            } else {
              printf("put ");
              fprintf(fp, "Put,");
            }
            printf("option on %s with strike price $%.2lf and expiry date "
                   "%s.\nmodel "
                   "price is $%.2lf and market price is $%.2lf. Volume is %d\n",
                   get_ticker(o), get_strike_price(o), (const char *)date,
                   model_price, get_market_price(o), get_volume(o));
            fprintf(fp, "%lf,%lf,%lf,%lf,%d\n", get_strike_price(o),
                    get_price(s), get_market_price(o), model_price,
                    get_volume(o));
          }
        }
      }
      option_destroy(o);
      options_chart_destroy(oc);
      stock_prices_destroy(s);
    }
    fclose(fp);
  }
}
