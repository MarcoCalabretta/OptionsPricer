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
double binomial_tree_expected_price(const struct stock_prices *s,
                                    const struct option *o) {
  assert(o);
  assert(s);
  double v = get_volatility(s);
  double yield = get_yield(s);
  double cur = get_price(s);
  struct date *today = current_date();
  double years_left =
      (double)date_compare(get_expiry_date(o), today) / DAYS_PER_YEAR;
  date_destroy(today);
  // if the option expires today I don't really care, the option is worthless to me
  if(years_left == 0) return 0;
  double dT = years_left / NUM_STEPS;
  double u = exp(v * sqrt(dT));
  double p = (exp((R - yield) * dT) - 1 / u) / (u - 1 / u);
  double values[NUM_STEPS + 1];
  for (int i = NUM_STEPS; i >= 0; i--) {
    for (int j = 0; j <= i; j++) {
      if (i == NUM_STEPS) {
        values[j] = cur * pow(u, i - 2 * j) - get_strike_price(o);
        if (!is_call(o))
          values[j] *= -1;
        if (values[j] < 0)
          values[j] = 0;
      } else {
        values[j] = (p * values[j] + (1 - p) * values[j + 1]) / pow(R, dT);
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
  return values[0];
}
