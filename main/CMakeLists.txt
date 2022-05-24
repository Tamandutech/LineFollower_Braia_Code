set(app_sources
	"main_robot.cpp"
	"main_gateway.cpp"
)


idf_component_register(
	SRCS ${app_sources}
)

list(
	APPEND 
		REQUIRES 
			"DebugService"
			"RobotData"
			"WiFiHandler"
)