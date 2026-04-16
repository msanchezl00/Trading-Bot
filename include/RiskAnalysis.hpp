#pragma once

#include "FinancialCalculator.hpp"

/**
 * @namespace RiskAnalysis
 * @brief Provides risk-related financial metrics derived from raw price data.
 *
 * All functions operate in-place on a FinancialCalculator::MarketData object.
 * They must be called in the order:
 *   1. calculateDailyReturns()
 *   2. calculateCumulativeReturns()   (requires dailyReturns)
 *   3. calculateAverageDailyVolume()
 */
namespace RiskAnalysis {

    /**
     * @brief Calculates the percentage change (daily return) of the Close price.
     *
     * The first element is always 0.0 (no prior bar to compare against).
     * Formula: dailyReturn[i] = (close[i] - close[i-1]) / close[i-1]
     *
     * @param data  MarketData object whose prices field must be non-empty.
     */
    void calculateDailyReturns(FinancialCalculator::MarketData& data);

    /**
     * @brief Calculates the running cumulative return from the start of the series.
     *
     * Formula: cumulativeReturn[i] = product( 1 + dailyReturn[j], j=0..i ) - 1
     *
     * Requires calculateDailyReturns() to have been called first.
     *
     * @param data  MarketData object with dailyReturns already populated.
     */
    void calculateCumulativeReturns(FinancialCalculator::MarketData& data);

    /**
     * @brief Calculates and stores the mean volume across all price records.
     *
     * Result is stored in data.averageVolume.
     *
     * @param data  MarketData object whose prices field must be non-empty.
     */
    void calculateAverageDailyVolume(FinancialCalculator::MarketData& data);

} // namespace RiskAnalysis
