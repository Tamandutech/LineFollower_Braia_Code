idf_component_register(
    REQUIRES 
        "Service"
        "console"
        "cmd_system"

    SRCS 
        "src/SerialService.cpp" 
        "src/cmd_wifi.c" 

    INCLUDE_DIRS 
        "src"
)