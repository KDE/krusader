set(Queue_SRCS
    queue.cpp
    queue_mgr.cpp
    queuewidget.cpp
    queuedialog.cpp)

kde4_add_library(Queue STATIC ${Queue_SRCS})
