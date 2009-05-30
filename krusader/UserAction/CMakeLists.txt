set(UserAction_SRCS
    kraction.cpp
    expander.cpp
    useractionpopupmenu.cpp
    kractionbase.cpp 
    useraction.cpp)

kde4_add_library(UserAction STATIC ${UserAction_SRCS})
