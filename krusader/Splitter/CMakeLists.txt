set(Splitter_SRCS
    crc32.cpp
    splittergui.cpp
    splitter.cpp
    combiner.cpp)

kde4_add_library(Splitter STATIC ${Splitter_SRCS})
