idf_component_register(
    REQUIRES "Service" "SensorsService" "ESP32MotorControl"
    SRCS "src/PIDService.cpp" 
    INCLUDE_DIRS "src"
)