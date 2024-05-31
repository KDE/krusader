set(Splitter_SRCS
    crc32.cpp
    splittergui.cpp
    splitter.cpp
    combiner.cpp)

add_library(Splitter STATIC ${Splitter_SRCS})

target_link_libraries(Splitter
    KF6::ConfigCore
    KF6::I18n
    KF6::KIOWidgets
    KF6::WidgetsAddons
)
