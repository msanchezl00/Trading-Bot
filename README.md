# TRADING-BOT

Este repositorio contiene el codigo de un bot de trading que da consejos de operaciones basandose en los datos de yfinance y de un periodo de tiempo que tu quieras darle.

## Archivos Principales

- **launcher.py**: Launcher de la aplicacion, donde se obtienen los datos y posteriormente se llamara a los otros modulos para su analisis. La tabla de salida ahora incluye RSI y Sharpe Ratio por activo.

- **financial_analysis.py**: Modulo de analisis financiero. Contiene calculos basicos (retornos diarios, retornos acumulados, volumen promedio) y calculos avanzados:
  - `calculate_volatility` – Volatilidad anualizada (desviacion estandar rodante de los retornos).
  - `calculate_rsi` – Indice de Fuerza Relativa (RSI) para analisis de momentum a corto plazo. RSI < 30 indica posible sobreventa; RSI > 70 indica posible sobrecompra.
  - `calculate_macd` – MACD (Moving Average Convergence Divergence) para seguimiento de tendencias. Incluye linea MACD, linea de señal e histograma.
  - `calculate_bollinger_bands` – Bandas de Bollinger para identificar rangos de volatilidad de precio.
  - `calculate_sharpe_ratio` – Ratio de Sharpe anualizado (retorno ajustado al riesgo).

- **moving_averages.py**: Analisis de medias moviles (EMA, SMA) y deteccion de Golden Cross y Death Cross.

- **trading_bot.py**: Bot refactorizado que usa un sistema de puntuacion de señales en lugar de if-else anidados. Evalua todas las señales disponibles (cruces, volumen, retornos, RSI, MACD, Bollinger Bands), calcula una puntuacion neta y emite una recomendacion clara.

- **scope.txt**: Lista de nombres de los activos a analizar.