import yfinance as yf
import pandas as pd
import numpy as np

def get_financial_info(asset, start_date, end_date):
    # Get financial information for a given asset within a specified date range.
    data = yf.Ticker(asset).history(start=start_date, end=end_date)
    return data

def calculate_returns(data):
    # Calculate daily returns for a given DataFrame.
    data['Daily Returns'] = data['Close'].pct_change()
    return data

def calculate_cumulative_returns(data):
    # Calculate cumulative returns for a given DataFrame.
    data['Cumulative Returns'] = (1 + data['Daily Returns']).cumprod() - 1
    return data

def calculate_average_daily_volume(data):
    # Calculate the average daily volume for a given DataFrame.
    data['Average Volume'] = data['Volume'].mean()
    return data

def calculate_volatility(data, window=14):
    # Calculate rolling volatility (annualised standard deviation of daily returns).
    # A higher value means greater price uncertainty – useful for risk assessment.
    # Multiplied by sqrt(252) to annualise (252 = typical trading days in a year).
    data['Volatility'] = data['Daily Returns'].rolling(window=window, min_periods=1).std() * np.sqrt(252)
    return data

def calculate_rsi(data, period=14):
    # Calculate the Relative Strength Index (RSI) for short-term momentum analysis.
    # RSI < 30: asset may be oversold (potential buy opportunity).
    # RSI > 70: asset may be overbought (potential sell signal).
    delta = data['Close'].diff()
    gain = delta.clip(lower=0)
    loss = -delta.clip(upper=0)
    avg_gain = gain.ewm(com=period - 1, min_periods=period).mean()
    avg_loss = loss.ewm(com=period - 1, min_periods=period).mean()
    # Replace zero avg_loss with a tiny value to avoid division by zero;
    # near-zero avg_loss means near-perfect gains, which maps RSI toward 100.
    rs = avg_gain / avg_loss.replace(0, 1e-10)
    data['RSI'] = 100 - (100 / (1 + rs))
    data['RSI'] = data['RSI'].fillna(50)
    return data

def calculate_macd(data, fast_period=12, slow_period=26, signal_period=9):
    # Calculate the MACD (Moving Average Convergence Divergence) indicator.
    # MACD line crossing above the signal line is a bullish signal.
    # MACD line crossing below the signal line is a bearish signal.
    ema_fast = data['Close'].ewm(span=fast_period, adjust=False, min_periods=1).mean()
    ema_slow = data['Close'].ewm(span=slow_period, adjust=False, min_periods=1).mean()
    data['MACD'] = ema_fast - ema_slow
    data['MACD Signal'] = data['MACD'].ewm(span=signal_period, adjust=False, min_periods=1).mean()
    data['MACD Histogram'] = data['MACD'] - data['MACD Signal']
    return data

def calculate_bollinger_bands(data, window=20, num_std=2):
    # Calculate Bollinger Bands to identify price volatility ranges.
    # Price near the lower band may indicate an oversold condition (potential buy).
    # Price near the upper band may indicate an overbought condition (potential sell).
    rolling_mean = data['Close'].rolling(window=window, min_periods=1).mean()
    rolling_std = data['Close'].rolling(window=window, min_periods=1).std().fillna(0)
    data['Bollinger Middle'] = rolling_mean
    data['Bollinger Upper'] = rolling_mean + num_std * rolling_std
    data['Bollinger Lower'] = rolling_mean - num_std * rolling_std
    return data

def calculate_sharpe_ratio(data, risk_free_rate=0.02):
    # Calculate the annualised Sharpe Ratio (risk-adjusted return).
    # A higher Sharpe Ratio means better return per unit of risk.
    # Assumes daily return data; annualises using sqrt(252) trading days.
    daily_returns = data['Daily Returns'].dropna()
    if daily_returns.std() < 1e-10:
        return 0.0
    excess_returns = daily_returns - risk_free_rate / 252
    sharpe = excess_returns.mean() / excess_returns.std() * np.sqrt(252)
    return round(float(sharpe), 4)