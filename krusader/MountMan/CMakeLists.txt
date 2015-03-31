include_directories(${KF5_INCLUDES_DIRS} ${QT_INCLUDES})

set(MountMan_SRCS
    kmountmangui.cpp
    kmountman.cpp)

add_library(MountMan STATIC ${MountMan_SRCS})

target_link_libraries(MountMan Dialogs)
