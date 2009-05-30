set(Search_SRCS
    krsearchmod.cpp
    krsearchdialog.cpp)

kde4_add_library(Search STATIC ${Search_SRCS})

target_link_libraries(Search Dialogs)
