#ifndef OPTIONS_CHART_H
#define OPTIONS_CHART_H
#include "date.h"
#include <stdbool.h>
// a list of functions to deal with downloading options charts and reading them

// ALL FUNCTIONS REQUIRE VALID POINTERS

// an option_chart holds a bunch of options
struct options_chart;
struct option;

// creates an options chart for ticker and returns a pointer to it
// effects: allocates heap memory, client must call
// options_destroy
struct options_chart *options_chart_create(const char *ticker);

// destroys all memory associated with oc
void options_chart_destroy(struct options_chart *oc);

// creates an option and returns an option to it
// effects: allocates heap memory, client must call option_destroy
struct option *option_create();

// destroys all memory associated with 0
void option_destroy(struct option *o);

// returns true if there are still options left in the chart, false otherwise
bool options_left(const struct options_chart *oc);

// reads the next option from oc and assigns the value to o
// requires: options_left(oc) is true
void options_next( struct options_chart *oc, struct option *o);

// returns the market price of o
double get_market_price(const struct option *o);

// returns the strike price of o
double get_strike_price(const struct option *o);

// returns the ticker name of o
const char *get_ticker(const struct option *o);

// returns the expiry date of o
const struct date *get_expiry_date(const struct option *o);

// returns true if o is a call
bool is_call(const struct option *o);

// returns true if o is an american option
bool is_american(const struct option *o);
#endif
