idf_component_register(SRCS "src/ESPNOWHandler.cpp" 
INCLUDE_DIRS "src"
REQUIRES "Service" "BetterConsole" "RobotData" "nvs_flash" "WiFiHandler")