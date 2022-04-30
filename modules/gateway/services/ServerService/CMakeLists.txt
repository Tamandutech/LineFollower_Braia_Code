idf_component_register(
    REQUIRES 
        "Service" "esp_http_server" "BetterConsole"

    SRCS 
        "src/ServerService.cpp" 

    INCLUDE_DIRS 
        "src"
)