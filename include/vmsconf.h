/*	SCCS Id: @(#)vmsconf.h	3.1	92/12/11	*/
/* Copyright (c) Stichting Mathematisch Centrum, Amsterdam, 1985. */
/* NetHack may be freely redistributed.  See license for details. */

#ifdef VMS
#ifndef VMSCONF_H
#define VMSCONF_H

/*
 * Edit these to choose values appropriate for your site.
 * WIZARD is the username allowed to use the debug option of nethack; no harm
 *   is done by leaving it as a username that doesn't exist at your site.
 * HACKDIR can be overridden at run-time with the logical name HACKDIR, as in
 *   $ define hackdir disk$users:[games.nethack]
 * Trailing NULs are present in the default values in order to make some
 *   extra room for patching longer values into an existing executable.
 */
#define Local_WIZARD	"NHWIZARD\0\0\0\0"
#define Local_HACKDIR	"DISK$USERS:[GAMES.NETHACK.3-1-0.PLAY]\0\0\0\0\0\0"

/*
 * This section cleans up the stuff done in config.h so that it
 * shouldn't need to be modified.  It's conservative so that if
 * config.h is actually edited, the changes won't impact us.
 */
#ifdef UNIX
# undef UNIX
#endif
#ifdef HACKDIR
# undef HACKDIR
#endif
#ifdef WIZARD
# undef WIZARD
#endif
#ifdef WIZARD_NAME
# undef WIZARD_NAME
#endif
#define HACKDIR Local_HACKDIR
#ifndef KR1ED
# define WIZARD Local_WIZARD
# define WIZARD_NAME WIZARD
#else
# define WIZARD 1
# define WIZARD_NAME Local_WIZARD
#endif

/* filenames require punctuation to avoid redirection via logical names */
#undef RECORD
#define RECORD	"record;1"	/* scoreboard file (retains high scores) */
#undef LOGFILE
#define LOGFILE	"logfile;0"	/* optional file (records all games) */

#define HLOCK	"perm;1"	/* an empty file used for locking purposes */

/* want compression--for level & save files--performed within NetHack itself */
#ifdef COMPRESS
# undef COMPRESS
#endif
#ifndef ZEROCOMP
# define ZEROCOMP
#endif

/* vision algorithm */
#ifdef VISION_TABLES
# if defined(VAXC) && defined(BRACES)
#  undef BRACES
# endif
# if defined(__GNUC__) && !defined(BRACES)
#  define BRACES		/* put braces around rows of 2d arrays */
# endif
#else	/* not VISION_TABLES */
# define MACRO_CPATH	/* use clear_path macro instead of function */
#endif

/*
 * If nethack.exe will be installed with privilege so that the playground
 * won't need to be left unprotected, define SECURE to suppress a couple
 * of file protection fixups (protection of bones files and ownership of
 * save files).
 */
/* #define SECURE /**/

/*
 * You may define TEXTCOLOR if your system has any terminals that recognize
 * ANSI color sequences of the form ``<ESCAPE>[#;#m'', where the first # is
 * a number between 40 and 47 represented background color, and the second
 * # is a number between 30 and 37 representing the foreground color.
 * GIGI terminals and DECterm windows on color VAXstations support these
 * color escape sequences, as do some 3rd party terminals and many micro
 * computers.
 */
/* #define TEXTCOLOR /**/

/*
 * If you define USE_QIO_INPUT, then you'll get raw characters from the
 * keyboard, not unlike those of the unix version of Nethack.  This will
 * allow you to use the Escape key in normal gameplay, and the appropriate
 * control characters in Wizard mode.  It will work most like the unix version.
 * It will also avoid "<interrupt>" being displayed when ^Y is pressed.
 *
 * Otherwise, the VMS SMG calls will be used.  These calls block use of
 * the escape key, as well as certain control keys, so gameplay is not
 * the same, although the differences are fairly negligible.  You must
 * then use a VTxxx function key or two <escape>s to give an ESC response.
 */
#define USE_QIO_INPUT	/* use SYS$QIOW instead of SMG$READ_KEYSTROKE */

/*
 * If you define MAIL, then NetHack will capture incoming broadcast
 * messages such as "New mail from so-and-so" and "Print job completed,"
 * and then deliver them to the player.  For mail and phone broadcasts
 * a scroll of mail will be created, which when read will cause NetHack
 * to prompt the player for a command to spawn in order to respond.  The
 * latter capability will not be available if SHELL is disabled below.
 * If you undefine MAIL, broadcasts will go straight to the terminal,
 * resulting in disruption of the screen display; use <ctrl/R> to redraw.
 */
#define MAIL		/* enable broadcast trapping */

/*
 * SHELL enables the player to 'escape' into a spawned subprocess via
 * the '!' command.  Logout or attach back to the parent to resume play.
 * If the player attaches back to NetHack, then a subsequent escape will
 * re-attach to the existing subprocess.  Any such subprocess left over
 * at game exit will be deleted by an exit handler.
 * SUSPEND enables someone running NetHack in a subprocess to reconnect
 * to the parent process with the <ctrl/Z> command; this is not very
 * close to Unix job control, but it's better than nothing.
 */
#define SHELL		/* do not delete the '!' command */
#define SUSPEND		/* don't delete the ^Z command, such as it is */

#define RANDOM		/* use sys/share/random.c instead of vaxcrtl rand */

#define FCMASK	0660	/* file creation mask */

/*
 * The remainder of the file should not need to be changed.
 */

#if defined(VAXC) && !defined(ANCIENT_VAXC)
# ifdef volatile
#  undef volatile
# endif
# ifdef const
#  undef const
# endif
#endif

#ifdef __DECC
# define STRICT_REF_DEF	/* used in lev_main.c */
#endif
#ifdef STRICT_REF_DEF
# define DEFINE_OSPEED
#endif

#ifndef alloca
	/* bison generated foo_yacc.c might try to use alloca() */
# ifdef __GNUC__
#  define alloca __builtin_alloca
# else
#  define ALLOCA_HACK	/* used in util/panic.c */
# endif
#endif

#include <time.h>
#if 0	/* <file.h> is missing for old gcc versions; skip it to save time */
#include <file.h>
#else	/* values needed from missing include file */
# define O_RDONLY 0
# define O_WRONLY 1
# define O_RDWR   2
# define O_CREAT 0x200
#endif

#ifndef REDO
# define Getchar nhgetch
#endif
#define tgetch vms_getchar

#include "system.h"

#define index	strchr
#define rindex	strrchr

/* Use the high quality random number routines. */
#if defined(RANDOM)
#define Rand()	random()
#else
#define Rand()	rand()
#endif

#ifndef __GNUC__
#define bcopy(s,d,n)	memcpy((d),(s),(n))	/* vaxcrtl */
#endif
#define abort()		vms_abort()		/* vmsmisc.c */
#define creat(f,m)	vms_creat(f,m)		/* vmsfiles.c */
#define exit(sts)	vms_exit(sts)		/* vmsmisc.c */
#define getuid()	vms_getuid()		/* vmsunix.c */
#define link(f1,f2)	vms_link(f1,f2)		/* vmsfiles.c */
#define open(f,k,m)	vms_open(f,k,m)		/* vmsfiles.c */
/* #define unlink(f0)	vms_unlink(f0)		/* vmsfiles.c */
#ifdef VERYOLD_VMS
#define unlink(f0)	delete(f0)		/* vaxcrtl */
#else
#define unlink(f0)	remove(f0)		/* vaxcrtl, decc$shr */
#endif
#define C$$TRANSLATE(n)	c__translate(n)	/* vmsfiles.c */

/* VMS global names are case insensitive... */
#define An vms_an
#define The vms_the

/* used in several files which don't #include "extern.h" */
extern void FDECL(vms_exit, (int));

#endif	/* VMSCONF_H */
#endif	/* VMS */