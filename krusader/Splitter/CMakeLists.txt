include_directories(${KF5_INCLUDES_DIRS} ${QT_INCLUDES})

set(Splitter_SRCS
    crc32.cpp
    splittergui.cpp
    splitter.cpp
    combiner.cpp)

add_library(Splitter STATIC ${Splitter_SRCS})

target_link_libraries(Splitter
    KF5::ConfigCore
    KF5::I18n
    KF5::KIOWidgets
    KF5::WidgetsAddons
)
