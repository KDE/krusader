set(Splitter_SRCS
    crc32.cpp
    splittergui.cpp
    splitter.cpp
    combiner.cpp)

add_library(Splitter STATIC ${Splitter_SRCS})
