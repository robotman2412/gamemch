idf_component_register(
    SRCS
        "main.cpp"
        "espnowwrapper.cpp"
        "connection.cpp"
        "player.cpp"
        "graphics.cpp"
    INCLUDE_DIRS
        "." "include"
    EMBED_FILES ${project_dir}/resources/pattern0.png
    EMBED_FILES ${project_dir}/resources/pattern1.png
)