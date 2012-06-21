## Process this file with automake to produce Makefile.in
bin_PROGRAMS = ledcat

ledcat_SOURCES = \
	version.c ledcat.c raw.c magick.c

ledcat_CFLAGS = \
	-Wall \
	$(niftyled_CFLAGS) $(ImageMagick_CFLAGS) 

ledcat_LDADD = \
	$(niftyled_LIBS) $(ImageMagick_LIBS)
