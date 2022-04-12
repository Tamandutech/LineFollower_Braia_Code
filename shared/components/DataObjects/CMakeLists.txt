idf_component_register(
    SRCS 
        "src/DataStorage.cpp"
        "src/DataAbstract.cpp"
        "src/DataManager.cpp"

    REQUIRES
        "vfs"
        "fatfs"

    INCLUDE_DIRS 
        "src"
)