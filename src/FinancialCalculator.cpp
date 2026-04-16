#include "FinancialCalculator.hpp"

#include <fstream>
#include <sstream>
#include <algorithm>
#include <cctype>

namespace FinancialCalculator {

namespace {

// Convert a string to lower-case in-place.
std::string toLower(std::string s) {
    std::transform(s.begin(), s.end(), s.begin(),
                   [](unsigned char c){ return std::tolower(c); });
    return s;
}

// Trim leading/trailing whitespace and surrounding quotes from a token.
std::string trim(const std::string& s) {
    std::size_t start = s.find_first_not_of(" \t\r\n\"");
    std::size_t end   = s.find_last_not_of(" \t\r\n\"");
    if (start == std::string::npos) return "";
    return s.substr(start, end - start + 1);
}

// Split a CSV line on commas, respecting quoted fields.
std::vector<std::string> splitCSV(const std::string& line) {
    std::vector<std::string> fields;
    std::string field;
    bool inQuotes = false;
    for (char c : line) {
        if (c == '"') {
            inQuotes = !inQuotes;
        } else if (c == ',' && !inQuotes) {
            fields.push_back(trim(field));
            field.clear();
        } else {
            field += c;
        }
    }
    fields.push_back(trim(field));
    return fields;
}

} // anonymous namespace

MarketData loadFromCSV(const std::string& filename, const std::string& assetName) {
    std::ifstream file(filename);
    if (!file.is_open()) {
        throw std::runtime_error("FinancialCalculator: cannot open file '" + filename + "'");
    }

    MarketData data;
    data.asset = assetName.empty() ? filename : assetName;

    // Column indices resolved from the header row.
    int idxDate = -1, idxOpen = -1, idxHigh = -1,
        idxLow  = -1, idxClose = -1, idxVolume = -1;
    bool headerParsed = false;

    std::string line;
    int lineNum = 0;
    while (std::getline(file, line)) {
        ++lineNum;
        // Skip blank lines and comment lines.
        if (line.empty() || line[0] == '#') continue;

        auto fields = splitCSV(line);

        if (!headerParsed) {
            // Map header names to column indices.
            for (int i = 0; i < static_cast<int>(fields.size()); ++i) {
                std::string h = toLower(fields[i]);
                if (h == "date" || h == "datetime")  idxDate   = i;
                else if (h == "open")                idxOpen   = i;
                else if (h == "high")                idxHigh   = i;
                else if (h == "low")                 idxLow    = i;
                else if (h == "close" || h == "adj close") idxClose = i;
                else if (h == "volume")              idxVolume = i;
            }
            if (idxClose == -1) {
                throw std::runtime_error(
                    "FinancialCalculator: CSV header missing required 'Close' column.");
            }
            headerParsed = true;
            continue;
        }

        if (fields.empty()) continue;

        PriceRecord rec;
        try {
            if (idxDate   >= 0 && idxDate   < static_cast<int>(fields.size()))
                rec.date   = fields[idxDate];
            if (idxOpen   >= 0 && idxOpen   < static_cast<int>(fields.size()))
                rec.open   = std::stod(fields[idxOpen]);
            if (idxHigh   >= 0 && idxHigh   < static_cast<int>(fields.size()))
                rec.high   = std::stod(fields[idxHigh]);
            if (idxLow    >= 0 && idxLow    < static_cast<int>(fields.size()))
                rec.low    = std::stod(fields[idxLow]);
            if (idxClose  >= 0 && idxClose  < static_cast<int>(fields.size()))
                rec.close  = std::stod(fields[idxClose]);
            if (idxVolume >= 0 && idxVolume < static_cast<int>(fields.size()))
                rec.volume = std::stoll(fields[idxVolume]);
        } catch (const std::exception& e) {
            throw std::runtime_error(
                "FinancialCalculator: parse error on line " +
                std::to_string(lineNum) + ": " + e.what());
        }
        data.prices.push_back(rec);
    }

    if (data.prices.empty()) {
        throw std::runtime_error(
            "FinancialCalculator: no price records found in '" + filename + "'");
    }

    return data;
}

} // namespace FinancialCalculator
