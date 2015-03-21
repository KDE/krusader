set(Synchronizer_SRCS
    synchronizedialog.cpp
    synchronizer.cpp 
    synchronizergui.cpp 
    feedtolistboxdialog.cpp 
    synchronizertask.cpp 
    synchronizerdirlist.cpp)

add_library(Synchronizer STATIC ${Synchronizer_SRCS})

target_link_libraries(Synchronizer Dialogs)
