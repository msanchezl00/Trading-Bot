#pragma once

#include <string>
#include <vector>
#include <stdexcept>

/**
 * @namespace FinancialCalculator
 * @brief Provides common data structures and utilities shared across all
 *        financial analysis modules.
 */
namespace FinancialCalculator {

    /**
     * @brief Represents a single OHLCV (Open, High, Low, Close, Volume) record.
     */
    struct PriceRecord {
        std::string date;   ///< ISO-8601 date string (e.g. "2024-01-15")
        double open   = 0.0;
        double high   = 0.0;
        double low    = 0.0;
        double close  = 0.0;
        long long volume = 0;
    };

    /**
     * @brief Holds a complete time series of price data together with all
     *        derived analytical metrics populated by the analysis modules.
     *
     * Fields are filled incrementally:
     *   1. RiskAnalysis::calculateDailyReturns()      -> dailyReturns
     *   2. RiskAnalysis::calculateCumulativeReturns() -> cumulativeReturns
     *   3. RiskAnalysis::calculateAverageDailyVolume()-> averageVolume
     *   4. ProfitabilityMetrics::calculateSMA()       -> sma
     *   5. ProfitabilityMetrics::calculateEMA()       -> ema
     *   6. ProfitabilityMetrics::findCrossDates()     -> goldenCross, deathCross
     */
    struct MarketData {
        std::string asset;                 ///< Ticker symbol (e.g. "AAPL")
        std::vector<PriceRecord> prices;   ///< Raw OHLCV records (chronological)

        // --- Metrics populated by RiskAnalysis ---
        std::vector<double> dailyReturns;       ///< Pct change of Close prices
        std::vector<double> cumulativeReturns;  ///< Cumulative product of (1 + dailyReturns)
        double averageVolume = 0.0;             ///< Mean volume over the series

        // --- Metrics populated by ProfitabilityMetrics ---
        std::vector<double> sma;          ///< Simple Moving Average
        std::vector<double> ema;          ///< Exponential Moving Average
        std::vector<bool> goldenCross;    ///< true when SMA crosses above EMA at that bar
        std::vector<bool> deathCross;     ///< true when SMA crosses below EMA at that bar
    };

    /**
     * @brief Loads OHLCV data from a CSV file.
     *
     * Expected header row (case-insensitive):
     *   date,open,high,low,close,volume
     *
     * Lines beginning with '#' are treated as comments and skipped.
     *
     * @param filename  Path to the CSV file.
     * @param assetName Optional ticker name stored in MarketData::asset.
     * @return          A MarketData instance with the prices field populated.
     * @throws std::runtime_error if the file cannot be opened or is malformed.
     */
    MarketData loadFromCSV(const std::string& filename,
                           const std::string& assetName = "");

} // namespace FinancialCalculator
