def _collect_signals(data):
    """Collect all available trading signals and return bullish/bearish scores.

    Each bullish signal adds +1 (or +2 for stronger indicators) to the
    bullish score; each bearish signal adds to the bearish score.
    A list of human-readable signal descriptions is also returned.
    """
    bullish_score = 0
    bearish_score = 0
    signal_details = []

    # --- Cross signals (weighted higher due to significance) ---
    if data['Golden Cross Dates'].any():
        bullish_score += 2
        signal_details.append("Golden Cross detected")
    if data['Death Cross Dates'].any():
        bearish_score += 2
        signal_details.append("Death Cross detected")

    # --- Volume signals ---
    avg_volume = data['Average Volume'].iloc[-1]
    current_volume = data['Volume'].iloc[-1]
    prev_volume = data['Volume'].iloc[-2]
    if avg_volume > 100000 and current_volume > prev_volume:
        bullish_score += 1
        signal_details.append("High and increasing volume")

    # --- Return signals ---
    if data['Daily Returns'].iloc[-1] > 0:
        bullish_score += 1
    elif data['Daily Returns'].iloc[-1] < 0:
        bearish_score += 1

    if data['Cumulative Returns'].iloc[-1] > 0:
        bullish_score += 1
    elif data['Cumulative Returns'].iloc[-1] < 0:
        bearish_score += 1

    # --- Moving-average relationship ---
    if data['SMA'].iloc[-1] > data['EMA'].iloc[-1]:
        bullish_score += 1
    else:
        bearish_score += 1

    if data['Close'].iloc[-1] > data['SMA'].iloc[-1]:
        bullish_score += 1
    else:
        bearish_score += 1

    # --- RSI signals (if calculated) ---
    if 'RSI' in data.columns:
        rsi = data['RSI'].iloc[-1]
        if rsi < 30:
            bullish_score += 2
            signal_details.append(f"RSI oversold ({rsi:.1f})")
        elif rsi > 70:
            bearish_score += 2
            signal_details.append(f"RSI overbought ({rsi:.1f})")

    # --- MACD signals (if calculated) ---
    if 'MACD' in data.columns and 'MACD Signal' in data.columns:
        macd = data['MACD'].iloc[-1]
        macd_signal = data['MACD Signal'].iloc[-1]
        prev_macd = data['MACD'].iloc[-2]
        prev_macd_signal = data['MACD Signal'].iloc[-2]
        if macd > macd_signal and prev_macd <= prev_macd_signal:
            bullish_score += 2
            signal_details.append("MACD bullish crossover")
        elif macd < macd_signal and prev_macd >= prev_macd_signal:
            bearish_score += 2
            signal_details.append("MACD bearish crossover")

    # --- Bollinger Band signals (if calculated) ---
    if 'Bollinger Upper' in data.columns and 'Bollinger Lower' in data.columns:
        close = data['Close'].iloc[-1]
        upper = data['Bollinger Upper'].iloc[-1]
        lower = data['Bollinger Lower'].iloc[-1]
        if close <= lower:
            bullish_score += 1
            signal_details.append("Price at Bollinger lower band (oversold)")
        elif close >= upper:
            bearish_score += 1
            signal_details.append("Price at Bollinger upper band (overbought)")

    return bullish_score, bearish_score, signal_details


def _determine_advice(bullish_score, bearish_score, signal_details):
    """Translate signal scores into a human-readable trading recommendation.

    Uses a threshold-based lookup to avoid nested conditionals:
    each entry is (condition, advice_string).
    The first matching condition wins.
    """
    net_score = bullish_score - bearish_score
    signals_summary = ", ".join(signal_details) if signal_details else "no notable signals"

    thresholds = [
        (net_score >= 5,  f"Strong buy signal. Active indicators: {signals_summary}."),
        (net_score >= 2,  f"Consider buying. Moderate bullish signals: {signals_summary}."),
        (net_score <= -5, f"Strong sell signal. Active indicators: {signals_summary}."),
        (net_score <= -2, f"Consider selling. Moderate bearish signals: {signals_summary}."),
    ]

    for condition, advice in thresholds:
        if condition:
            return advice

    return f"Hold. Mixed or neutral signals ({signals_summary})."


def bot_advice(data):
    """Generate trading advice based on multiple financial indicators.

    Collects all available bullish/bearish signals, scores them, and
    returns a clear recommendation string.
    """
    bullish_score, bearish_score, signal_details = _collect_signals(data)
    return _determine_advice(bullish_score, bearish_score, signal_details)

