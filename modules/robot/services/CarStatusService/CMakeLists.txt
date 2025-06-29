idf_component_register(
    REQUIRES "Service" "SpeedService" "MappingService" "LEDsService"
    SRCS "src/CarStatusService.cpp" 
    INCLUDE_DIRS "src"
)