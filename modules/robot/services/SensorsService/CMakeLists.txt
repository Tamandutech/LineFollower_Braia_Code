idf_component_register(
    REQUIRES "Service" "QTRSensors" "LEDsService"
    SRCS "src/SensorsService.cpp" 
    INCLUDE_DIRS "src"
)