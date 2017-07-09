#!/bin/bash
# 
# Ideas and some parts from the original dgl-create-chroot (by joshk@triplehelix.org, modifications by jilles@stack.nl)
# More by <paxed@alt.org>
# More by Michael Andrew Streib <dtype@dtype.org>
# Licensed under the MIT License
# https://opensource.org/licenses/MIT

# autonamed chroot directory. Can rename.
DATESTAMP=`date +%Y%m%d-%H%M%S`
NAO_CHROOT="/opt/nethack/hardfought.org"
# already compiled versions of dgl and nethack
GRUNTHACK_GIT="/home/build/sporkhack"
# the user & group from dgamelaunch config file.
USRGRP="games:games"
# fixed data to copy (leave blank to skip)
GH_GIT="/home/build/sporkhack"
# HACKDIR from include/config.h; aka nethack subdir inside chroot
# Make a new one each time save compat is broken
#GHSUBDIR="sporkhack_new"
GHSUBDIR="sporkhack"
#for combining xlogfile, etc on minor version bump
#GH_LOG_SYMLINK_TARGET="/sporkhack/var"
# END OF CONFIG
##############################################################################

errorexit()
{
    echo "Error: $@" >&2
    exit 1
}

findlibs()
{
  for i in "$@"; do
      if [ -z "`ldd "$i" | grep 'not a dynamic executable'`" ]; then
         echo $(ldd "$i" | awk '{ print $3 }' | egrep -v ^'\(' | grep lib)
         echo $(ldd "$i" | grep 'ld-linux' | awk '{ print $1 }')
      fi
  done
}

set -e

umask 022

echo "Creating inprogress and userdata directories"
mkdir -p "$NAO_CHROOT/dgldir/inprogress-sp"
chown "$USRGRP" "$NAO_CHROOT/dgldir/inprogress-sp"
mkdir -p "$NAO_CHROOT/dgldir/extrainfo-sp"
chown "$USRGRP" "$NAO_CHROOT/dgldir/extrainfo-sp"

echo "Making $NAO_CHROOT/$GHSUBDIR"
mkdir -p "$NAO_CHROOT/$GHSUBDIR"

GRUNTHACKBIN="$GRUNTHACK_GIT/src/sporkhack"
if [ -n "$GRUNTHACKBIN" -a ! -e "$GRUNTHACKBIN" ]; then
  errorexit "Cannot find SporkHack binary $GRUNTHACKBIN"
fi

if [ -n "$GRUNTHACKBIN" -a -e "$GRUNTHACKBIN" ]; then
  echo "Copying $GRUNTHACKBIN"
  cd "$NAO_CHROOT/$GHSUBDIR"
  GHBINFILE="`basename $GRUNTHACKBIN`-$DATESTAMP"
  cp "$GRUNTHACKBIN" "$GHBINFILE"
  ln -fs "$GHBINFILE" sporkhack
  LIBS="$LIBS `findlibs $GRUNTHACKBIN`"
  cd "$NAO_CHROOT"
fi

echo "Copying NetHack playground stuff"
cp "$GRUNTHACK_GIT/dat/nhdat" "$NAO_CHROOT/$GHSUBDIR/var"
chmod 644 "$NAO_CHROOT/$GHSUBDIR/var/nhdat"

echo "Creating NetHack variable dir stuff."
mkdir -p "$NAO_CHROOT/$GHSUBDIR/var"
chown -R "$USRGRP" "$NAO_CHROOT/$GHSUBDIR/var"
mkdir -p "$NAO_CHROOT/$GHSUBDIR/var/save"
chown -R "$USRGRP" "$NAO_CHROOT/$GHSUBDIR/var/save"

#symlink the logs to the symlink target
if [ -z "$GH_LOG_SYMLINK_TARGET" -o ! -e "$NAO_CHROOT$GH_LOG_SYMLINK_TARGET" -o "$GH_LOG_SYMLINK_TARGET" = "/$GHSUBDIR/var" ]; then
  # don't symlink file to itself
  touch "$NAO_CHROOT/$GHSUBDIR/var/logfile"
  chown -R "$USRGRP" "$NAO_CHROOT/$GHSUBDIR/var/logfile"
  touch "$NAO_CHROOT/$GHSUBDIR/var/record"
  chown -R "$USRGRP" "$NAO_CHROOT/$GHSUBDIR/var/record"
  touch "$NAO_CHROOT/$GHSUBDIR/var/xlogfile"
  chown -R "$USRGRP" "$NAO_CHROOT/$GHSUBDIR/var/xlogfile"
  touch "$NAO_CHROOT/$GHSUBDIR/var/livelog"
  chown -R "$USRGRP" "$NAO_CHROOT/$GHSUBDIR/var/livelog"
  touch "$NAO_CHROOT/$GHSUBDIR/var/perm"
  chown -R "$USRGRP" "$NAO_CHROOT/$GHSUBDIR/var/perm"
else
  if [ -f $NAO_CHROOT/$GHSUBDIR/var/xlogfile ]; then
    errorexit "$NAO_CHROOT/$GHSUBDIR/var/xlogfile exists as a regular file. Proceeding will casuse data loss."
  fi
  ln -fs $GH_LOG_SYMLINK_TARGET/xlogfile $NAO_CHROOT/$GHSUBDIR/var
  ln -fs $GH_LOG_SYMLINK_TARGET/livelog $NAO_CHROOT/$GHSUBDIR/var
  ln -fs $GH_LOG_SYMLINK_TARGET/record $NAO_CHROOT/$GHSUBDIR/var
  ln -fs $GH_LOG_SYMLINK_TARGET/logfile $NAO_CHROOT/$GHSUBDIR/var
  ln -fs $GH_LOG_SYMLINK_TARGET/perm $NAO_CHROOT/$GHSUBDIR/var
fi

RECOVER="$GRUNTHACK_GIT/util/recover"

if [ -n "$RECOVER" -a -e "$RECOVER" ]; then
  echo "Copying $RECOVER"
  cp "$RECOVER" "$NAO_CHROOT/$GHSUBDIR/var"
  LIBS="$LIBS `findlibs $RECOVER`"
  cd "$NAO_CHROOT"
fi


LIBS=`for lib in $LIBS; do echo $lib; done | sort | uniq`
echo "Copying libraries:" $LIBS
for lib in $LIBS; do
        mkdir -p "$NAO_CHROOT`dirname $lib`"
        if [ -f "$NAO_CHROOT$lib" ]
	then
		echo "$NAO_CHROOT$lib already exists - skipping."
	else
		cp $lib "$NAO_CHROOT$lib"
	fi
done

echo "Finished."


