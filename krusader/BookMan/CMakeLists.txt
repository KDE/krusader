set(BookMan_SRCS
    krbookmark.cpp
    krbookmarkbutton.cpp
    krbookmarkhandler.cpp
    kraddbookmarkdlg.cpp)

add_library(BookMan STATIC ${BookMan_SRCS})

target_link_libraries(BookMan
    KF5::Bookmarks
    KF5::Completion
    KF5::ConfigCore
    KF5::ConfigWidgets
    KF5::CoreAddons
    KF5::I18n
    KF5::IconThemes
    KF5::KIOCore
    KF5::XmlGui
)
