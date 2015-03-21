set(Search_SRCS
    krsearchmod.cpp
    krsearchdialog.cpp)

add_library(Search STATIC ${Search_SRCS})

target_link_libraries(Search Dialogs)
