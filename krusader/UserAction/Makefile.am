if include_libkjsembed
AM_CPPFLAGS = -D__KJSEMBED__
endif

noinst_LIBRARIES = libUserAction.a

INCLUDES = $(all_includes)

libUserAction_a_METASOURCES = AUTO

libUserAction_a_SOURCES = useraction.cpp \
	kraction.cpp \
	expander.cpp \
	useractionpopupmenu.cpp \
	kractionbase.cpp


METASOURCES = AUTO
noinst_HEADERS = tstring.h
