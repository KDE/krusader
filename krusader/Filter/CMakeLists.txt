set(Filter_SRCS
    filtersettings.cpp
    advancedfilter.cpp
    generalfilter.cpp
    filtertabs.cpp
    filterdialog.cpp)

kde4_add_library(Filter STATIC ${Filter_SRCS})

target_link_libraries(Filter Dialogs)
