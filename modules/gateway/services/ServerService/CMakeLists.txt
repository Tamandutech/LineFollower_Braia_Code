idf_component_register(
    REQUIRES 
        "Service" "esp_http_server" "BetterConsole" "json"

    SRCS 
        "src/ServerService.cpp" 

    INCLUDE_DIRS 
        "src"
)