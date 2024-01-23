idf_component_register(
    REQUIRES "Service" "SensorsService" "SpeedService" "ESP32MotorControl"
    SRCS "src/PIDService.cpp" 
    INCLUDE_DIRS "src"
)