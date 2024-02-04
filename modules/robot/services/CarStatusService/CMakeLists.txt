idf_component_register(
    REQUIRES "Service" "MappingService" "SpeedService" "LEDsService"
    SRCS "src/CarStatusService.cpp" 
    INCLUDE_DIRS "src"
)