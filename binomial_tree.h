#ifndef BINOMIAL_TREE_H
#define BINOMIAL_TREE_H

#include <stdbool.h>
#include <time.h>

// Put in an option, and it prints out the binomial tree value of the option.
// Volatility is estimated to be the total historical volatility of the
// security. Made by Marco Calabretta on
// September 1 2023

// ALL FUNCTIONS REQUIRE VALID POINTERS

// Returns the expected price of the option
// risk_free_rate is expressed as a percentage, e.g input 5 for 5% rate
// requires: strike_price > 0
// 		risk_free_rate >= 0
double binomial_tree_expected_price(bool call, bool american,
                                    struct tm *expiry_date, double strike_price,
                                    const char *ticker);
#endif
