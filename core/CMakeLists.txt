add_library(envelope SHARED message.cpp)

add_executable(lirch core.cpp plugin_loader.cpp)
target_link_libraries(lirch envelope dl pthread)
