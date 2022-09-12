idf_component_register(
    SRCS 
        "src/DataStorage.cpp"
        "src/DataManager.cpp"
        "src/DataAbstract.cpp"
        "src/DataMap.cpp"

    REQUIRES
        "vfs"
        "fatfs"
        "newlib"

    INCLUDE_DIRS 
        "src"
)