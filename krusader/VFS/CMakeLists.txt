include_directories(${KF5_INCLUDES_DIRS} ${QT_INCLUDES})

set(VFS_SRCS
    vfilecontainer.cpp
    krvfshandler.cpp
    virt_vfs.cpp
    vfs.cpp
    vfile.cpp
    default_vfs.cpp
    krpermhandler.cpp
    krquery.cpp
    krtrashhandler.cpp
    ../../krArc/krlinecountingprocess.cpp
)

add_library(VFS STATIC ${VFS_SRCS})

target_link_libraries(VFS
    KF5::I18n
    KF5::KIOCore
    KF5::KIOWidgets
)

if(ACL_FOUND)
    target_link_libraries(VFS ${ACL_LIBS})
endif(ACL_FOUND)
