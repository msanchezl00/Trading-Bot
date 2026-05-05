import yfinance as yf
import financial_analysis
import trading_bot
from moving_averages import calculate_moving_averages_and_crosses
from tabulate import tabulate

# Read the scopes of our trading bot
with open("scope.txt", "r") as fscope:
    scope = fscope.read().splitlines()

# period time 1m, 2m, 5m, 15m, 30m, 60m, 90m, 1h, 1d, 5d, 1wk, 1mo, 3mo
period = "6mo"

# window suele ser de 50 y 200 sesiones
window = 50

# Table header
table_header = ["Asset", "RSI", "Sharpe Ratio", "Trading Advice", "Period"]

# Table data
table_data = []

# Collect data about each asset from yfinance
for asset in scope:
    # Get historical price data for the last 6 months
    data = yf.Ticker(asset).history(period)

    # --- Basic financial analysis ---
    data = financial_analysis.calculate_returns(data)
    data = financial_analysis.calculate_cumulative_returns(data)
    data = financial_analysis.calculate_average_daily_volume(data)

    # --- Advanced financial analysis ---
    data = financial_analysis.calculate_volatility(data)
    data = financial_analysis.calculate_rsi(data)
    data = financial_analysis.calculate_macd(data)
    data = financial_analysis.calculate_bollinger_bands(data)

    # Sharpe Ratio returns a scalar, not a column
    sharpe = financial_analysis.calculate_sharpe_ratio(data)

    # Calculate moving averages and cross signals
    data = calculate_moving_averages_and_crosses(asset, data, window)

    # Use the trading bot to get advice
    advice = trading_bot.bot_advice(data)

    # Extract the latest RSI value for display (default to 50 if column missing)
    rsi_value = round(float(data['RSI'].iloc[-1]) if 'RSI' in data.columns else 50, 1)

    # Add data to the table
    table_data.append([asset, rsi_value, sharpe, advice, period])

# Print the table
print(tabulate(table_data, headers=table_header, tablefmt="fancy_grid"))
