#include "binomial_tree.h"
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

int main() {
  double strike = 0;
  char ticker[100];
  int call = 1;
  printf("input ticker: ");
  scanf("%s", ticker);
  printf("input 1 for call, 0 for put: ");
  scanf("%d", &call);
  printf("input strike price: ");
  scanf("%lf", &strike);
  struct tm *exp = malloc(sizeof(struct tm));
  exp->tm_year = 123;
  exp->tm_yday = 349;
  double price = binomial_tree_expected_price(call, true, exp, strike,
                                              (const char *)ticker);
  free(exp);
  printf("price is %lf", price);
}
