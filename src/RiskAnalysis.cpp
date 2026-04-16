#include "RiskAnalysis.hpp"

#include <numeric>
#include <stdexcept>

namespace RiskAnalysis {

void calculateDailyReturns(FinancialCalculator::MarketData& data) {
    const auto& prices = data.prices;
    if (prices.empty()) {
        throw std::invalid_argument("RiskAnalysis: price data is empty.");
    }

    std::size_t n = prices.size();
    data.dailyReturns.resize(n, 0.0);

    // First element has no prior bar, so return is 0.
    for (std::size_t i = 1; i < n; ++i) {
        double prev = prices[i - 1].close;
        if (prev == 0.0) {
            data.dailyReturns[i] = 0.0;
        } else {
            data.dailyReturns[i] = (prices[i].close - prev) / prev;
        }
    }
}

void calculateCumulativeReturns(FinancialCalculator::MarketData& data) {
    const auto& dr = data.dailyReturns;
    if (dr.empty()) {
        throw std::invalid_argument(
            "RiskAnalysis: dailyReturns is empty. "
            "Call calculateDailyReturns() first.");
    }

    std::size_t n = dr.size();
    data.cumulativeReturns.resize(n, 0.0);

    double cumProduct = 1.0;
    for (std::size_t i = 0; i < n; ++i) {
        cumProduct *= (1.0 + dr[i]);
        data.cumulativeReturns[i] = cumProduct - 1.0;
    }
}

void calculateAverageDailyVolume(FinancialCalculator::MarketData& data) {
    const auto& prices = data.prices;
    if (prices.empty()) {
        throw std::invalid_argument("RiskAnalysis: price data is empty.");
    }

    double total = 0.0;
    for (const auto& p : prices) {
        total += static_cast<double>(p.volume);
    }
    data.averageVolume = total / static_cast<double>(prices.size());
}

} // namespace RiskAnalysis
