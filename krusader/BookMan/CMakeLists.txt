set(BookMan_SRCS
    krbookmark.cpp
    krbookmarkbutton.cpp
    krbookmarkhandler.cpp
    kraddbookmarkdlg.cpp)

kde4_add_library(BookMan STATIC ${BookMan_SRCS})
