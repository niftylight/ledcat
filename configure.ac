#############
# ledcat configure.ac
# v0.4 - (c) Daniel Hiepler (2010-2012)

# --------------------------------
#    initialization
# --------------------------------

AC_INIT([ledcat], m4_esyscmd([./version --short]),[http://github.com/niftylight/ledcat/issues], [ledcat], [http://wiki.niftylight.de/AC_PACKAGE_NAME])
AC_COPYRIGHT([Copyright (C) 2006-2012 Daniel Hiepler])

AM_INIT_AUTOMAKE([color-tests dist-bzip2 parallel-tests silent-rules -Wall -Wno-portability])
AC_CONFIG_MACRO_DIR([m4])


# --------------------------------
#    information on the package
# --------------------------------
# infos
PACKAGE_DESCRIPTION="niftyled cat tool"
PACKAGE_DESCRIPTION_LONG="cat like tool to pipe pixel data to LED hardware conveniently"

AC_SUBST([PACKAGE_DESCRIPTION], [${PACKAGE_DESCRIPTION}], [package description])
AC_SUBST([PACKAGE_DESCRIPTION_LONG], [${PACKAGE_DESCRIPTION_LONG}], [long package description])
AC_DEFINE_UNQUOTED(PACKAGE_DESCRIPTION, "${PACKAGE_DESCRIPTION}", [package description])
AC_DEFINE_UNQUOTED(PACKAGE_DESCRIPTION_LONG, "${PACKAGE_DESCRIPTION_LONG}", [long package description])

# versioning (s. version script
AC_SUBST([PACKAGE_MAJOR_VERSION], m4_esyscmd([./version --major]), [major version])
AC_SUBST([PACKAGE_MINOR_VERSION], m4_esyscmd([./version --minor]), [minor version])
AC_SUBST([PACKAGE_MICRO_VERSION], m4_esyscmd([./version --micro]), [micro version])
AC_SUBST([PACKAGE_GIT_VERSION], m4_esyscmd([./version --git]), [git version])


# --------------------------------
#    checks for programs
# --------------------------------
AM_PROG_CC_C_O
AC_PROG_CC_STDC
PKG_PROG_PKG_CONFIG([0.14])
if test -z "${PKG_CONFIG}"; then
  AC_MSG_ERROR([*** pkg-config not found. See http://www.freedesktop.org/software/pkgconfig/ or check your distribution.])
fi


# --------------------------------
#    checks for libraries
# --------------------------------

AC_SEARCH_LIBS([cposix])

PKG_CHECK_MODULES(ImageMagick, [ImageMagick,MagickWand], [HAVE_IMAGEMAGICK=1], [HAVE_IMAGEMAGICK=0 ; AC_MSG_RESULT([ImageMagick not found. Will support only RAW pixeldata.])])
AC_SUBST(ImageMagick_CFLAGS)
AC_SUBST(ImageMagick_LIBS)
AM_CONDITIONAL([USE_IMAGEMAGICK], [test $HAVE_IMAGEMAGICK -eq 1])

PKG_CHECK_MODULES(niftyled, [niftyled], [], [AC_MSG_ERROR([You need libniftyled + development headers installed])])
AC_SUBST(niftyled_CFLAGS)
AC_SUBST(niftyled_LIBS)


# --------------------------------
#    checks for header files
# --------------------------------
AC_HEADER_STDC


# --------------------------------
#    checks for types
# --------------------------------


# --------------------------------
#    checks for structures
# --------------------------------


# --------------------------------
#    checks for compiler characteristics
# --------------------------------


# --------------------------------
#    checks for library functions
# --------------------------------
AC_FUNC_ALLOCA


# --------------------------------
#    checks for system services
# --------------------------------


# --------------------------------
#   compile process
# --------------------------------
m4_ifdef([AM_SILENT_RULES], [AM_SILENT_RULES([yes])])


# --------------------------------
# Arguments for configure-script
# --------------------------------
AC_ARG_ENABLE(
        debug,
		AS_HELP_STRING([--enable-debug], [enable debugging, default: no]),
		[case "${enableval}" in
             yes) debug=true ;;
             no)  debug=false ;;
             *)   AC_MSG_ERROR([bad value ${enableval} for --enable-debug]) ;;
		esac],
		[debug=false])
AM_CONDITIONAL(DEBUG, test x$debug = xtrue)

# use ImageMagick to decode pixeldata
AC_ARG_ENABLE(
	imagemagick,
	AS_HELP_STRING([--enable-imagemagick], [enable pixel decoding using ImageMagick]),
	[ if test x$enableval = xno ; then WANT_IMAGEMAGICK=false ; else if test $HAVE_IMAGEMAGICK -eq 1 ; then WANT_IMAGEMAGICK=true ; else AC_MSG_ERROR([ImageMagick requested but development headers not found.]) ; fi ; fi ],
	[ WANT_IMAGEMAGICK=true ])
AM_CONDITIONAL([USE_IMAGEMAGICK], [test "x$WANT_IMAGEMAGICK" = xtrue && test $HAVE_IMAGEMAGICK -eq 1])


if test "x$WANT_IMAGEMAGICK" = xtrue && test $HAVE_IMAGEMAGICK -eq 1 ; then
	MSG_IMAGEMAGICK="enabled"
else
	MSG_IMAGEMAGICK="disabled - only supporting RAW pixelformats"
fi


# --------------------------------
# Output
# --------------------------------
AC_CONFIG_HEADERS([config.h])
AC_CONFIG_FILES(
[
	Makefile
	src/version.c
	src/Makefile
	gentoo/media-gfx/ledcat/ledcat-9999.ebuild
])
AC_OUTPUT


echo "
Configure report:

    Package.....................:  ${PACKAGE_STRING}
    Description.................:  ${PACKAGE_DESCRIPTION}
    Revision....................:  ${PACKAGE_GIT_VERSION}
    URL.........................:  ${PACKAGE_URL}
    Bugreports..................:  ${PACKAGE_BUGREPORT}
    ImageMagick.................:  ${MSG_IMAGEMAGICK}

    Install prefix..............:  ${prefix}
    C compiler..................:  ${CC}
    C++ compiler................:  ${CPP}
    System CFLAGS...............:  ${CFLAGS}
    System CXXFLAGS.............:  ${CXXFLAGS}
    System LDFLAGS..............:  ${LDFLAGS}
"
