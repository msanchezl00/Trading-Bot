#pragma once

#include "FinancialCalculator.hpp"

#include <string>
#include <vector>

/**
 * @namespace DataFetcher
 * @brief Fetches live OHLCV market data from the Yahoo Finance API and
 *        reads ticker lists from scope files.
 *
 * This module is the C++ equivalent of the Python launcher's yfinance calls:
 *   data = yf.Ticker(asset).history(period)
 *
 * Requires libcurl at link time. If a network error occurs, a
 * std::runtime_error is thrown with a descriptive message.
 */
namespace DataFetcher {

    /**
     * @brief Fetches historical daily OHLCV data from Yahoo Finance.
     *
     * Mirrors Python:  yf.Ticker(ticker).history(period)
     *
     * Internally hits:
     *   https://query1.finance.yahoo.com/v8/finance/chart/{ticker}?range={period}&interval=1d
     *
     * Supported period values (same as yfinance):
     *   "1d", "5d", "1mo", "3mo", "6mo", "1y", "2y", "5y", "ytd", "max"
     *   and the yfinance weekly alias "1wk" (mapped to "5d").
     *
     * @param ticker  Ticker symbol (e.g. "AAPL", "BTC-USD").
     * @param period  Historical range string (default "1mo").
     * @return        Populated MarketData with prices in chronological order.
     * @throws std::runtime_error on network failure or unexpected response.
     */
    FinancialCalculator::MarketData fetchFromYahooFinance(
        const std::string& ticker,
        const std::string& period = "1mo");

    /**
     * @brief Reads a list of ticker symbols from a plain-text file.
     *
     * One ticker per line. Lines beginning with '#' and blank lines are
     * ignored. This mirrors the Python:
     *   with open("scope.txt") as f: scope = f.read().splitlines()
     *
     * @param filename  Path to the scope file (default "scope.txt").
     * @return          Vector of non-empty ticker strings.
     * @throws std::runtime_error if the file cannot be opened.
     */
    std::vector<std::string> readScope(const std::string& filename = "scope.txt");

} // namespace DataFetcher
