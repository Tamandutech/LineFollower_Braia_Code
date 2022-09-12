idf_component_register(
    SRCS 
        "src/Service.cpp"
        "src/ccondition_variable.cpp"
        "src/cevent_groups.cpp"
        "src/cmem_pool.cpp"
        "src/cmutex.cpp"
        "src/cqueue.cpp"
        "src/cread_write_lock.cpp"
        "src/csemaphore.cpp"
        "src/ctasklet.cpp"
        "src/cthread.cpp"
        "src/ctimer.cpp"
        "src/cworkqueue.cpp"
        
    REQUIRES 
        "RobotData"

    INCLUDE_DIRS 
        "src" "src/Injector"
)