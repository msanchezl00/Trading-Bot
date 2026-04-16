#pragma once

#include "FinancialCalculator.hpp"
#include <string>

/**
 * @namespace TradingBot
 * @brief Produces a human-readable trading recommendation based on a fully
 *        analysed MarketData object.
 *
 * The advice mirrors the multi-factor strategy implemented in the original
 * Python trading_bot.py:
 *   - Presence of a recent Golden Cross or Death Cross
 *   - Average daily volume threshold (> 100 000)
 *   - Sign of the most recent daily return
 *   - Sign of the most recent cumulative return
 *   - Relative position of SMA vs EMA
 *   - Close price vs SMA
 *   - Volume trend (last bar vs previous bar)
 */
namespace TradingBot {

    /**
     * @brief Returns a trading recommendation string for the given asset data.
     *
     * The MarketData object must have all metrics fully populated:
     *   dailyReturns, cumulativeReturns, averageVolume,
     *   sma, ema, goldenCross, deathCross.
     *
     * @param data  Fully analysed MarketData.
     * @return      A recommendation string (e.g. "Consider buying. ...").
     */
    std::string getAdvice(const FinancialCalculator::MarketData& data);

} // namespace TradingBot
