AUTOMAKE_OPTIONS = subdir-objects


libvpp_sources = \
	taka/string.c \
	vpp.c

libvpp_headers = \
	taka/list.h \
	taka/string.h \
	vpp.h

noinst_LTLIBRARIES = libvpp.la
libvpp_la_SOURCES = $(libvpp_sources) $(libvpp_headers)


bin_PROGRAMS = vpp
vpp_DEPENDENCIES = libvpp.la
vpp_LDADD = -lvpp
vpp_SOURCES = main.c
