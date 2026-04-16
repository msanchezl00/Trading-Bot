cmake_minimum_required(VERSION 3.14)
project(TradingBot VERSION 1.0.0 LANGUAGES CXX)

# -----------------------------------------------------------------------
# Language standard
# -----------------------------------------------------------------------
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# -----------------------------------------------------------------------
# Compiler warnings
# -----------------------------------------------------------------------
if(MSVC)
    add_compile_options(/W4)
else()
    add_compile_options(-Wall -Wextra -Wpedantic)
endif()

# -----------------------------------------------------------------------
# Include directories
# -----------------------------------------------------------------------
include_directories(${PROJECT_SOURCE_DIR}/include)

# -----------------------------------------------------------------------
# Library target – all reusable modules
# -----------------------------------------------------------------------
add_library(trading_bot_lib STATIC
    src/FinancialCalculator.cpp
    src/RiskAnalysis.cpp
    src/ProfitabilityMetrics.cpp
    src/TradingBot.cpp
)

target_include_directories(trading_bot_lib PUBLIC
    ${PROJECT_SOURCE_DIR}/include
)

# -----------------------------------------------------------------------
# Main executable
# -----------------------------------------------------------------------
add_executable(trading_bot src/main.cpp)
target_link_libraries(trading_bot PRIVATE trading_bot_lib)

# -----------------------------------------------------------------------
# Tests (enabled by default; disable with -DBUILD_TESTS=OFF)
# -----------------------------------------------------------------------
option(BUILD_TESTS "Build unit tests" ON)

if(BUILD_TESTS)
    enable_testing()

    add_executable(test_financial tests/test_financial.cpp)
    target_link_libraries(test_financial PRIVATE trading_bot_lib)

    add_test(NAME FinancialTests COMMAND test_financial)
endif()

# -----------------------------------------------------------------------
# Install rules (optional – useful for packaging)
# -----------------------------------------------------------------------
install(TARGETS trading_bot RUNTIME DESTINATION bin)
install(DIRECTORY include/ DESTINATION include/trading_bot)
