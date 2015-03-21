set(MountMan_SRCS
    kmountmangui.cpp
    kmountman.cpp)

add_library(MountMan STATIC ${MountMan_SRCS})

target_link_libraries(MountMan Dialogs)
