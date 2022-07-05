idf_component_register(
    REQUIRES 
        "Service" "bt" "esp-nimble-cpp"

    SRCS 
        "src/BLEServerService.cpp"
    
    INCLUDE_DIRS 
        "src"
)