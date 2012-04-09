# Should envelope should be a static lib?
# 1) It's small and unlikely to grow large
# 2) If it changes, we want plugins (shared libs) to link the new version
# 3) We can enable PIC with the second line, so plugins will build
# Downside is that every plugin will contain it (~MB gain overall)
#add_library(envelope message.cpp)
#set_target_properties(envelope PROPERTIES COMPILE_FLAGS -fPIC)

# Build a shared library that practically everything uses
add_library(envelope SHARED message.cpp)

# Build the core executable
add_executable(lirch core.cpp plugin_loader.cpp)
target_link_libraries(lirch envelope dl pthread)
