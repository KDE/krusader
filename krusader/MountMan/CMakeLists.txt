set(MountMan_SRCS
    kmountmangui.cpp
    kmountman.cpp)

kde4_add_library(MountMan STATIC ${MountMan_SRCS})

target_link_libraries(MountMan Dialogs)
