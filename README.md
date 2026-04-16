# TRADING-BOT

Este repositorio contiene el código de un bot de trading que da consejos de operaciones basándose en datos de mercado históricos (OHLCV). Existe una implementación original en **Python** y una implementación portada a **C++17** con estructura de clases y módulos.

---

## Módulo C++ (nuevo)

### Estructura del Proyecto

```
├── include/                     # Cabeceras públicas (.hpp)
│   ├── FinancialCalculator.hpp  # Estructuras de datos comunes y cargador CSV
│   ├── DataFetcher.hpp          # Obtención de datos en vivo desde Yahoo Finance API
│   ├── RiskAnalysis.hpp         # Métricas de riesgo (retornos, volumen)
│   ├── ProfitabilityMetrics.hpp # Medias móviles y detección de cruces
│   └── TradingBot.hpp           # Lógica de consejo de trading
├── src/                         # Implementaciones (.cpp)
│   ├── FinancialCalculator.cpp
│   ├── DataFetcher.cpp
│   ├── RiskAnalysis.cpp
│   ├── ProfitabilityMetrics.cpp
│   ├── TradingBot.cpp
│   └── main.cpp                 # Launcher equivalente a launcher.py
├── tests/
│   └── test_financial.cpp       # Pruebas unitarias sin frameworks externos
├── CMakeLists.txt               # Sistema de construcción CMake
└── README.md
```

### Descripción de Módulos

| Módulo | Namespace | Responsabilidad |
|---|---|---|
| `FinancialCalculator` | `FinancialCalculator` | Estructuras de datos (`PriceRecord`, `MarketData`) y cargador CSV |
| `DataFetcher` | `DataFetcher` | Descarga datos OHLCV en vivo desde Yahoo Finance; lee `scope.txt` |
| `RiskAnalysis` | `RiskAnalysis` | Retornos diarios, retornos acumulados, volumen promedio |
| `ProfitabilityMetrics` | `ProfitabilityMetrics` | SMA, EMA, Golden Cross, Death Cross |
| `TradingBot` | `TradingBot` | Consejo de trading multi-factor |

---

### Requisitos

| Herramienta | Versión mínima |
|---|---|
| Compilador C++ | GCC 9+, Clang 10+ o MSVC 2019+ |
| Estándar C++ | C++17 |
| CMake | 3.14+ |
| libcurl | 7.x+ (para descarga de datos en vivo; `libcurl4-openssl-dev` en Ubuntu/Debian) |

---

### Instalación de dependencias (Ubuntu/Debian)

```bash
sudo apt-get install -y build-essential cmake libcurl4-openssl-dev
```

---

### Compilación

#### Opción A – CMake (recomendado)

```bash
# Desde la raíz del repositorio
mkdir build && cd build
cmake .. -DCMAKE_BUILD_TYPE=Release
cmake --build .
```

#### Opción B – compilación directa con g++

```bash
g++ -std=c++17 -O2 \
    -Iinclude \
    src/FinancialCalculator.cpp \
    src/DataFetcher.cpp \
    src/RiskAnalysis.cpp \
    src/ProfitabilityMetrics.cpp \
    src/TradingBot.cpp \
    src/main.cpp \
    -lcurl -o trading_bot
```

---

### Ejecución

El ejecutable principal replica exactamente el comportamiento de `launcher.py`:

```bash
# Modo por defecto: lee scope.txt y descarga datos en vivo de Yahoo Finance
./trading_bot

# Usar un scope diferente
./trading_bot --scope mi_lista.txt

# Cambiar período (por defecto: 1mo)
# Valores: 1d 5d 1wk 1mo 3mo 6mo 1y 2y 5y ytd max
./trading_bot --period 3mo

# Cambiar ventana SMA/EMA (por defecto: 50)
./trading_bot --window 200

# Combinar opciones
./trading_bot --period 6mo --window 50 --scope scope.txt

# Modo demo con datos sintéticos integrados (sin red)
./trading_bot --demo

# Cargar datos desde un archivo CSV local
./trading_bot --csv data/AAPL.csv --ticker AAPL --window 50
```

#### Salida esperada (modo por defecto con scope.txt)

```
Fetching data for 25 asset(s)  (period=1mo, window=50) ...

+----------+-------------------------------------------------------+--------+
| Asset    | Trading Advice                                        | Period |
+==========+=======================================================+========+
| BTC-USD  | Consider buying. Recent Golden Cross, high average... | 1mo    |
+----------+-------------------------------------------------------+--------+
| AAPL     | Hold. No specific trading signals based on the...     | 1mo    |
+----------+-------------------------------------------------------+--------+
| ...      | ...                                                   | ...    |
+----------+-------------------------------------------------------+--------+
```

---

### Ejecutar las pruebas unitarias

```bash
# Con CMake (desde la carpeta build)
ctest --output-on-failure

# O directamente
g++ -std=c++17 -O2 \
    -Iinclude \
    src/FinancialCalculator.cpp \
    src/DataFetcher.cpp \
    src/RiskAnalysis.cpp \
    src/ProfitabilityMetrics.cpp \
    src/TradingBot.cpp \
    tests/test_financial.cpp \
    -lcurl -o test_financial && ./test_financial
```

---

### Formato del archivo CSV de entrada (modo `--csv`)

```
date,open,high,low,close,volume
2024-01-01,100.0,105.0,98.0,103.0,1000000
2024-01-08,103.0,108.0,101.0,107.0,1100000
...
```

Las líneas que comienzan con `#` se tratan como comentarios y se omiten.

---

### Ejemplo de uso de las clases C++

```cpp
#include "DataFetcher.hpp"
#include "RiskAnalysis.hpp"
#include "ProfitabilityMetrics.hpp"
#include "TradingBot.hpp"
#include <iostream>

int main() {
    // 1. Obtener datos en vivo de Yahoo Finance (equivalente a yf.Ticker("AAPL").history("1mo"))
    auto data = DataFetcher::fetchFromYahooFinance("AAPL", "1mo");

    // 2. Calcular métricas de riesgo
    RiskAnalysis::calculateDailyReturns(data);
    RiskAnalysis::calculateCumulativeReturns(data);
    RiskAnalysis::calculateAverageDailyVolume(data);

    // 3. Calcular medias móviles y cruces (ventana de 50 sesiones)
    ProfitabilityMetrics::calculateMovingAveragesAndCrosses(data, 50);

    // 4. Obtener consejo de trading
    std::string advice = TradingBot::getAdvice(data);
    std::cout << "AAPL: " << advice << '\n';
}
```

---

## Módulo Python (original)

### Archivos Principales

- **launcher.py**: Launcher de la aplicación. Obtiene datos vía `yfinance` y orquesta los demás módulos.
- **financial_analysis.py**: Análisis financiero básico: volumen promedio, retornos diarios y acumulados.
- **moving_averages.py**: Análisis sobre EMA, SMA y detección de Golden & Death Crosses.
- **trading_bot.py**: Bot que analiza la información procesada y emite un consejo de operación.
- **scope.txt**: Lista de tickers a analizar.

### Dependencias Python

```bash
pip install yfinance pandas matplotlib tabulate
```

### Ejecución Python

```bash
python launcher.py
```