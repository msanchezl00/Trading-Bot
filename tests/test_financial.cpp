/**
 * @file test_financial.cpp
 * @brief Unit tests for the C++ financial analysis modules.
 *
 * No external testing framework is required. Each TEST() macro runs a named
 * test case; failures are reported to stderr and the program exits with a
 * non-zero status code so that CI can detect them.
 *
 * Build and run:
 *   cd build && cmake .. -DBUILD_TESTS=ON && cmake --build . && ctest --output-on-failure
 * or directly:
 *   g++ -std=c++17 -I../include ../src/FinancialCalculator.cpp \
 *       ../src/RiskAnalysis.cpp ../src/ProfitabilityMetrics.cpp \
 *       ../src/TradingBot.cpp ../src/DataFetcher.cpp \
 *       test_financial.cpp -lcurl -o test_financial && ./test_financial
 */

#include "FinancialCalculator.hpp"
#include "RiskAnalysis.hpp"
#include "ProfitabilityMetrics.hpp"
#include "TradingBot.hpp"
#include "DataFetcher.hpp"

#include <iostream>
#include <cmath>
#include <cassert>
#include <stdexcept>
#include <fstream>
#include <string>

// ------------------------------------------------------------------ //
// Minimal test harness                                                 //
// ------------------------------------------------------------------ //

static int g_passed = 0;
static int g_failed = 0;

#define TEST(name) void name(); \
    struct _Reg_##name { _Reg_##name() { \
        std::cout << "  Running " #name " ... "; \
        try { name(); std::cout << "PASS\n"; ++g_passed; } \
        catch (const std::exception& e) \
            { std::cout << "FAIL: " << e.what() << "\n"; ++g_failed; } \
        catch (...) \
            { std::cout << "FAIL (unknown exception)\n"; ++g_failed; } \
    }} _reg_##name; \
    void name()

#define ASSERT_EQ(a, b) \
    do { if (!((a) == (b))) throw std::runtime_error( \
        std::string("ASSERT_EQ failed: " #a " != " #b \
                    " (got " + std::to_string(a) + ")")); } while(0)

#define ASSERT_NEAR(a, b, eps) \
    do { if (std::fabs((a)-(b)) > (eps)) throw std::runtime_error( \
        std::string("ASSERT_NEAR failed: |" #a " - " #b "| > " #eps \
                    " (diff=" + std::to_string(std::fabs((a)-(b))) + ")")); } while(0)

#define ASSERT_TRUE(cond) \
    do { if (!(cond)) throw std::runtime_error("ASSERT_TRUE failed: " #cond); } while(0)

#define ASSERT_FALSE(cond) \
    do { if (cond) throw std::runtime_error("ASSERT_FALSE failed: " #cond); } while(0)

#define ASSERT_THROWS(expr) \
    do { bool _threw = false; \
         try { expr; } catch (...) { _threw = true; } \
         if (!_threw) throw std::runtime_error("Expected exception not thrown: " #expr); \
    } while(0)

// ------------------------------------------------------------------ //
// Helper: build a small MarketData manually                           //
// ------------------------------------------------------------------ //

static FinancialCalculator::MarketData makeData(
        const std::vector<double>& closes,
        const std::vector<long long>& volumes = {}) {
    using namespace FinancialCalculator;
    MarketData d;
    d.asset = "TEST";
    for (std::size_t i = 0; i < closes.size(); ++i) {
        PriceRecord r;
        r.date   = "2024-01-" + std::to_string(i + 1);
        r.close  = closes[i];
        r.open   = closes[i];
        r.high   = closes[i];
        r.low    = closes[i];
        r.volume = (i < volumes.size()) ? volumes[i] : 1000000LL;
        d.prices.push_back(r);
    }
    return d;
}

// ------------------------------------------------------------------ //
// RiskAnalysis tests                                                   //
// ------------------------------------------------------------------ //

TEST(test_daily_returns_first_element_is_zero) {
    auto d = makeData({100.0, 110.0, 99.0});
    RiskAnalysis::calculateDailyReturns(d);
    ASSERT_EQ(d.dailyReturns.size(), 3u);
    ASSERT_NEAR(d.dailyReturns[0], 0.0, 1e-10);
}

TEST(test_daily_returns_values) {
    auto d = makeData({100.0, 110.0, 99.0});
    RiskAnalysis::calculateDailyReturns(d);
    ASSERT_NEAR(d.dailyReturns[1],  0.10, 1e-9);   // +10 %
    ASSERT_NEAR(d.dailyReturns[2], -0.10, 1e-9);   // -10 %
}

TEST(test_cumulative_returns) {
    auto d = makeData({100.0, 110.0, 121.0});
    RiskAnalysis::calculateDailyReturns(d);
    RiskAnalysis::calculateCumulativeReturns(d);
    // After two +10 % returns: (1.1 * 1.1) - 1 = 0.21
    ASSERT_NEAR(d.cumulativeReturns[2], 0.21, 1e-9);
}

TEST(test_average_volume) {
    auto d = makeData({100.0, 110.0, 120.0}, {1000LL, 2000LL, 3000LL});
    RiskAnalysis::calculateAverageDailyVolume(d);
    ASSERT_NEAR(d.averageVolume, 2000.0, 1e-9);
}

TEST(test_daily_returns_throws_on_empty) {
    FinancialCalculator::MarketData d;
    ASSERT_THROWS(RiskAnalysis::calculateDailyReturns(d));
}

TEST(test_cumulative_returns_throws_on_empty_dailyReturns) {
    auto d = makeData({100.0, 110.0});
    // dailyReturns not yet populated
    ASSERT_THROWS(RiskAnalysis::calculateCumulativeReturns(d));
}

// ------------------------------------------------------------------ //
// ProfitabilityMetrics tests                                          //
// ------------------------------------------------------------------ //

TEST(test_sma_single_value) {
    auto d = makeData({50.0});
    ProfitabilityMetrics::calculateSMA(d, 5);
    ASSERT_NEAR(d.sma[0], 50.0, 1e-9);
}

TEST(test_sma_window_equals_size) {
    // [10, 20, 30]  SMA(3): [10, 15, 20]
    auto d = makeData({10.0, 20.0, 30.0});
    ProfitabilityMetrics::calculateSMA(d, 3);
    ASSERT_NEAR(d.sma[0], 10.0, 1e-9);
    ASSERT_NEAR(d.sma[1], 15.0, 1e-9);
    ASSERT_NEAR(d.sma[2], 20.0, 1e-9);
}

TEST(test_sma_window_smaller_than_size) {
    // [10, 20, 30, 40]  SMA(2): [10, 15, 25, 35]
    auto d = makeData({10.0, 20.0, 30.0, 40.0});
    ProfitabilityMetrics::calculateSMA(d, 2);
    ASSERT_NEAR(d.sma[0], 10.0, 1e-9);
    ASSERT_NEAR(d.sma[1], 15.0, 1e-9);
    ASSERT_NEAR(d.sma[2], 25.0, 1e-9);
    ASSERT_NEAR(d.sma[3], 35.0, 1e-9);
}

TEST(test_ema_single_value) {
    auto d = makeData({50.0});
    ProfitabilityMetrics::calculateEMA(d, 3);
    ASSERT_NEAR(d.ema[0], 50.0, 1e-9);
}

TEST(test_ema_convergence) {
    // A constant series – EMA should stay constant.
    auto d = makeData({100.0, 100.0, 100.0, 100.0, 100.0});
    ProfitabilityMetrics::calculateEMA(d, 3);
    for (double v : d.ema) ASSERT_NEAR(v, 100.0, 1e-9);
}

TEST(test_find_cross_dates_golden) {
    // Construct a series where SMA will cross above EMA.
    // We prime the MarketData with pre-computed sma/ema values.
    FinancialCalculator::MarketData d;
    d.asset = "CROSS_TEST";
    for (int i = 0; i < 4; ++i) {
        FinancialCalculator::PriceRecord r;
        r.date  = "2024-01-0" + std::to_string(i + 1);
        r.close = 100.0;
        r.volume = 1000LL;
        d.prices.push_back(r);
    }
    // Manually set SMA and EMA so a Golden Cross occurs at index 2.
    // bar0: sma=98, ema=100  -> sma < ema (no cross)
    // bar1: sma=99, ema=100  -> sma < ema
    // bar2: sma=101, ema=100 -> sma > ema, prev sma(99) <= prev ema(100): GOLDEN
    // bar3: sma=101, ema=100 -> still above, no new cross
    d.sma = {98.0, 99.0, 101.0, 101.0};
    d.ema = {100.0, 100.0, 100.0, 100.0};
    ProfitabilityMetrics::findCrossDates(d);

    ASSERT_FALSE(d.goldenCross[0]);
    ASSERT_FALSE(d.goldenCross[1]);
    ASSERT_TRUE(d.goldenCross[2]);
    ASSERT_FALSE(d.goldenCross[3]);
    ASSERT_FALSE(d.deathCross[2]);
}

TEST(test_find_cross_dates_death) {
    FinancialCalculator::MarketData d;
    d.asset = "DEATH_TEST";
    for (int i = 0; i < 3; ++i) {
        FinancialCalculator::PriceRecord r;
        r.date  = "2024-01-0" + std::to_string(i + 1);
        r.close = 100.0;
        r.volume = 1000LL;
        d.prices.push_back(r);
    }
    // bar0: sma=102, ema=100 -> sma > ema
    // bar1: sma=99,  ema=100 -> sma < ema, prev sma(102) >= prev ema(100): DEATH
    // bar2: sma=98,  ema=100 -> still below, no new cross
    d.sma = {102.0, 99.0, 98.0};
    d.ema = {100.0, 100.0, 100.0};
    ProfitabilityMetrics::findCrossDates(d);

    ASSERT_FALSE(d.deathCross[0]);
    ASSERT_TRUE(d.deathCross[1]);
    ASSERT_FALSE(d.deathCross[2]);
    ASSERT_FALSE(d.goldenCross[1]);
}

TEST(test_sma_throws_on_zero_window) {
    auto d = makeData({100.0, 200.0});
    ASSERT_THROWS(ProfitabilityMetrics::calculateSMA(d, 0));
}

// ------------------------------------------------------------------ //
// TradingBot tests                                                     //
// ------------------------------------------------------------------ //

// Build a fully analysed MarketData suitable for TradingBot tests.
static FinancialCalculator::MarketData buildFullData(
        bool goldenCross, bool deathCross,
        double avgVolume,
        double lastDailyReturn, double lastCumReturn,
        double lastSMA, double lastEMA,
        double lastClose, double lastVolume, double prevVolume) {

    using namespace FinancialCalculator;
    MarketData d;
    d.asset = "BOT_TEST";

    // Two bars are enough for the bot (it only inspects last and second-last volume).
    for (int i = 0; i < 2; ++i) {
        PriceRecord r;
        r.date   = "2024-01-0" + std::to_string(i + 1);
        r.close  = lastClose;
        r.volume = (i == 0) ? static_cast<long long>(prevVolume)
                            : static_cast<long long>(lastVolume);
        d.prices.push_back(r);
    }

    d.dailyReturns       = {0.0, lastDailyReturn};
    d.cumulativeReturns  = {0.0, lastCumReturn};
    d.averageVolume      = avgVolume;
    d.sma                = {lastSMA, lastSMA};
    d.ema                = {lastEMA, lastEMA};
    d.goldenCross        = {goldenCross, false};
    d.deathCross         = {deathCross,  false};
    return d;
}

TEST(test_bot_consider_buying_full_conditions) {
    // Golden Cross, high volume, positive return, positive cum return,
    // SMA > EMA, Close > SMA, volume increasing.
    auto d = buildFullData(
        /*golden=*/true, /*death=*/false,
        /*avgVol=*/200000.0,
        /*dailyRet=*/0.01, /*cumRet=*/0.05,
        /*sma=*/110.0, /*ema=*/108.0,
        /*close=*/115.0,
        /*lastVol=*/500000.0, /*prevVol=*/400000.0);
    auto advice = TradingBot::getAdvice(d);
    ASSERT_TRUE(advice.find("Consider buying") != std::string::npos);
}

TEST(test_bot_hold_no_signal) {
    // No crosses.
    auto d = buildFullData(
        /*golden=*/false, /*death=*/false,
        /*avgVol=*/200000.0,
        /*dailyRet=*/0.01, /*cumRet=*/0.05,
        /*sma=*/110.0, /*ema=*/108.0,
        /*close=*/115.0,
        /*lastVol=*/500000.0, /*prevVol=*/400000.0);
    auto advice = TradingBot::getAdvice(d);
    ASSERT_TRUE(advice.find("Hold. No specific") != std::string::npos);
}

TEST(test_bot_consider_selling_full_conditions) {
    // Death Cross, high volume, negative return, negative cum return,
    // SMA < EMA, Close < SMA, volume increasing.
    auto d = buildFullData(
        /*golden=*/false, /*death=*/true,
        /*avgVol=*/200000.0,
        /*dailyRet=*/-0.01, /*cumRet=*/-0.05,
        /*sma=*/108.0, /*ema=*/110.0,
        /*close=*/105.0,
        /*lastVol=*/500000.0, /*prevVol=*/400000.0);
    auto advice = TradingBot::getAdvice(d);
    ASSERT_TRUE(advice.find("Consider selling") != std::string::npos);
}

TEST(test_bot_throws_on_incomplete_data) {
    FinancialCalculator::MarketData d;
    ASSERT_THROWS(TradingBot::getAdvice(d));
}

// ------------------------------------------------------------------ //
// CSV loader tests                                                     //
// ------------------------------------------------------------------ //

TEST(test_csv_loader_valid_file) {
    // Write a temporary CSV file and load it.
    const std::string path = "/tmp/test_trading_bot_data.csv";
    {
        std::ofstream f(path);
        f << "date,open,high,low,close,volume\n"
          << "2024-01-01,100,105,98,103,1000000\n"
          << "2024-01-08,103,108,101,107,1100000\n"
          << "2024-01-15,107,110,105,109,1200000\n";
    }
    auto d = FinancialCalculator::loadFromCSV(path, "TCKR");
    ASSERT_EQ(d.prices.size(), 3u);
    ASSERT_NEAR(d.prices[0].close, 103.0, 1e-9);
    ASSERT_NEAR(d.prices[2].close, 109.0, 1e-9);
    ASSERT_EQ(d.prices[1].volume, 1100000LL);
}

TEST(test_csv_loader_missing_file) {
    ASSERT_THROWS(FinancialCalculator::loadFromCSV("/tmp/nonexistent_xyz_file.csv"));
}

TEST(test_csv_loader_skips_comment_lines) {
    const std::string path = "/tmp/test_trading_bot_comments.csv";
    {
        std::ofstream f(path);
        f << "# This is a comment\n"
          << "date,open,high,low,close,volume\n"
          << "# Another comment\n"
          << "2024-01-01,100,105,98,103,500000\n";
    }
    auto d = FinancialCalculator::loadFromCSV(path);
    ASSERT_EQ(d.prices.size(), 1u);
}

// ------------------------------------------------------------------ //
// Integration test: full pipeline                                      //
// ------------------------------------------------------------------ //

TEST(test_full_pipeline_runs_without_error) {
    using namespace FinancialCalculator;
    using namespace RiskAnalysis;
    using namespace ProfitabilityMetrics;

    // 15 bars with a clear upward trend.
    std::vector<double> closes = {
        100, 103, 106, 110, 108, 112, 115, 118, 116, 120, 122, 119, 124, 127, 130
    };
    std::vector<long long> vols = {
        1000000, 1100000, 1200000, 1300000, 1250000,
        1350000, 1400000, 1500000, 1450000, 1550000,
        1600000, 1500000, 1650000, 1700000, 1800000
    };
    auto data = makeData(closes, vols);

    calculateDailyReturns(data);
    calculateCumulativeReturns(data);
    calculateAverageDailyVolume(data);
    calculateMovingAveragesAndCrosses(data, 5);

    ASSERT_EQ(data.dailyReturns.size(), closes.size());
    ASSERT_EQ(data.cumulativeReturns.size(), closes.size());
    ASSERT_EQ(data.sma.size(), closes.size());
    ASSERT_EQ(data.ema.size(), closes.size());
    ASSERT_EQ(data.goldenCross.size(), closes.size());
    ASSERT_EQ(data.deathCross.size(), closes.size());

    // Running the bot should not throw.
    std::string advice = TradingBot::getAdvice(data);
    ASSERT_FALSE(advice.empty());
}

// ------------------------------------------------------------------ //
// DataFetcher::readScope tests                                         //
// ------------------------------------------------------------------ //

TEST(test_read_scope_basic) {
    const std::string path = "/tmp/test_trading_bot_scope.txt";
    {
        std::ofstream f(path);
        f << "AAPL\n"
          << "MSFT\n"
          << "GOOG\n";
    }
    auto tickers = DataFetcher::readScope(path);
    ASSERT_EQ(tickers.size(), 3u);
    ASSERT_TRUE(tickers[0] == "AAPL");
    ASSERT_TRUE(tickers[1] == "MSFT");
    ASSERT_TRUE(tickers[2] == "GOOG");
}

TEST(test_read_scope_skips_comments_and_blanks) {
    const std::string path = "/tmp/test_trading_bot_scope_comments.txt";
    {
        std::ofstream f(path);
        f << "# crypto\n"
          << "BTC-USD\n"
          << "\n"
          << "# stocks\n"
          << "AAPL\n"
          << "  \n"   // whitespace-only line
          << "TSLA\n";
    }
    auto tickers = DataFetcher::readScope(path);
    ASSERT_EQ(tickers.size(), 3u);
    ASSERT_TRUE(tickers[0] == "BTC-USD");
    ASSERT_TRUE(tickers[2] == "TSLA");
}

TEST(test_read_scope_missing_file_throws) {
    ASSERT_THROWS(DataFetcher::readScope("/tmp/nonexistent_scope_xyz.txt"));
}

TEST(test_read_scope_trims_whitespace) {
    const std::string path = "/tmp/test_trading_bot_scope_trim.txt";
    {
        std::ofstream f(path);
        f << "  AAPL  \n"
          << "\tMSFT\t\n";
    }
    auto tickers = DataFetcher::readScope(path);
    ASSERT_EQ(tickers.size(), 2u);
    ASSERT_TRUE(tickers[0] == "AAPL");
    ASSERT_TRUE(tickers[1] == "MSFT");
}

// ------------------------------------------------------------------ //
// main                                                                 //
// ------------------------------------------------------------------ //

int main() {
    std::cout << "\n=== Trading Bot C++ Unit Tests ===\n\n";

    // Tests self-register and run via static constructors above.
    // (All TEST() macros expand to a struct whose constructor runs the test.)

    std::cout << "\n----------------------------------\n";
    std::cout << " Results: " << g_passed << " passed, "
                              << g_failed << " failed\n";
    std::cout << "==================================\n\n";

    return (g_failed > 0) ? 1 : 0;
}
