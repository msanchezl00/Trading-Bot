# TRADING-BOT

Este repositorio contiene el código de un bot de trading que da consejos de operaciones basándose en datos de mercado históricos (OHLCV). Existe una implementación original en **Python** y una implementación portada a **C++17** con estructura de clases y módulos.

---

## Módulo C++ (nuevo)

### Estructura del Proyecto

```
├── include/                     # Cabeceras públicas (.hpp)
│   ├── FinancialCalculator.hpp  # Estructuras de datos comunes y cargador CSV
│   ├── RiskAnalysis.hpp         # Métricas de riesgo (retornos, volumen)
│   ├── ProfitabilityMetrics.hpp # Medias móviles y detección de cruces
│   └── TradingBot.hpp           # Lógica de consejo de trading
├── src/                         # Implementaciones (.cpp)
│   ├── FinancialCalculator.cpp
│   ├── RiskAnalysis.cpp
│   ├── ProfitabilityMetrics.cpp
│   ├── TradingBot.cpp
│   └── main.cpp                 # Punto de entrada con demo incluida
├── tests/
│   └── test_financial.cpp       # Pruebas unitarias sin dependencias externas
├── CMakeLists.txt               # Sistema de construcción CMake
└── README.md
```

### Descripción de Módulos

| Módulo | Namespace | Responsabilidad |
|---|---|---|
| `FinancialCalculator` | `FinancialCalculator` | Estructuras de datos (`PriceRecord`, `MarketData`) y cargador CSV |
| `RiskAnalysis` | `RiskAnalysis` | Retornos diarios, retornos acumulados, volumen promedio |
| `ProfitabilityMetrics` | `ProfitabilityMetrics` | SMA, EMA, Golden Cross, Death Cross |
| `TradingBot` | `TradingBot` | Consejo de trading multi-factor |

---

### Requisitos

| Herramienta | Versión mínima |
|---|---|
| Compilador C++ | GCC 9+, Clang 10+ o MSVC 2019+ |
| Estándar C++ | C++17 |
| CMake | 3.14+ (opcional; se puede compilar directamente con `g++`) |

---

### Compilación y Ejecución

#### Opción A – CMake (recomendado)

```bash
# Desde la raíz del repositorio
mkdir build && cd build
cmake .. -DCMAKE_BUILD_TYPE=Release
cmake --build .

# Ejecutar con dataset de demostración integrado
./trading_bot

# Ejecutar con un archivo CSV propio
./trading_bot ruta/a/datos.csv TICKER WINDOW
# Ejemplo: ./trading_bot data/AAPL.csv AAPL 50
```

#### Opción B – compilación directa con g++

```bash
g++ -std=c++17 -O2 \
    -Iinclude \
    src/FinancialCalculator.cpp \
    src/RiskAnalysis.cpp \
    src/ProfitabilityMetrics.cpp \
    src/TradingBot.cpp \
    src/main.cpp \
    -o trading_bot

./trading_bot
```

#### Ejecutar las pruebas unitarias

```bash
# Con CMake
cd build && ctest --output-on-failure

# O directamente
g++ -std=c++17 -O2 \
    -Iinclude \
    src/FinancialCalculator.cpp \
    src/RiskAnalysis.cpp \
    src/ProfitabilityMetrics.cpp \
    src/TradingBot.cpp \
    tests/test_financial.cpp \
    -o test_financial && ./test_financial
```

---

### Formato del archivo CSV de entrada

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
#include "FinancialCalculator.hpp"
#include "RiskAnalysis.hpp"
#include "ProfitabilityMetrics.hpp"
#include "TradingBot.hpp"
#include <iostream>

int main() {
    // 1. Cargar datos desde CSV
    auto data = FinancialCalculator::loadFromCSV("AAPL.csv", "AAPL");

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