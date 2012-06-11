# configure.ac 
# v0.2 - Daniel Hiepler (2010-2011)

# --------------------------------
# Initialization macros.
# --------------------------------

#release versioning
define([MAJOR_VERSION], 0)
define([MINOR_VERSION], 1)
define([MICRO_VERSION], 0)


# --------------------------------
# AC_INIT
# --------------------------------
AC_INIT([ledcat],[MAJOR_VERSION.MINOR_VERSION.MICRO_VERSION],[https://github.com/niftylight/niftylight/issues],[ledcat],[http://wiki.niftylight.de/ledcat])
AM_INIT_AUTOMAKE(AC_PACKAGE_NAME, AC_PACKAGE_VERSION, AC_PACKAGE_BUGREPORT, AC_PACKAGE_URL)
AC_CONFIG_MACRO_DIR([m4])
AC_LANG([C])

# Test for new silent rules and enable only if they are available
m4_ifdef([AM_SILENT_RULES], [AM_SILENT_RULES([yes])])

# --------------------------------
# Test for libraries
# --------------------------------

# Test for ImageMagick
PKG_CHECK_MODULES(
    ImageMagick, 
    [ImageMagick,MagickWand], 
    [have_imagemagick=1], 
    [have_imagemagick=0 ; AC_MSG_WARN([ImageMagick not found. Multiple image-format decoding will not be compiled.])]
)
AC_DEFINE_UNQUOTED([HAVE_IMAGEMAGICK], $have_imagemagick, [Define to 1 if you have ImageMagick])
AC_SUBST(ImageMagick_CFLAGS)
AC_SUBST(ImageMagick_LIBS)


# Test for libniftyled
PKG_CHECK_MODULES(niftyled, [niftyled], [], [AC_MSG_ERROR([You need libniftyled + development headers installed])])
AC_SUBST(niftyled_CFLAGS)
AC_SUBST(niftyled_LIBS)


# --------------------------------
# Test for system functions
# --------------------------------
AC_FUNC_ALLOCA



AC_CONFIG_HEADERS([config.h])
AM_MAINTAINER_MODE
AC_PROG_CC
AM_PROG_CC_C_O
AC_HEADER_STDC
AC_SEARCH_LIBS([cposix])


AC_CONFIG_FILES([
Makefile
src/Makefile 
])
AC_OUTPUT
