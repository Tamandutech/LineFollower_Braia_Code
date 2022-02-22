idf_component_register(
    REQUIRES "Service" "ESP32MotorControl"
    SRCS "src/MotorsService.cpp" 
    INCLUDE_DIRS "src"
)