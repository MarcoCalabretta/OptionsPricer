#ifndef STOCK_PRICES_H
#define STOCK_PRICES_H
// This file downloads the historical price data of a given stock and keeps in
// in an array, allowing the client to request various attributes about the
// stock's prices (i.e. mean, median, std dev, etc) Made by Marco Calabretta on
// September 4 2023

// ALL FUNCTIONS REQUIRE VALID POINTERS

// a stock_prices struct that stores a lot of data about a stock's prices
struct stock_prices;

// creates a stock_prices struct with a particular ticker, returns the pointer
// to that struct ,or NULL if the ticker's data couldn't be downloaded ticker is
// lowercase, eg "aapl" for Apple
// effects: allocates heap memory, client must call stock_prices_destroy
struct stock_prices *stock_prices_create(const char *ticker);

// destroys s and all memory associated with it
void *stock_prices_destroy(struct stock_prices *s);

// gives an estimate for the volatility of s, for the black_scholes model
double get_volatility(struct stock_prices *s);

// returns the dividend yield of s, as a percentage, i.e. 5 for 5%
double get_yield(struct stock_prices *s);

// returns the current price of s
double get_price(struct stock_prices *s);
#endif
