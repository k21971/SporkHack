#!/bin/bash
#
# Ideas and some parts from the original dgl-create-chroot (by joshk@triplehelix.org, modifications by jilles@stack.nl)
# More by <paxed@alt.org>
# More by Michael Andrew Streib <dtype@dtype.org>
# Licensed under the MIT License
# https://opensource.org/licenses/MIT

# autonamed chroot directory. Can rename.
DATESTAMP=`date +%Y%m%d-%H%M%S`
NAO_CHROOT="/opt/nethack/chroot"
# already compiled versions of dgl and nethack
SPORKHACK_GIT="/home/build/SporkHack"
# the user & group from dgamelaunch config file.
USRGRP="games:games"
# fixed data to copy (leave blank to skip)
SP_GIT="/home/build/SporkHack"
# HACKDIR from include/config.h; aka nethack subdir inside chroot
# Make a new one each time save compat is broken
SPSUBDIR="sporkhack-0.6.5"
#for combining xlogfile, etc on minor version bump
#SP_LOG_SYMLINK_TARGET="/sporkhack-0.6.5/var"
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
mkdir -p "$NAO_CHROOT/dgldir/inprogress-sp065"
chown "$USRGRP" "$NAO_CHROOT/dgldir/inprogress-sp065"
mkdir -p "$NAO_CHROOT/dgldir/extrainfo-sp"
chown "$USRGRP" "$NAO_CHROOT/dgldir/extrainfo-sp"

echo "Making $NAO_CHROOT/$SPSUBDIR"
mkdir -p "$NAO_CHROOT/$SPSUBDIR"

SPORKHACKBIN="$SPORKHACK_GIT/src/sporkhack"
if [ -n "$SPORKHACKBIN" -a ! -e "$SPORKHACKBIN" ]; then
  errorexit "Cannot find SporkHack binary $SPORKHACKBIN"
fi

if [ -n "$SPORKHACKBIN" -a -e "$SPORKHACKBIN" ]; then
  echo "Copying $SPORKHACKBIN"
  cd "$NAO_CHROOT/$SPSUBDIR"
  SPBINFILE="`basename $SPORKHACKBIN`-$DATESTAMP"
  cp "$SPORKHACKBIN" "$SPBINFILE"
  ln -fs "$SPBINFILE" sporkhack
  LIBS="$LIBS `findlibs $SPORKHACKBIN`"
  cd "$NAO_CHROOT"
fi

echo "Creating SporkHack variable dir stuff."
mkdir -p "$NAO_CHROOT/$SPSUBDIR/var"

echo "Copying SporkHack playground stuff"
cp "$SPORKHACK_GIT/dat/nhdat" "$NAO_CHROOT/$SPSUBDIR/var"
chmod 644 "$NAO_CHROOT/$SPSUBDIR/var/nhdat"

chown -R "$USRGRP" "$NAO_CHROOT/$SPSUBDIR/var"
mkdir -p "$NAO_CHROOT/$SPSUBDIR/var/save"
chown -R "$USRGRP" "$NAO_CHROOT/$SPSUBDIR/var/save"

#symlink the logs to the symlink target
if [ -z "$SP_LOG_SYMLINK_TARGET" -o ! -e "$NAO_CHROOT$SP_LOG_SYMLINK_TARGET" -o "$SP_LOG_SYMLINK_TARGET" = "/$SPSUBDIR/var" ]; then
  # don't symlink file to itself
  touch "$NAO_CHROOT/$SPSUBDIR/var/logfile"
  chown -R "$USRGRP" "$NAO_CHROOT/$SPSUBDIR/var/logfile"
  touch "$NAO_CHROOT/$SPSUBDIR/var/record"
  chown -R "$USRGRP" "$NAO_CHROOT/$SPSUBDIR/var/record"
  touch "$NAO_CHROOT/$SPSUBDIR/var/xlogfile"
  chown -R "$USRGRP" "$NAO_CHROOT/$SPSUBDIR/var/xlogfile"
  touch "$NAO_CHROOT/$SPSUBDIR/var/livelog"
  chown -R "$USRGRP" "$NAO_CHROOT/$SPSUBDIR/var/livelog"
  touch "$NAO_CHROOT/$SPSUBDIR/var/perm"
  chown -R "$USRGRP" "$NAO_CHROOT/$SPSUBDIR/var/perm"
else
  if [ -f $NAO_CHROOT/$SPSUBDIR/var/xlogfile ]; then
    errorexit "$NAO_CHROOT/$SPSUBDIR/var/xlogfile exists as a regular file. Proceeding will casuse data loss."
  fi
  ln -fs $SP_LOG_SYMLINK_TARGET/xlogfile $NAO_CHROOT/$SPSUBDIR/var
  ln -fs $SP_LOG_SYMLINK_TARGET/livelog $NAO_CHROOT/$SPSUBDIR/var
  ln -fs $SP_LOG_SYMLINK_TARGET/record $NAO_CHROOT/$SPSUBDIR/var
  ln -fs $SP_LOG_SYMLINK_TARGET/logfile $NAO_CHROOT/$SPSUBDIR/var
  ln -fs $SP_LOG_SYMLINK_TARGET/perm $NAO_CHROOT/$SPSUBDIR/var
fi

RECOVER="$SPORKHACK_GIT/util/recover"

if [ -n "$RECOVER" -a -e "$RECOVER" ]; then
  echo "Copying $RECOVER"
  cp "$RECOVER" "$NAO_CHROOT/$SPSUBDIR/var"
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
