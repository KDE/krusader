set(Panel_SRCS
    krpreviews.cpp
    krpreviewjob.cpp
    krcolorcache.cpp 
    krcalcspacedialog.cpp 
    krpopupmenu.cpp 
    krpreviewpopup.cpp 
    krview.cpp 
    krviewitem.cpp 
    panelfunc.cpp
    krpanel.cpp
    listpanel.cpp 
    panelpopup.cpp 
    krselectionmode.cpp 
    krvfsmodel.cpp
    krinterview.cpp
    krinterbriefview.cpp
    krinterdetailedview.cpp
    krinterviewitemdelegate.cpp
    krviewfactory.cpp
    krmousehandler.cpp)

kde4_add_library(Panel STATIC ${Panel_SRCS})

target_link_libraries(Panel Dialogs GUI KViewer ${KDE4_KFILE_LIBS} )


if(LIBKONQ_FOUND)
    target_link_libraries( Panel ${LIBKONQ_LIBRARY} )
endif(LIBKONQ_FOUND)
