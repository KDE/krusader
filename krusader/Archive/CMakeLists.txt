include_directories(${KF5_INCLUDES_DIRS} ${QT_INCLUDES})

set(Archive_SRCS
    abstractthreadedjob.cpp
    krarchandler.cpp
    kr7zencryptionchecker.cpp
    packjob.cpp
    ../../krArc/krarcbasemanager.cpp
    ../../krArc/krlinecountingprocess.cpp
)

add_library(Archive STATIC ${Archive_SRCS})

target_link_libraries(Archive
    KF5::Archive
    KF5::I18n
    KF5::KIOCore
    KF5::KIOWidgets
    KF5::Wallet
)
