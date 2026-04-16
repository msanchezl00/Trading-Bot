/**
 * @file main.cpp
 * @brief Entry point for the C++ Trading Bot – equivalent to Python launcher.py.
 *
 * Default behaviour (no arguments):
 *   Reads ticker symbols from scope.txt, fetches live daily OHLCV data from
 *   Yahoo Finance for each ticker, runs all analysis modules, and prints a
 *   formatted summary table – identical to the Python launcher.py workflow.
 *
 * Usage:
 *   ./trading_bot                        Read scope.txt → live data
 *   ./trading_bot --period 1mo           Override default period (default: 1mo)
 *   ./trading_bot --window 50            Override SMA/EMA window (default: 50)
 *   ./trading_bot --scope mylist.txt     Override scope file
 *   ./trading_bot --csv FILE TICKER      Load a local CSV instead of live data
 *   ./trading_bot --demo                 Run with built-in synthetic data
 *
 * Period values (same as yfinance):
 *   1d  5d  1wk  1mo  3mo  6mo  1y  2y  5y  ytd  max
 */

#include "DataFetcher.hpp"
#include "FinancialCalculator.hpp"
#include "ProfitabilityMetrics.hpp"
#include "RiskAnalysis.hpp"
#include "TradingBot.hpp"

#include <algorithm>
#include <iomanip>
#include <iostream>
#include <stdexcept>
#include <string>
#include <vector>

// ------------------------------------------------------------------ //
// Built-in synthetic demo dataset                                      //
// ------------------------------------------------------------------ //

static FinancialCalculator::MarketData buildDemoData() {
    using namespace FinancialCalculator;
    MarketData d;
    d.asset = "DEMO";

    // 20 synthetic weekly bars with an uptrend followed by a small pullback.
    const struct {
        const char* date;
        double open; double high; double low; double close;
        long long volume;
    } rows[] = {
        {"2024-01-01", 100.0, 103.0,  98.0, 102.0,  900000LL},
        {"2024-01-08", 102.0, 106.0, 100.0, 105.0, 1100000LL},
        {"2024-01-15", 105.0, 108.0, 103.0, 107.0, 1050000LL},
        {"2024-01-22", 107.0, 111.0, 105.0, 110.0, 1200000LL},
        {"2024-01-29", 110.0, 113.0, 108.0, 112.0, 1300000LL},
        {"2024-02-05", 112.0, 116.0, 110.0, 115.0, 1400000LL},
        {"2024-02-12", 115.0, 119.0, 113.0, 118.0, 1350000LL},
        {"2024-02-19", 118.0, 122.0, 116.0, 121.0, 1250000LL},
        {"2024-02-26", 121.0, 124.0, 119.0, 123.0, 1150000LL},
        {"2024-03-04", 123.0, 126.0, 121.0, 125.0, 1100000LL},
        {"2024-03-11", 125.0, 128.0, 123.0, 127.0, 1200000LL},
        {"2024-03-18", 127.0, 130.0, 125.0, 129.0, 1300000LL},
        {"2024-03-25", 129.0, 132.0, 127.0, 131.0, 1400000LL},
        {"2024-04-01", 131.0, 134.0, 129.0, 133.0, 1500000LL},
        {"2024-04-08", 133.0, 136.0, 131.0, 135.0, 1600000LL},
        {"2024-04-15", 135.0, 138.0, 133.0, 137.0, 1550000LL},
        {"2024-04-22", 137.0, 140.0, 135.0, 139.0, 1450000LL},
        {"2024-04-29", 139.0, 142.0, 137.0, 141.0, 1350000LL},
        {"2024-05-06", 141.0, 144.0, 139.0, 143.0, 1250000LL},
        {"2024-05-13", 143.0, 146.0, 141.0, 145.0, 1150000LL},
    };

    for (const auto& r : rows) {
        PriceRecord rec;
        rec.date   = r.date;
        rec.open   = r.open;
        rec.high   = r.high;
        rec.low    = r.low;
        rec.close  = r.close;
        rec.volume = r.volume;
        d.prices.push_back(rec);
    }
    return d;
}

// ------------------------------------------------------------------ //
// Analysis pipeline                                                    //
// ------------------------------------------------------------------ //

static void runAnalysis(FinancialCalculator::MarketData& data, int window) {
    RiskAnalysis::calculateDailyReturns(data);
    RiskAnalysis::calculateCumulativeReturns(data);
    RiskAnalysis::calculateAverageDailyVolume(data);
    ProfitabilityMetrics::calculateMovingAveragesAndCrosses(data, window);
}

// ------------------------------------------------------------------ //
// ASCII table printer (equivalent to tabulate fancy_grid)             //
// ------------------------------------------------------------------ //

struct TableRow {
    std::string asset;
    std::string advice;
    std::string period;
};

static void printTable(const std::vector<TableRow>& rows,
                       const std::string& col1 = "Asset",
                       const std::string& col2 = "Trading Advice",
                       const std::string& col3 = "Period") {
    // Compute column widths.
    std::size_t w1 = col1.size();
    std::size_t w2 = col2.size();
    std::size_t w3 = col3.size();

    for (const auto& r : rows) {
        w1 = std::max(w1, r.asset.size());
        w2 = std::max(w2, r.advice.size());
        w3 = std::max(w3, r.period.size());
    }

    // Print with 1-space padding on each side.
    auto sep = [&](char l, char m, char r, char f) {
        std::cout << l
                  << std::string(w1 + 2, f) << m
                  << std::string(w2 + 2, f) << m
                  << std::string(w3 + 2, f) << r << "\n";
    };

    auto row = [&](const std::string& a, const std::string& b, const std::string& c) {
        std::cout << "| " << std::left << std::setw(static_cast<int>(w1)) << a
                  << " | " << std::setw(static_cast<int>(w2)) << b
                  << " | " << std::setw(static_cast<int>(w3)) << c
                  << " |\n";
    };

    sep('+', '+', '+', '-');
    row(col1, col2, col3);
    sep('+', '+', '+', '=');
    for (const auto& r : rows) {
        row(r.asset, r.advice, r.period);
        sep('+', '+', '+', '-');
    }
}

// ------------------------------------------------------------------ //
// Detailed metrics output (single asset)                              //
// ------------------------------------------------------------------ //

static void printDetailedMetrics(const FinancialCalculator::MarketData& d,
                                  int window) {
    std::size_t n = d.prices.size();
    if (n == 0) return;
    std::size_t last = n - 1;

    std::string sep(72, '=');
    std::cout << sep << "\n"
              << " Asset: " << d.asset
              << "  |  Bars: " << n
              << "  |  Window: " << window << "\n"
              << sep << "\n";

    std::cout << std::fixed << std::setprecision(4);
    std::cout << " Last date           : " << d.prices[last].date              << "\n"
              << " Last close          : " << d.prices[last].close             << "\n"
              << " Last daily return   : " << d.dailyReturns[last]    * 100.0 << " %\n"
              << " Cumulative return   : " << d.cumulativeReturns[last]* 100.0 << " %\n"
              << " Average volume      : " << std::setprecision(0)
                                           << d.averageVolume                  << "\n"
              << std::setprecision(4)
              << " Last SMA            : " << d.sma[last]                      << "\n"
              << " Last EMA            : " << d.ema[last]                      << "\n";

    int gc = 0, dc = 0;
    for (std::size_t i = 0; i < n; ++i) {
        if (d.goldenCross[i]) ++gc;
        if (d.deathCross[i])  ++dc;
    }
    std::cout << " Golden Crosses      : " << gc << "\n"
              << " Death Crosses       : " << dc << "\n"
              << std::string(72, '-') << "\n";
}

// ------------------------------------------------------------------ //
// Argument parsing helpers                                             //
// ------------------------------------------------------------------ //

static std::string getArg(const std::vector<std::string>& args,
                           const std::string& flag,
                           const std::string& fallback = "") {
    for (std::size_t i = 0; i + 1 < args.size(); ++i) {
        if (args[i] == flag) return args[i + 1];
    }
    return fallback;
}

static bool hasFlag(const std::vector<std::string>& args, const std::string& flag) {
    return std::find(args.begin(), args.end(), flag) != args.end();
}

// ------------------------------------------------------------------ //
// main                                                                 //
// ------------------------------------------------------------------ //

int main(int argc, char* argv[]) {
    std::vector<std::string> args(argv + 1, argv + argc);

    // ---- Parse options ----
    const bool demoMode  = hasFlag(args, "--demo");
    const bool csvMode   = hasFlag(args, "--csv");
    const std::string scopeFile = getArg(args, "--scope",  "scope.txt");
    const std::string period    = getArg(args, "--period", "1mo");
    const int window = [&] {
        std::string w = getArg(args, "--window", "50");
        return std::stoi(w);
    }();

    // ---- Demo mode ----
    if (demoMode) {
        auto data = buildDemoData();
        int demoWindow = 5;   // small window suits the 20-bar demo
        runAnalysis(data, demoWindow);
        printDetailedMetrics(data, demoWindow);
        std::cout << " ADVICE: " << TradingBot::getAdvice(data) << "\n"
                  << std::string(72, '=') << "\n";
        return 0;
    }

    // ---- CSV mode ----
    if (csvMode) {
        std::string csvFile = getArg(args, "--csv");
        std::string ticker  = getArg(args, "--ticker", csvFile);
        if (csvFile.empty()) {
            std::cerr << "[ERROR] --csv requires a file path argument.\n"
                      << "Usage: ./trading_bot --csv data.csv --ticker AAPL\n";
            return 1;
        }
        try {
            auto data = FinancialCalculator::loadFromCSV(csvFile, ticker);
            runAnalysis(data, window);
            printDetailedMetrics(data, window);
            std::cout << " ADVICE: " << TradingBot::getAdvice(data) << "\n"
                      << std::string(72, '=') << "\n";
        } catch (const std::exception& ex) {
            std::cerr << "[ERROR] " << ex.what() << "\n";
            return 1;
        }
        return 0;
    }

    // ---- Live mode (default – mirrors launcher.py) ----
    // 1. Read tickers from scope file.
    std::vector<std::string> tickers;
    try {
        tickers = DataFetcher::readScope(scopeFile);
    } catch (const std::exception& ex) {
        std::cerr << "[ERROR] " << ex.what() << "\n"
                  << "Hint: create scope.txt with one ticker per line, "
                     "or use --demo / --csv mode.\n";
        return 1;
    }
    if (tickers.empty()) {
        std::cerr << "[ERROR] " << scopeFile << " contains no valid tickers.\n";
        return 1;
    }

    // 2. For each ticker: fetch → analyse → collect advice.
    std::cout << "Fetching data for " << tickers.size()
              << " asset(s)  (period=" << period
              << ", window=" << window << ") ...\n\n";

    std::vector<TableRow> tableRows;
    int errors = 0;

    for (const auto& ticker : tickers) {
        try {
            auto data = DataFetcher::fetchFromYahooFinance(ticker, period);
            runAnalysis(data, window);
            std::string advice = TradingBot::getAdvice(data);
            tableRows.push_back({ticker, advice, period});
        } catch (const std::exception& ex) {
            std::cerr << "[WARN] " << ticker << ": " << ex.what() << "\n";
            tableRows.push_back({ticker, std::string("Error: ") + ex.what(), period});
            ++errors;
        }
    }

    // 3. Print summary table (mirrors Python tabulate output).
    std::cout << "\n";
    printTable(tableRows);

    if (errors > 0) {
        std::cerr << "\n[WARN] " << errors << " ticker(s) could not be fetched.\n";
    }

    return (errors == static_cast<int>(tickers.size())) ? 1 : 0;
}

