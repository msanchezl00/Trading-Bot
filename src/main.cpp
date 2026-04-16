/**
 * @file main.cpp
 * @brief Entry point for the C++ Trading Bot financial analysis module.
 *
 * Usage:
 *   ./trading_bot [data.csv [TICKER [WINDOW]]]
 *
 * When no CSV is provided, a built-in synthetic dataset is used to
 * demonstrate all modules without external data dependencies.
 *
 * CSV format (header row required):
 *   date,open,high,low,close,volume
 */

#include "FinancialCalculator.hpp"
#include "RiskAnalysis.hpp"
#include "ProfitabilityMetrics.hpp"
#include "TradingBot.hpp"

#include <iostream>
#include <iomanip>
#include <string>
#include <stdexcept>

// ------------------------------------------------------------------ //
// Built-in synthetic demo dataset                                      //
// ------------------------------------------------------------------ //

static FinancialCalculator::MarketData buildDemoData() {
    using namespace FinancialCalculator;
    MarketData d;
    d.asset = "DEMO";

    // 20 synthetic weekly bars with an uptrend followed by a small pullback.
    const struct { const char* date; double o; double h; double l; double c; long long v; }
    rows[] = {
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
        rec.open   = r.o;
        rec.high   = r.h;
        rec.low    = r.l;
        rec.close  = r.c;
        rec.volume = r.v;
        d.prices.push_back(rec);
    }
    return d;
}

// ------------------------------------------------------------------ //
// Display helpers                                                      //
// ------------------------------------------------------------------ //

static void printSeparator(char ch = '-', int width = 72) {
    std::cout << std::string(static_cast<std::size_t>(width), ch) << '\n';
}

static void printHeader(const std::string& asset, int window) {
    printSeparator('=');
    std::cout << " Trading Bot Analysis  |  Asset: " << std::left << std::setw(12)
              << asset << "  |  Window: " << window << "\n";
    printSeparator('=');
}

static void printMetricsSummary(const FinancialCalculator::MarketData& d) {
    std::size_t n = d.prices.size();
    if (n == 0) return;
    std::size_t last = n - 1;

    std::cout << std::fixed << std::setprecision(4);
    std::cout << " Bars analysed       : " << n                              << "\n"
              << " Last date           : " << d.prices[last].date             << "\n"
              << " Last close          : " << d.prices[last].close            << "\n"
              << " Last daily return   : " << d.dailyReturns[last]   * 100.0 << " %\n"
              << " Cumulative return   : " << d.cumulativeReturns[last]*100.0 << " %\n"
              << " Average volume      : " << std::setprecision(0)
                                           << d.averageVolume                 << "\n"
              << std::setprecision(4)
              << " Last SMA            : " << d.sma[last]                     << "\n"
              << " Last EMA            : " << d.ema[last]                     << "\n";

    // Count cross events.
    int gc = 0, dc = 0;
    for (std::size_t i = 0; i < n; ++i) {
        if (d.goldenCross[i]) ++gc;
        if (d.deathCross[i])  ++dc;
    }
    std::cout << " Golden Crosses      : " << gc                              << "\n"
              << " Death Crosses       : " << dc                              << "\n";
    printSeparator();
}

// ------------------------------------------------------------------ //
// main                                                                 //
// ------------------------------------------------------------------ //

int main(int argc, char* argv[]) {
    // Default parameters.
    std::string csvFile;
    std::string ticker   = "DEMO";
    int         window   = 5;   // small window suits the 20-bar demo data

    if (argc >= 2) csvFile = argv[1];
    if (argc >= 3) ticker  = argv[2];
    if (argc >= 4) window  = std::stoi(argv[3]);

    try {
        // 1. Load or synthesise data.
        FinancialCalculator::MarketData data =
            csvFile.empty() ? buildDemoData()
                            : FinancialCalculator::loadFromCSV(csvFile, ticker);

        if (!csvFile.empty()) data.asset = ticker;

        printHeader(data.asset, window);

        // 2. Risk analysis.
        RiskAnalysis::calculateDailyReturns(data);
        RiskAnalysis::calculateCumulativeReturns(data);
        RiskAnalysis::calculateAverageDailyVolume(data);

        // 3. Profitability metrics.
        ProfitabilityMetrics::calculateMovingAveragesAndCrosses(data, window);

        // 4. Print summary.
        printMetricsSummary(data);

        // 5. Trading advice.
        std::string advice = TradingBot::getAdvice(data);
        std::cout << " ADVICE: " << advice << "\n";
        printSeparator('=');

    } catch (const std::exception& ex) {
        std::cerr << "[ERROR] " << ex.what() << '\n';
        return 1;
    }

    return 0;
}
