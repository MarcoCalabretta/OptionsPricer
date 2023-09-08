#include "constants.h"
#include "date.h"
#include "options_chart.h"
#include "stock_prices.h"
#include <assert.h>
#include <math.h>
#include <stdbool.h>
#include <stdio.h>

// see binomial_tree.h

// Returns the expected price of the option
double binomial_tree_expected_price(struct option *o) {
  assert(o);
  struct stock_prices *s = stock_prices_create(get_ticker(o));
  double v = get_volatility(s);
  double yield = get_yield(s);
  double cur = get_price(s);
  struct date *today = current_date();
  double years_left = date_compare(get_expiry_date(o), today);
  date_destroy(today);
  double dT = years_left / NUM_STEPS;
  double u = exp(v * sqrt(dT));
  double p = (exp((R - yield) * dT / 100) - 1 / u) / (u - 1 / u);
  double values[NUM_STEPS + 1];
  for (int i = NUM_STEPS; i > 0; i--) {
    for (int j = 0; j <= i; j++) {
      if (i == NUM_STEPS) {
        values[j] = cur * pow(u, i - 2 * j) - get_strike_price(o);
        if (!is_call(o))
          values[j] *= -1;
        if (values[j] < 0)
          values[j] = 0;
      } else {
        values[j] = (p * values[j] + (1 - p) * values[j + 1]) *
                    exp((R + 1) / 100 * dT * -1);
        if (is_american(o)) {
          double exercise_price = cur * pow(u, i - 2 * j) - get_strike_price(o);
          if (!is_call(o))
            exercise_price *= -1;
          if (exercise_price > values[j]) {
            values[j] = exercise_price;
          }
        }
      }
    }
  }
  stock_prices_destroy(s);
  return values[0];
}
