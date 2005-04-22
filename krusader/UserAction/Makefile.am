if include_libkjsembed
AM_CPPFLAGS = -D__KJSEMBED__
endif

noinst_LIBRARIES = libUserAction.a

INCLUDES = $(all_includes)

libUserAction_a_METASOURCES = AUTO

libUserAction_a_SOURCES = \
	useraction.cpp \
	kraction.cpp \
	expander.cpp \
	useractionxml.cpp \
	useractionproperties.cpp \
	useractionpopupmenu.cpp
