#!/bin/sh
# Run this to generate all the initial makefiles, etc.

srcdir=`dirname $0`
test -z "$srcdir" && srcdir=.

ORIGDIR=`pwd`
cd $srcdir
PROJECT=mdk
TEST_TYPE=-d
FILE=mixlib

DIE=0

grep "^AM_GNU_GETTEXT" $srcdir/configure.ac >/dev/null && {
  (gettext --version) < /dev/null > /dev/null 2>&1 || {
    echo
    echo "**Error**: You must have \`gettext' installed to compile $PROJECT."
    echo "(0.13.1 or a newer version if it is available)"
    DIE=1
  }
}

(autoconf --version) < /dev/null > /dev/null 2>&1 || {
    echo
    echo "**Error**: You must have autoconf installed to compile $PROJECT."
    echo "(2.53 or a newer version if it is available)"
    DIE=1
}

(automake --version) < /dev/null > /dev/null 2>&1 || {
    echo
    echo "**Error**: You must have automake installed to compile $PROJECT."
    echo "(1.5 or a newer version if it is available)"
    DIE=1
    NO_AUTOMAKE=yes
}

# if no automake, don't bother testing for aclocal
test -n "$NO_AUTOMAKE" || (aclocal --version) < /dev/null > /dev/null 2>&1 || {
  echo
  echo "**Error**: Missing \`aclocal'.  The version of \`automake'"
  echo "installed doesn't appear recent enough."
  DIE=1
}


if test "$DIE" -eq 1; then
    exit 1
fi

test $TEST_TYPE $FILE || {
    echo "You must run this script in the top-level $PROJECT directory"
    exit 1
}

case $CC in
*xlc | *xlc\ * | *lcc | *lcc\ *) am_opt=--include-deps;;
esac

echo "Running gettext's autopoint..."
autopoint --force

echo "Running intltoolize"
intltoolize --copy --force --automake

ACLOCAL_FLAGS="-I m4"
echo "Running aclocal ..."
aclocal $ACLOCAL_FLAGS

# optionally feature autoheader
(autoheader --version)  < /dev/null > /dev/null 2>&1 && autoheader

# changelog file
touch ChangeLog

echo "Running automake --add-missing --gnu $am_opt ..."
automake -a --gnu $am_opt

echo "Running autoconf ..."
autoconf
cd $ORIGDIR
