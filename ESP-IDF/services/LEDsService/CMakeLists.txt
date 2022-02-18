idf_component_register(
    REQUIRES "Service" "FastLED-idf" "WS2812FX-idf"
    SRCS "src/LEDsService.cpp" 
    INCLUDE_DIRS "src"
)