#include "constants.h"
#include "stock_prices.h"
#include <assert.h>
#include <math.h>
#include <stdbool.h>
#include <stdio.h>
#include <time.h>

// see binomial_tree.h

// Returns the expected price of the option
double binomial_tree_expected_price(bool call, bool american,
                                    struct tm *expiry_date, double strike_price,
                                    const char *ticker) {
  assert(expiry_date);
  assert(ticker);
  assert(strike_price);
  struct stock_prices *s = stock_prices_create(ticker);
  double v = get_volatility(s);
  double yield = get_yield(s);
  double cur = get_price(s);
  time_t t;
  time(&t);
  struct tm *today = gmtime(&t);
  double years_left = ((double)(expiry_date->tm_yday - today->tm_yday)) / 365 +
                      expiry_date->tm_year - today->tm_year;
  double dT = years_left / NUM_STEPS;
  double u = exp(v * sqrt(dT));
  double p = (exp((R - yield) * dT / 100) - 1 / u) / (u - 1 / u);
  double values[NUM_STEPS + 1];
  for (int i = NUM_STEPS; i > 0; i--) {
    for (int j = 0; j <= i; j++) {
      if (i == NUM_STEPS) {
        values[j] = cur * pow(u, i - 2 * j) - strike_price;
        if (!call)
          values[j] *= -1;
        if (values[j] < 0)
          values[j] = 0;
      } else {
        values[j] = (p * values[j] + (1 - p) * values[j + 1]) *
                    exp((R + 1) / 100 * dT * -1);
        if (american) {
          double exercise_price = cur * pow(u, i - 2 * j) - strike_price;
          if (!call)
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
