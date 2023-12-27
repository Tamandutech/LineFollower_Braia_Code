idf_component_register(
    SRCS 
        "src/RobotData.cpp" 
        "src/PID/dataPID.cpp" 
        "src/Sensor/dataSensor.cpp" 
        "src/Speed/dataSpeed.cpp" 
        "src/Status/RobotStatus.cpp" 
        "src/SLatMarks/dataSLatMarks.cpp"

    REQUIRES
        "DataObjects" "MappingService" "esp_adc"

    INCLUDE_DIRS 
        "src"
        "src/PID"
        "src/Sensor"
        "src/SLatMarks"
        "src/Speed"
        "src/Status"
)