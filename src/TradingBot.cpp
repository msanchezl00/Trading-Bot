#include "TradingBot.hpp"

#include <algorithm>
#include <stdexcept>

namespace TradingBot {

/// Minimum average volume required to generate a strong buy or sell signal.
static constexpr double MIN_AVERAGE_VOLUME_THRESHOLD = 100000.0;

std::string getAdvice(const FinancialCalculator::MarketData& data) {
    const std::size_t n = data.prices.size();

    if (n == 0 ||
        data.dailyReturns.size()      != n ||
        data.cumulativeReturns.size() != n ||
        data.sma.size()               != n ||
        data.ema.size()               != n ||
        data.goldenCross.size()       != n ||
        data.deathCross.size()        != n) {
        throw std::invalid_argument(
            "TradingBot: MarketData is not fully analysed. "
            "Populate all metrics before calling getAdvice().");
    }

    // Check whether any Golden Cross or Death Cross exists in the series.
    bool hasGoldenCross = std::any_of(data.goldenCross.begin(),
                                      data.goldenCross.end(),
                                      [](bool b){ return b; });
    bool hasDeathCross  = std::any_of(data.deathCross.begin(),
                                      data.deathCross.end(),
                                      [](bool b){ return b; });

    // Grab last-bar values.
    const double lastDailyReturn      = data.dailyReturns.back();
    const double lastCumulativeReturn = data.cumulativeReturns.back();
    const double lastSMA              = data.sma.back();
    const double lastEMA              = data.ema.back();
    const double lastClose            = data.prices.back().close;
    const double lastVolume           = static_cast<double>(data.prices.back().volume);
    const double prevVolume           = (n >= 2)
                                        ? static_cast<double>(data.prices[n - 2].volume)
                                        : lastVolume;
    const double avgVolume            = data.averageVolume;

    // ------------------------------------------------------------------ //
    // Golden Cross branch                                                  //
    // ------------------------------------------------------------------ //
    if (hasGoldenCross) {
        if (avgVolume > MIN_AVERAGE_VOLUME_THRESHOLD) {
            if (lastDailyReturn > 0.0) {
                if (lastCumulativeReturn > 0.0) {
                    if (lastSMA > lastEMA) {
                        if (lastClose > lastSMA) {
                            if (lastVolume > prevVolume) {
                                return "Consider buying. Recent Golden Cross, high average "
                                       "daily volume, positive daily return, positive "
                                       "cumulative returns, SMA above EMA, Close above SMA, "
                                       "and increasing volume.";
                            } else {
                                return "Hold. Recent Golden Cross, high average daily volume, "
                                       "positive daily return, positive cumulative returns, "
                                       "SMA above EMA, Close above SMA, but volume is not "
                                       "increasing.";
                            }
                        } else {
                            return "Hold. Recent Golden Cross, high average daily volume, "
                                   "positive daily return, positive cumulative returns, "
                                   "SMA above EMA, but Close is not above SMA.";
                        }
                    } else {
                        return "Hold. Recent Golden Cross, high average daily volume, "
                               "positive daily return, positive cumulative returns, "
                               "but SMA is not above EMA.";
                    }
                } else {
                    return "Consider buying. Recent Golden Cross, high average daily volume, "
                           "positive daily return, but be cautious about recent negative "
                           "cumulative returns.";
                }
            } else {
                return "Consider buying. Recent Golden Cross and high average daily volume, "
                       "but be cautious about recent negative daily return.";
            }
        } else {
            return "Hold. Recent Golden Cross, but average daily volume is not high enough "
                   "for a strong buy signal.";
        }
    }

    // ------------------------------------------------------------------ //
    // Death Cross branch                                                   //
    // ------------------------------------------------------------------ //
    if (hasDeathCross) {
        if (avgVolume > MIN_AVERAGE_VOLUME_THRESHOLD) {
            if (lastDailyReturn < 0.0) {
                if (lastCumulativeReturn < 0.0) {
                    if (lastSMA < lastEMA) {
                        if (lastClose < lastSMA) {
                            if (lastVolume > prevVolume) {
                                return "Consider selling. Recent Death Cross, high average "
                                       "daily volume, negative daily return, negative "
                                       "cumulative returns, SMA below EMA, Close below SMA, "
                                       "and increasing volume.";
                            } else {
                                return "Hold. Recent Death Cross, high average daily volume, "
                                       "negative daily return, negative cumulative returns, "
                                       "SMA below EMA, Close below SMA, but volume is not "
                                       "increasing.";
                            }
                        } else {
                            return "Hold. Recent Death Cross, high average daily volume, "
                                   "negative daily return, negative cumulative returns, "
                                   "SMA below EMA, but Close is not below SMA.";
                        }
                    } else {
                        return "Hold. Recent Death Cross, high average daily volume, "
                               "negative daily return, negative cumulative returns, "
                               "but SMA is not below EMA.";
                    }
                } else {
                    return "Consider selling. Recent Death Cross, high average daily volume, "
                           "negative daily return, but be cautious about recent positive "
                           "cumulative returns.";
                }
            } else {
                return "Consider selling. Recent Death Cross and high average daily volume, "
                       "but be cautious about recent positive daily return.";
            }
        } else {
            return "Hold. Recent Death Cross, but average daily volume is not high enough "
                   "for a strong sell signal.";
        }
    }

    // ------------------------------------------------------------------ //
    // No signal                                                            //
    // ------------------------------------------------------------------ //
    return "Hold. No specific trading signals based on the current analysis.";
}

} // namespace TradingBot
