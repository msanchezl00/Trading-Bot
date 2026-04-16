#include "ProfitabilityMetrics.hpp"

#include <stdexcept>

namespace ProfitabilityMetrics {

void calculateSMA(FinancialCalculator::MarketData& data, int window) {
    const auto& prices = data.prices;
    if (prices.empty()) {
        throw std::invalid_argument("ProfitabilityMetrics: price data is empty.");
    }
    if (window <= 0) {
        throw std::invalid_argument("ProfitabilityMetrics: window must be > 0.");
    }

    std::size_t n = prices.size();
    data.sma.resize(n, 0.0);

    for (std::size_t i = 0; i < n; ++i) {
        // Use min_periods=1: average over however many bars are available.
        std::size_t start  = (i + 1 > static_cast<std::size_t>(window))
                             ? (i + 1 - static_cast<std::size_t>(window)) : 0;
        double sum  = 0.0;
        std::size_t count = 0;
        for (std::size_t j = start; j <= i; ++j) {
            sum += prices[j].close;
            ++count;
        }
        data.sma[i] = (count > 0) ? (sum / static_cast<double>(count)) : 0.0;
    }
}

void calculateEMA(FinancialCalculator::MarketData& data, int window) {
    const auto& prices = data.prices;
    if (prices.empty()) {
        throw std::invalid_argument("ProfitabilityMetrics: price data is empty.");
    }
    if (window <= 0) {
        throw std::invalid_argument("ProfitabilityMetrics: window must be > 0.");
    }

    std::size_t n = prices.size();
    data.ema.resize(n, 0.0);

    // Smoothing factor: alpha = 2 / (span + 1)
    double alpha = 2.0 / (static_cast<double>(window) + 1.0);

    // Seed first EMA with the first Close price (min_periods=1, adjust=False).
    data.ema[0] = prices[0].close;
    for (std::size_t i = 1; i < n; ++i) {
        data.ema[i] = alpha * prices[i].close + (1.0 - alpha) * data.ema[i - 1];
    }
}

void findCrossDates(FinancialCalculator::MarketData& data) {
    std::size_t n = data.prices.size();
    if (data.sma.size() != n || data.ema.size() != n) {
        throw std::invalid_argument(
            "ProfitabilityMetrics: sma/ema must be calculated before findCrossDates().");
    }

    data.goldenCross.assign(n, false);
    data.deathCross.assign(n, false);

    for (std::size_t i = 1; i < n; ++i) {
        bool currAbove = data.sma[i]     > data.ema[i];
        bool prevBelow = data.sma[i - 1] <= data.ema[i - 1];
        bool currBelow = data.sma[i]     < data.ema[i];
        bool prevAbove = data.sma[i - 1] >= data.ema[i - 1];

        if (currAbove && prevBelow) {
            data.goldenCross[i] = true;   // SMA crossed above EMA
        }
        if (currBelow && prevAbove) {
            data.deathCross[i] = true;    // SMA crossed below EMA
        }
    }
}

void calculateMovingAveragesAndCrosses(FinancialCalculator::MarketData& data,
                                       int window) {
    calculateSMA(data, window);
    calculateEMA(data, window);
    findCrossDates(data);
}

} // namespace ProfitabilityMetrics
