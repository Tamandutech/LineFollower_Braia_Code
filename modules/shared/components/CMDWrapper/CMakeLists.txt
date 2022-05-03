idf_component_register(
    SRCS 
        "cmd_system/cmd_system.cpp"
        "cmd_param/cmd_param.cpp"
        "cmd_remote/cmd_remote.cpp"
        "cmd_wifi/cmd_wifi.cpp"

    REQUIRES
        "BetterConsole" 
        "spi_flash"
        "ESPNOWHandler"

    INCLUDE_DIRS 
        "cmd_system"
        "cmd_param"
        "cmd_remote"
        "cmd_wifi"
)