set(Filter_SRCS
    filtersettings.cpp
    advancedfilter.cpp
    generalfilter.cpp
    filtertabs.cpp
    filterdialog.cpp)

add_library(Filter STATIC ${Filter_SRCS})

target_link_libraries(Filter
    Dialogs
    KF6::Codecs
    KF6::ConfigCore
    KF6::I18n
    KF6::IconThemes
    KF6::WidgetsAddons
)
