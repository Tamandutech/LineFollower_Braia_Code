idf_component_register(
    SRCS 
        "src/RobotData.cpp" 
        "src/TrackSegment.cpp"
        "src/PID/TypePID.cpp" 
        "src/PID/dataPID.cpp" 
        "src/Sensor/dataSensor.cpp" 
        "src/Speed/dataSpeed.cpp" 
        "src/Status/RobotStatus.cpp"
        "src/Status/LedStatus.cpp"
        "src/Mapping/dataMapping.cpp"

    REQUIRES
        "DataObjects" "MappingService" "esp_adc"

    INCLUDE_DIRS 
        "src"
        "src/PID"
        "src/Sensor"
        "src/Mapping"
        "src/Speed"
        "src/Status"
)