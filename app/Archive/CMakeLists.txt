set(Archive_SRCS
    abstractthreadedjob.cpp
    krarchandler.cpp
    kr7zencryptionchecker.cpp
    packjob.cpp
    ../../plugins/krarc/krarcbasemanager.cpp
    ../../plugins/krarc/krlinecountingprocess.cpp
)

add_library(Archive STATIC ${Archive_SRCS})

target_link_libraries(Archive
    KF5::Archive
    KF5::I18n
    KF5::KIOCore
    KF5::KIOWidgets
    KF5::Wallet
)
