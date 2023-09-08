#include "binomial_tree.h"
#include "date.h"
#include "options_chart.h"
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>

int main() {
  char ticker[20];
  printf("input ticker: ");
  scanf("%s", ticker);
  double model_price;
  struct options_chart *oc = options_chart_create((const char *)ticker);
  struct option *o = option_create();
  while (options_left(oc)) {
    options_next(oc, o);
    model_price = binomial_tree_expected_price(o);
    if (is_american(o))
      printf("American ");
    else
      printf("European ");
    if (is_call(o))
      printf("call ");
    else
      printf("put ");
    char date[DATE_LENGTH];
    date_string(get_expiry_date(o), date);
    printf("option on %s with strike price %lf and expiry date %s.\nmodel "
           "price is %lf and market price is %lf\n",
           get_ticker(o), get_strike_price(o), (const char *)date, model_price,
           get_market_price(o));
  }
  option_destroy(o);
  options_chart_destroy(oc);
}
/*
  printf("input 1 for call, 0 for put: ");
  scanf("%d", &call);
  printf("input strike price: ");
  scanf("%lf", &strike);
  struct date *exp = date_create(15, 12, 2023);
  double price = binomial_tree_expected_price(call, true, exp, strike,
                                              (const char *)ticker);
  date_destroy(exp);
  printf("price is %lf", price);
printf("input 1 for call, 0 for put: ");
scanf("%d", &call);
printf("input strike price: ");
scanf("%lf", &strike);
struct date *exp = date_create(15, 12, 2023);
double price = binomial_tree_expected_price(call, true, exp, strike,
                                            (const char *)ticker);
date_destroy(exp);
printf("price is %lf", price);
*/
