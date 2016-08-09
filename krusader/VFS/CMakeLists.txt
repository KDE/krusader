include_directories(${KF5_INCLUDES_DIRS} ${QT_INCLUDES})

set(VFS_SRCS
    vfilecontainer.cpp
    krvfshandler.cpp
    virt_vfs.cpp
    vfs.cpp
    vfile.cpp
    normal_vfs.cpp
    krpermhandler.cpp
    krarchandler.cpp
    preservingcopyjob.cpp
    ftp_vfs.cpp
    krquery.cpp
    packjob.cpp
    abstractthreadedjob.cpp
    kr7zencryptionchecker.cpp
    ../../krArc/krarcbasemanager.cpp
    ../../krArc/krlinecountingprocess.cpp
)

add_library(VFS STATIC ${VFS_SRCS})

target_link_libraries(VFS
    KF5::Archive
    KF5::I18n
    KF5::KIOCore
    KF5::KIOWidgets
    KF5::Solid
    KF5::Wallet
    KF5::WidgetsAddons
)

if(ACL_FOUND)
    target_link_libraries(VFS ${ACL_LIBS})
endif(ACL_FOUND)
