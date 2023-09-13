#include "binomial_tree.h"
#include "date.h"
#include "options_chart.h"
#include "stock_prices.h"
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

const char *quit = "quit";

int main() {
  char ticker[20];
  FILE *fp = fopen("options.csv", "w");
  fprintf(fp, "Ticker,Type,Strike Price,Market Price,Option Price,Model "
              "Price,Volume,Date\n");
  while (true) {
    printf("input ticker (enter %s to stop): ", quit);
    scanf("%s", ticker);
    if (strcmp((const char *)ticker, quit) == 0)
      break;
    double model_price;
    struct stock_prices *s = stock_prices_create(ticker);
    struct options_chart *oc = options_chart_create((const char *)ticker);
    struct option *o = option_create();
    char date[DATE_LENGTH];
    while (options_left(oc)) {
      options_next(oc, o);

      // some options will show as a $0 market price because there's no trades.
      // We don't want to look at those
      if (get_market_price(o) && get_volume(o)) {
        model_price = binomial_tree_expected_price(s, o);
        double moneyness = get_strike_price(o) / get_price(s);
        // here, insert filters to decide which options you want to see.
        if (model_price > get_market_price(o) && moneyness < 1.05 &&
            moneyness > 0.95) {
          fprintf(fp, "%s,", get_ticker(o));
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
          date_string(get_expiry_date(o), date);
          printf("option on %s with strike price $%.2lf and expiry date "
                 "%s.\nmodel "
                 "price is $%.2lf and market price is $%.2lf. Volume is %d\n",
                 get_ticker(o), get_strike_price(o), (const char *)date,
                 model_price, get_market_price(o), get_volume(o));
          fprintf(fp, "%lf,%lf,%lf,%lf,%d,%s\n", get_strike_price(o),
                  get_price(s), get_market_price(o), model_price, get_volume(o),
                  (const char *)date);
        }
      }
    }
    option_destroy(o);
    options_chart_destroy(oc);
    stock_prices_destroy(s);
  }
  fclose(fp);
}
