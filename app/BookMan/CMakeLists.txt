set(BookMan_SRCS
    krbookmark.cpp
    krbookmarkbutton.cpp
    krbookmarkhandler.cpp
    kraddbookmarkdlg.cpp)

add_library(BookMan STATIC ${BookMan_SRCS})

target_link_libraries(BookMan
    KF6::Archive
    KF6::Bookmarks
    KF6::BookmarksWidgets
    KF6::Completion
    KF6::ConfigCore
    KF6::ConfigWidgets
    KF6::CoreAddons
    KF6::I18n
    KF6::IconThemes
    KF6::KIOCore
    KF6::XmlGui
    Qt6::Xml
)
