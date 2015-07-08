include_directories(${KF5_INCLUDES_DIRS} ${QT_INCLUDES})

set(Queue_SRCS
    queue.cpp
    queue_mgr.cpp
    queuewidget.cpp
    queuedialog.cpp)

add_library(Queue STATIC ${Queue_SRCS})

target_link_libraries(Queue
    KF5::ConfigCore
    KF5::ConfigGui
    KF5::I18n
    KF5::KIOCore
    KF5::WidgetsAddons
)
