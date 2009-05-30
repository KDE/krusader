set(ActionMan_SRCS
    actionman.cpp
    actionproperty.cpp
    addplaceholderpopup.cpp
    useractionlistview.cpp
    useractionpage.cpp)

kde4_add_ui_files(ActionMan_SRCS actionproperty.ui)

kde4_add_library(ActionMan STATIC ${ActionMan_SRCS})
