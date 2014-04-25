#############
# ledcat Makefile.am
# v0.4 - (c) Daniel Hiepler (2010-2014)


# compile for debugging ?
if DEBUG
    DEBUG_CFLAGS = -DDEBUG
endif


bin_PROGRAMS = ledcat

ledcat_SOURCES = \
	version.c \
	ledcat.c \
	cache.c \
	raw.c

EXTRA_DIST = \
	ledcat.h \
	cache.h \
	magick.h \
	raw.h \
	version.h

ledcat_CFLAGS = \
	-Wall -Wextra -Werror -Wno-unused-parameter \
	$(niftyled_CFLAGS) \
	$(DEBUG_CFLAGS)

ledcat_LDADD = \
	$(niftyled_LIBS)


if USE_IMAGEMAGICK
ledcat_SOURCES += magick.c
ledcat_CFLAGS += $(ImageMagick_CFLAGS) -DHAVE_IMAGEMAGICK=1
ledcat_LDADD += $(ImageMagick_LIBS)
endif

