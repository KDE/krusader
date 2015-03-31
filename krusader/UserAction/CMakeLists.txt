include_directories(${KF5_INCLUDES_DIRS} ${QT_INCLUDES})

set(UserAction_SRCS
    kraction.cpp
    expander.cpp
    useractionpopupmenu.cpp
    kractionbase.cpp 
    useraction.cpp)

add_library(UserAction STATIC ${UserAction_SRCS})

target_link_libraries(UserAction KF5::CoreAddons KF5::XmlGui KF5::Parts KF5::IconThemes KF5::KDELibs4Support)