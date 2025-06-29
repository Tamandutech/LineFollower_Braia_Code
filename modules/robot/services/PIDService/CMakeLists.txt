idf_component_register(
    REQUIRES "Service" "SpeedService" "SensorsService" "ESP32MotorControl"
    SRCS "src/PIDService.cpp" 
    INCLUDE_DIRS "src"
)