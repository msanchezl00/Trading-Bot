#include "DataFetcher.hpp"

#include <curl/curl.h>

#include <algorithm>
#include <cstring>
#include <ctime>
#include <fstream>
#include <sstream>
#include <stdexcept>
#include <string>
#include <vector>

namespace DataFetcher {

// ------------------------------------------------------------------ //
// libcurl write callback                                               //
// ------------------------------------------------------------------ //

static std::size_t curlWriteCallback(void* ptr, std::size_t size,
                                     std::size_t nmemb, std::string* out) {
    out->append(static_cast<const char*>(ptr), size * nmemb);
    return size * nmemb;
}

// ------------------------------------------------------------------ //
// HTTP GET helper                                                      //
// ------------------------------------------------------------------ //

static std::string httpGet(const std::string& url,
                           const std::string& cookies = "") {
    CURL* curl = curl_easy_init();
    if (!curl) {
        throw std::runtime_error("DataFetcher: failed to initialise libcurl handle.");
    }

    std::string response;

    curl_easy_setopt(curl, CURLOPT_URL,           url.c_str());
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, curlWriteCallback);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA,     &response);
    curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, 1L);
    curl_easy_setopt(curl, CURLOPT_TIMEOUT,       30L);
    curl_easy_setopt(curl, CURLOPT_SSL_VERIFYPEER, 1L);
    curl_easy_setopt(curl, CURLOPT_SSL_VERIFYHOST, 2L);
    // Identify as a standard browser to satisfy Yahoo Finance.
    curl_easy_setopt(curl, CURLOPT_USERAGENT,
                     "Mozilla/5.0 (X11; Linux x86_64) "
                     "AppleWebKit/537.36 (KHTML, like Gecko) "
                     "Chrome/120.0.0.0 Safari/537.36");

    if (!cookies.empty()) {
        curl_easy_setopt(curl, CURLOPT_COOKIE, cookies.c_str());
    }

    CURLcode res = curl_easy_perform(curl);
    long httpCode = 0;
    curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &httpCode);
    curl_easy_cleanup(curl);

    if (res != CURLE_OK) {
        throw std::runtime_error(
            std::string("DataFetcher: network error for '") + url +
            "': " + curl_easy_strerror(res));
    }
    if (httpCode == 404) {
        throw std::runtime_error(
            "DataFetcher: ticker not found (HTTP 404). "
            "Check that the symbol is valid on Yahoo Finance.");
    }
    if (httpCode != 200) {
        throw std::runtime_error(
            "DataFetcher: HTTP " + std::to_string(httpCode) +
            " response from Yahoo Finance.");
    }
    return response;
}

// ------------------------------------------------------------------ //
// Minimal JSON array extractor                                         //
// ------------------------------------------------------------------ //

// Returns the raw text of the first JSON array whose key matches 'key'.
// e.g.  extractArrayText(json, "timestamp") -> "1700000000,1700086400,..."
static std::string extractArrayText(const std::string& json,
                                    const std::string& key) {
    // Look for  "key":[
    std::string needle = "\"" + key + "\":[";
    auto pos = json.find(needle);
    if (pos == std::string::npos) return {};
    pos += needle.size();

    // Find the matching closing bracket (handle nested arrays).
    int depth = 1;
    std::size_t i = pos;
    while (i < json.size() && depth > 0) {
        if (json[i] == '[') ++depth;
        else if (json[i] == ']') --depth;
        ++i;
    }
    if (depth != 0) return {};
    return json.substr(pos, i - pos - 1);
}

// Splits a raw array text on commas and parses each token as double.
// "null" values are replaced with NaN.
static std::vector<double> parseDoubleArray(const std::string& arrayText) {
    std::vector<double> result;
    if (arrayText.empty()) return result;

    std::istringstream ss(arrayText);
    std::string token;
    while (std::getline(ss, token, ',')) {
        // Trim whitespace.
        auto start = token.find_first_not_of(" \t\r\n");
        auto end   = token.find_last_not_of(" \t\r\n");
        if (start == std::string::npos) { result.push_back(0.0); continue; }
        token = token.substr(start, end - start + 1);

        if (token == "null") {
            result.push_back(0.0);
        } else {
            try {
                result.push_back(std::stod(token));
            } catch (...) {
                result.push_back(0.0);
            }
        }
    }
    return result;
}

// Splits a raw array text on commas and parses each token as long long.
static std::vector<long long> parseLongArray(const std::string& arrayText) {
    std::vector<long long> result;
    if (arrayText.empty()) return result;

    std::istringstream ss(arrayText);
    std::string token;
    while (std::getline(ss, token, ',')) {
        auto start = token.find_first_not_of(" \t\r\n");
        auto end   = token.find_last_not_of(" \t\r\n");
        if (start == std::string::npos) { result.push_back(0); continue; }
        token = token.substr(start, end - start + 1);

        if (token == "null") {
            result.push_back(0);
        } else {
            try {
                result.push_back(std::stoll(token));
            } catch (...) {
                result.push_back(0);
            }
        }
    }
    return result;
}

// Convert a Unix timestamp (UTC) to an ISO date string "YYYY-MM-DD".
static std::string timestampToDate(long long ts) {
    std::time_t t = static_cast<std::time_t>(ts);
    char buf[12] = {0};
    // Use gmtime_r when available to be thread-safe; fall back to gmtime.
#ifdef _WIN32
    struct tm tmBuf;
    gmtime_s(&tmBuf, &t);
    std::strftime(buf, sizeof(buf), "%Y-%m-%d", &tmBuf);
#else
    struct tm tmBuf;
    gmtime_r(&t, &tmBuf);
    std::strftime(buf, sizeof(buf), "%Y-%m-%d", &tmBuf);
#endif
    return buf;
}

// ------------------------------------------------------------------ //
// Yahoo Finance JSON parser                                            //
// ------------------------------------------------------------------ //
// The v8/finance/chart endpoint returns JSON with this shape:
//   { "chart": { "result": [{ "timestamp": [...],
//       "indicators": { "quote": [{ "open": [...], "high": [...],
//           "low": [...], "close": [...], "volume": [...] }] } }] } }

static FinancialCalculator::MarketData parseChartJSON(
        const std::string& json, const std::string& ticker) {

    // Check for API-level error.
    if (json.find("\"error\":{\"code\"") != std::string::npos ||
        json.find("\"result\":null")     != std::string::npos) {
        throw std::runtime_error(
            "DataFetcher: Yahoo Finance returned an error for ticker '" +
            ticker + "'. Verify the symbol is correct.");
    }

    auto tsText     = extractArrayText(json, "timestamp");
    auto openText   = extractArrayText(json, "open");
    auto highText   = extractArrayText(json, "high");
    auto lowText    = extractArrayText(json, "low");
    auto closeText  = extractArrayText(json, "close");
    auto volumeText = extractArrayText(json, "volume");

    if (tsText.empty() || closeText.empty()) {
        throw std::runtime_error(
            "DataFetcher: could not parse Yahoo Finance response for '" +
            ticker + "'. The API format may have changed.");
    }

    auto timestamps = parseLongArray(tsText);
    auto opens      = parseDoubleArray(openText);
    auto highs      = parseDoubleArray(highText);
    auto lows       = parseDoubleArray(lowText);
    auto closes     = parseDoubleArray(closeText);
    auto volumes    = parseLongArray(volumeText);

    std::size_t n = timestamps.size();

    FinancialCalculator::MarketData data;
    data.asset = ticker;
    data.prices.reserve(n);

    for (std::size_t i = 0; i < n; ++i) {
        FinancialCalculator::PriceRecord rec;
        rec.date   = timestampToDate(timestamps[i]);
        rec.open   = (i < opens.size())   ? opens[i]   : 0.0;
        rec.high   = (i < highs.size())   ? highs[i]   : 0.0;
        rec.low    = (i < lows.size())    ? lows[i]    : 0.0;
        rec.close  = (i < closes.size())  ? closes[i]  : 0.0;
        rec.volume = (i < volumes.size()) ? volumes[i] : 0LL;
        // Skip bars with zero close (corrupted data).
        if (rec.close == 0.0) continue;
        data.prices.push_back(rec);
    }

    if (data.prices.empty()) {
        throw std::runtime_error(
            "DataFetcher: no valid price records received for '" + ticker + "'.");
    }

    return data;
}

// ------------------------------------------------------------------ //
// Public API                                                           //
// ------------------------------------------------------------------ //

FinancialCalculator::MarketData fetchFromYahooFinance(
        const std::string& ticker, const std::string& period) {

    // Map yfinance's "1wk" alias to the Yahoo Finance range "5d".
    std::string range = (period == "1wk") ? "5d" : period;

    // Yahoo Finance v8 chart JSON endpoint – no crumb required for read-only.
    std::string url =
        "https://query1.finance.yahoo.com/v8/finance/chart/" + ticker +
        "?range=" + range + "&interval=1d&events=history";

    std::string json;
    try {
        json = httpGet(url);
    } catch (const std::runtime_error& e) {
        // Retry on the backup host.
        std::string url2 =
            "https://query2.finance.yahoo.com/v8/finance/chart/" + ticker +
            "?range=" + range + "&interval=1d&events=history";
        json = httpGet(url2);
    }

    return parseChartJSON(json, ticker);
}

std::vector<std::string> readScope(const std::string& filename) {
    std::ifstream file(filename);
    if (!file.is_open()) {
        throw std::runtime_error(
            "DataFetcher: cannot open scope file '" + filename + "'.");
    }

    std::vector<std::string> tickers;
    std::string line;
    while (std::getline(file, line)) {
        // Trim whitespace.
        auto start = line.find_first_not_of(" \t\r\n");
        if (start == std::string::npos) continue;
        auto end = line.find_last_not_of(" \t\r\n");
        line = line.substr(start, end - start + 1);
        // Skip comments and blank lines.
        if (line.empty() || line[0] == '#') continue;
        tickers.push_back(line);
    }
    return tickers;
}

} // namespace DataFetcher
