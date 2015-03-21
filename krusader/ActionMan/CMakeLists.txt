set(ActionMan_SRCS
    actionman.cpp
    actionproperty.cpp
    addplaceholderpopup.cpp
    useractionlistview.cpp
    useractionpage.cpp)

qt5_wrap_ui(ActionMan_SRCS actionproperty.ui)

add_library(ActionMan STATIC ${ActionMan_SRCS})
