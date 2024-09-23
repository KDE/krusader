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
    KF6::Archive
    KF6::I18n
    KF6::KIOCore
    KF6::KIOWidgets
    KF6::Wallet
)
