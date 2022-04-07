idf_component_register(
    SRCS 
        "src/RobotData.cpp" 
        "src/dataTypes/DataAbstract.cpp"
        "src/PID/dataPID.cpp" 
        "src/Sensor/dataSensor.cpp" 
        "src/Speed/dataSpeed.cpp" 
        "src/Status/RobotStatus.cpp" 
        "src/SLatMarks/dataSLatMarks.cpp"

    REQUIRES
        "vfs"
        "fatfs"

    INCLUDE_DIRS 
        "src"
        "src/dataTypes"
        "src/PID"
        "src/Sensor"
        "src/SLatMarks"
        "src/Speed"
        "src/Status"
)