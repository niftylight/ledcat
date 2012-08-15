## Process this file with automake to produce Makefile.in

# compile for debugging ?
if DEBUG
    OPT_CFLAGS = -O0 -g
else
    OPT_CFLAGS = -O2
endif


bin_PROGRAMS = ledcat

ledcat_SOURCES = \
	version.c ledcat.c raw.c magick.c

ledcat_CFLAGS = \
	-Wall -Wextra -Werror -Wno-unused-parameter \
	$(niftyled_CFLAGS) $(ImageMagick_CFLAGS) $(OPT_CFLAGS)

ledcat_LDADD = \
	$(niftyled_LIBS) $(ImageMagick_LIBS)
