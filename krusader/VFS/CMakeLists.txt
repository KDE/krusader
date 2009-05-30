set(VFS_SRCS
    krvfshandler.cpp
    virt_vfs.cpp 
    vfs.cpp 
    vfile.cpp 
    normal_vfs.cpp 
    krpermhandler.cpp 
    krarchandler.cpp 
    preservingcopyjob.cpp 
    virtualcopyjob.cpp 
    ftp_vfs.cpp 
    krquery.cpp
    kiojobwrapper.cpp
    preserveattrcopyjob.cpp
    packjob.cpp
    abstractthreadedjob.cpp)

kde4_add_library(VFS STATIC ${VFS_SRCS})
