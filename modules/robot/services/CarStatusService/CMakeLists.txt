idf_component_register(
    REQUIRES "Service" "MappingService" "LEDsService"
    SRCS "src/CarStatusService.cpp" 
    INCLUDE_DIRS "src"
)