# Copyright (C) 2000, 2001, 2002, 2003, 2004, 2005, 2006, 2008, 2009,
#               2010, 2013, 2014 Free Software Foundation, Inc.
#
# This file is free software; as a special exception the author gives
# unlimited permission to copy and/or distribute it, with or without
# modifications, as long as this notice is preserved.
#
# This program is distributed in the hope that it will be useful, but
# WITHOUT ANY WARRANTY, to the extent permitted by law; without even the
# implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.

AC_INIT([GNU MDK],1.2.8,[bug-mdk@gnu.org],mdk)
AC_CONFIG_SRCDIR([mixlib/mix.h])
AM_INIT_AUTOMAKE

AM_CONFIG_HEADER(config.h)

AM_MAINTAINER_MODE

AC_ISC_POSIX
AC_PROG_CC
AM_PROG_CC_STDC
AC_HEADER_STDC

AM_PROG_LEX
AC_PROG_RANLIB

dnl additional c flags
AM_CFLAGS="$AM_CFLAGS -Wall -fno-strict-aliasing"
AC_ARG_WITH(werror,
[  --with-werror         treat compilation warnings as errors],
[case "${withval}" in
  yes) werr=true ;;
  no)  werr=false ;;
  *) AC_MSG_ERROR(bad value ${withval} for --with-werror) ;;
esac], [werr=false])

if test x$werr = xtrue; then
   AM_CFLAGS="$AM_CFLAGS -Werror"
fi

dnl i18n stuff
AM_GNU_GETTEXT_VERSION(0.14)
AM_GNU_GETTEXT
AC_PROG_INTLTOOL([0.37], [no-xml])

GETTEXT_PACKAGE=mdk
AC_SUBST(GETTEXT_PACKAGE)
AC_DEFINE_UNQUOTED(GETTEXT_PACKAGE,"$GETTEXT_PACKAGE", [Package Name for Gettext])

test "x$prefix" = xNONE && prefix=$ac_default_prefix

AH_TEMPLATE(LOCALEDIR,[locale directory])
AC_DEFINE_UNQUOTED(LOCALEDIR, "${prefix}/share/locale")

dnl Check for readline and history
AC_ARG_WITH(readline,
[  --without-readline         build mixvm without readline support],
[case "${withval}" in
  yes) readl=true ;;
  no)  readl=false ;;
  *) AC_MSG_ERROR(bad value ${withval} for --with-readline) ;;
esac], [readl=true])

if test x$readl = xtrue; then
  AC_CHECK_LIB(ncurses, initscr, ,
               [AC_MSG_WARN(Cannot find ncurses lib); readl=false])
fi

if test x$readl = xtrue; then
  AC_CHECK_LIB(readline, readline, ,
               [AC_MSG_WARN(Cannot find GNU readline lib); readl=false])
fi

if test x$readl = xtrue; then
AC_CHECK_LIB(history, add_history, ,
             [AC_MSG_WARN(Cannot find GNU history lib); readl=false])
dnl old versions of readline use completion_matches instead
dnl of rl_completion_matches
AC_CHECK_FUNCS(rl_completion_matches,,)
fi

dnl Check for glib
PKG_CHECK_MODULES(GLIB,glib-2.0 >= 2.0)

dnl Check for guile
AC_ARG_WITH(guile,
[  --without-guile         build MDK without Guile support],
[case "${withval}" in
  yes) wguile=true ;;
  no)  wguile=false ;;
  *) AC_MSG_ERROR(bad value ${withval} for --with-guile) ;;
esac], [wguile=true])

if test x$wguile = xtrue; then
  PKG_CHECK_MODULES(GUILE,guile-2.0 >= 2.0,[wguile=true],[wguile=false])
fi

if test x$wguile = xtrue; then
  LIBS="$LIBS $GUILE_LIBS"
  AM_CFLAGS="$AM_CFLAGS $GUILE_CFLAGS"
  AC_SUBST(MAKE_GUILE)
fi

AM_CONDITIONAL(MAKE_GUILE, test x$wguile = xtrue)

LIBS="$LIBS $GLIB_LIBS"
AM_CFLAGS="$AM_CFLAGS $GLIB_CFLAGS -DG_DISABLE_DEPRECATED"

dnl Check if the gtk gui is required
AC_ARG_ENABLE(gui,
[  --disable-gui          do not build the GTK+ GUI for mixvm (gmixvm)],
[case "${enableval}" in
  yes) gui=true ;;
  no)  gui=false ;;
  *) AC_MSG_ERROR(bad value ${enableval} for --enable-gui) ;;
esac],[gui=true])

dnl Check for gtk+
if test x$gui = xtrue; then
PKG_CHECK_MODULES(GTK, gtk+-2.0 >= 2.6.0 libglade-2.0 >= 2.0.0 pango >= 1.4,
                  [LIBS="$LIBS $GTK_LIBS" AM_CFLAGS="$AM_CFLAGS $GTK_CFLAGS"],
                  [AC_MSG_WARN(Cannot find GTK+/Glade/Pango: the GUI shall not be built)
                  gui=false])
fi

if test x$gui = xtrue; then
AM_CFLAGS="$AM_CFLAGS -DGTK_DISABLE_DEPRECATED -DGDK_DISABLE_DEPRECATED"
fi

dnl Are we in a darwin system?
case "${host_os}" in
 darwin*) darwin_system=true;;
 *) darwin_system=false;
esac

AM_CONDITIONAL(MAKE_GUI, test x$gui = xtrue)
AM_CONDITIONAL(DARWIN_SYSTEM, test x$darwin_system = xtrue)

dnl use the provided getopt if gnu getopt is not available
AC_REPLACE_FUNCS(getopt_long)

AC_SUBST(AM_CFLAGS)

AC_CONFIG_FILES([
Makefile
doc/Makefile
doc/img/Makefile
mixlib/Makefile
mixlib/testsuite/Makefile
mixguile/Makefile
mixutils/Makefile
mixgtk/Makefile
lib/Makefile
misc/Makefile
samples/Makefile
samples/tests/Makefile
po/Makefile.in
intl/Makefile
])

AC_OUTPUT

if test x$readl = xtrue; then
  rlinfo=", with readline support"
fi

if test x$wguile = xtrue; then
  guileinfo=", with guile support"
fi

echo
echo "*** $PACKAGE_NAME $VERSION has been successfully configured. ***"
echo
echo "Type 'make' to build the following utilities:"
echo "    - mixasm (MIX assembler)"
echo "    - mixvm (MIX virtual machine${rlinfo}${guileinfo})"
if test x$gui = xtrue; then
echo "    - gmixvm (mixvm GTK+ GUI${guileinfo})"
fi
if test x$wguile = xtrue; then
echo "    - mixguile (the mixvm guile shell)"
fi
echo
