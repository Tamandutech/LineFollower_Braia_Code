idf_component_register(
    SRCS 
        "cmd_system/cmd_system.cpp"
        "cmd_param/cmd_param.cpp"
        "cmd_datamanager/cmd_datamanager.cpp"
        "cmd_runtime/cmd_runtime.cpp"

    REQUIRES
        "BetterConsole" 
        "spi_flash"
        "esp_adc"
        "SensorsService"
        "CarStatusService"

    INCLUDE_DIRS 
        "cmd_system"
        "cmd_param"
        "cmd_datamanager"
        "cmd_runtime"
)