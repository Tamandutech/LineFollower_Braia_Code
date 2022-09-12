idf_component_register(SRCS "commands.cpp"
                            "argtable3/argtable3.c"
                            "linenoise/linenoise.c"
                    INCLUDE_DIRS "."
                    REQUIRES vfs)
