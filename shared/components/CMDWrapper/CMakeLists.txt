idf_component_register(
    SRCS 
        "cmd_system/cmd_system.cpp"
        "cmd_robot/cmd_robot.cpp"

    REQUIRES
        "BetterConsole" 
        "spi_flash"
        "EspNowProtocol"

    INCLUDE_DIRS 
        "cmd_system"
        "cmd_robot"
)