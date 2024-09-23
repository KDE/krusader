set(FileSystem_SRCS
    defaultfilesystem.cpp
    dirlisterinterface.cpp
    fileitem.cpp
    filesystem.cpp
    filesystemprovider.cpp
    krpermhandler.cpp
    krquery.cpp
    krtrashhandler.cpp
    sizecalculator.cpp
    virtualfilesystem.cpp
)

add_library(FileSystem STATIC ${FileSystem_SRCS})

target_link_libraries(FileSystem
    KF6::Archive
    KF6::I18n
    KF6::KIOCore
    KF6::KIOWidgets
    Qt6::Core5Compat # For QRegExp
)

if(ACL_FOUND)
    target_link_libraries(FileSystem ${ACL_LIBS})
endif(ACL_FOUND)
