#pragma once

#include "FinancialCalculator.hpp"

/**
 * @namespace ProfitabilityMetrics
 * @brief Provides trend-following metrics based on moving averages.
 *
 * Typical usage:
 *   ProfitabilityMetrics::calculateMovingAveragesAndCrosses(data, window);
 *
 * Or, step by step:
 *   ProfitabilityMetrics::calculateSMA(data, window);
 *   ProfitabilityMetrics::calculateEMA(data, window);
 *   ProfitabilityMetrics::findCrossDates(data);
 */
namespace ProfitabilityMetrics {

    /**
     * @brief Computes the Simple Moving Average (SMA) of Close prices.
     *
     * For bars with fewer than 'window' preceding values the average is taken
     * over the available bars only (equivalent to min_periods=1 in pandas).
     * Result is stored in data.sma.
     *
     * @param data    MarketData with prices populated.
     * @param window  Look-back period (number of bars).
     */
    void calculateSMA(FinancialCalculator::MarketData& data, int window);

    /**
     * @brief Computes the Exponential Moving Average (EMA) of Close prices.
     *
     * Uses the standard smoothing factor alpha = 2 / (window + 1).
     * The first EMA value is seeded with the first Close price
     * (equivalent to adjust=False, min_periods=1 in pandas ewm).
     * Result is stored in data.ema.
     *
     * @param data    MarketData with prices populated.
     * @param window  Look-back span (number of bars).
     */
    void calculateEMA(FinancialCalculator::MarketData& data, int window);

    /**
     * @brief Identifies Golden Cross and Death Cross events in the series.
     *
     * A **Golden Cross** occurs when SMA crosses from below to above EMA.
     * A **Death Cross** occurs when SMA crosses from above to below EMA.
     *
     * Requires calculateSMA() and calculateEMA() to have been called first.
     * Results are stored in data.goldenCross and data.deathCross (vectors of bool,
     * same length as data.prices; element is true on the bar the cross occurs).
     *
     * @param data  MarketData with sma and ema already populated.
     */
    void findCrossDates(FinancialCalculator::MarketData& data);

    /**
     * @brief Convenience function that calls calculateSMA, calculateEMA, and
     *        findCrossDates in the correct order.
     *
     * @param data    MarketData with prices populated.
     * @param window  Look-back period (number of bars).
     */
    void calculateMovingAveragesAndCrosses(FinancialCalculator::MarketData& data,
                                           int window);

} // namespace ProfitabilityMetrics
