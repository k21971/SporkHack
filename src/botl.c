/*	SCCS Id: @(#)botl.c	3.4	1996/07/15	*/
/* Copyright (c) Stichting Mathematisch Centrum, Amsterdam, 1985. */
/* NetHack may be freely redistributed.  See license for details. */

#include "hack.h"

#ifdef TTY_GRAPHICS
#include "wintty.h"
#endif

#ifdef OVL0
extern const char *hu_stat[];	/* defined in eat.c */

const char * const enc_stat[] = {
	"",
	"Burdened",
	"Stressed",
	"Strained",
	"Overtaxed",
	"Overloaded"
};

STATIC_DCL void NDECL(bot1);
STATIC_DCL void NDECL(bot2);
#endif /* OVL0 */

/* MAXCO must hold longest uncompressed status line, and must be larger
 * than COLNO
 *
 * longest practical second status line at the moment is
 *	Astral Plane $:12345 HP:700(700) Pw:111(111) AC:-127 Xp:30/123456789
 *	T:123456 Satiated Conf FoodPois Ill Blind Stun Hallu Overloaded
 * -- or somewhat over 130 characters
 */
#if COLNO <= 140
#define MAXCO 160
#else
#define MAXCO (COLNO+20)
#endif

#if defined(STATUS_COLORS) && defined(TEXTCOLOR)

extern const struct percent_color_option *hp_colors;
extern const struct percent_color_option *pw_colors;
extern const struct text_color_option *text_colors;

struct color_option
text_color_of(text, color_options)
const char *text;
const struct text_color_option *color_options;
{
	if (color_options == NULL) {
		struct color_option result = {NO_COLOR, 0};
		return result;
	}
	if (strstri(color_options->text, text)
	 || strstri(text, color_options->text))
		return color_options->color_option;
	return text_color_of(text, color_options->next);
}

struct color_option
percentage_color_of(value, max, color_options)
int value, max;
const struct percent_color_option *color_options;
{
	if (color_options == NULL) {
		struct color_option result = {NO_COLOR, 0};
		return result;
	}
	if (100 * value <= color_options->percentage * max)
		return color_options->color_option;
	return percentage_color_of(value, max, color_options->next);
}

void
start_color_option(color_option)
struct color_option color_option;
{
	int i;
	if (color_option.color != NO_COLOR)
		term_start_color(color_option.color);
	for (i = 0; (1 << i) <= color_option.attr_bits; ++i)
		if (i != ATR_NONE && color_option.attr_bits & (1 << i))
			term_start_attr(i);
}

void
end_color_option(color_option)
struct color_option color_option;
{
	int i;
	if (color_option.color != NO_COLOR)
		term_end_color();
	for (i = 0; (1 << i) <= color_option.attr_bits; ++i)
		if (i != ATR_NONE && color_option.attr_bits & (1 << i))
			term_end_attr(i);
}

void
apply_color_option(color_option, str, x)
struct color_option color_option;
const char *str;
int x;
{
	if (x >= CO) return;
	curs(WIN_STATUS, x, 1);
	if (iflags.use_status_colors) start_color_option(color_option);
	putstr(WIN_STATUS, 0, str);
	if (iflags.use_status_colors) end_color_option(color_option);
}

void
add_colored_text(text, x)
const char *text;
int x;
{
	if (*text == '\0') return;
	apply_color_option(text_color_of(text, text_colors), text, x);
}

#endif

#ifndef OVLB
STATIC_DCL int mrank_sz;
#else /* OVLB */
STATIC_OVL NEARDATA int mrank_sz = 0; /* loaded by max_rank_sz (from u_init) */
#endif /* OVLB */

STATIC_DCL const char *NDECL(rank);

#ifdef OVL1

/* convert experience level (1..30) to rank index (0..8) */
int
xlev_to_rank(xlev)
int xlev;
{
	return (xlev <= 2) ? 0 : (xlev <= 30) ? ((xlev + 2) / 4) : 8;
}

#if 0	/* not currently needed */
/* convert rank index (0..8) to experience level (1..30) */
int
rank_to_xlev(rank)
int rank;
{
	return (rank <= 0) ? 1 : (rank <= 8) ? ((rank * 4) - 2) : 30;
}
#endif

const char *
rank_of(lev, monnum, female)
	int lev;
	short monnum;
	boolean female;
{
	register struct Role *role;
	register int i;

	/* Find the role */
	for (role = (struct Role *) roles; role->name.m; role++)
	    if (monnum == role->malenum || monnum == role->femalenum)
	    	break;
	if (!role->name.m)
	    role = &urole;

	/* Gratuitous hack */
	if (u.ualign.type == A_CHAOTIC && Role_if(PM_KNIGHT)) {
		role = &urole;
	}

	/* Find the rank */
	for (i = xlev_to_rank((int)lev); i >= 0; i--) {
	    if (female && role->rank[i].f) return (role->rank[i].f);
	    if (role->rank[i].m) return (role->rank[i].m);
	}

	/* Try the role name, instead */
	if (female && role->name.f) return (role->name.f);
	else if (role->name.m) return (role->name.m);
	return ("Player");
}


STATIC_OVL const char *
rank()
{
	return(rank_of(u.ulevel, Role_switch, flags.female));
}

int
title_to_mon(str, rank_indx, title_length)
const char *str;
int *rank_indx, *title_length;
{
	register int i, j;


	/* Loop through each of the roles */
	for (i = 0; roles[i].name.m; i++)
	    for (j = 0; j < 9; j++) {
	    	if (roles[i].rank[j].m && !strncmpi(str,
	    			roles[i].rank[j].m, strlen(roles[i].rank[j].m))) {
	    	    if (rank_indx) *rank_indx = j;
	    	    if (title_length) *title_length = strlen(roles[i].rank[j].m);
	    	    return roles[i].malenum;
	    	}
	    	if (roles[i].rank[j].f && !strncmpi(str,
	    			roles[i].rank[j].f, strlen(roles[i].rank[j].f))) {
	    	    if (rank_indx) *rank_indx = j;
	    	    if (title_length) *title_length = strlen(roles[i].rank[j].f);
	    	    return ((roles[i].femalenum != NON_PM) ?
	    	    		roles[i].femalenum : roles[i].malenum);
	    	}
	    }
	return NON_PM;
}

#endif /* OVL1 */
#ifdef OVLB

void
max_rank_sz()
{
	register int i, r, maxr = 0;
	for (i = 0; i < 9; i++) {
	    if (urole.rank[i].m && (r = strlen(urole.rank[i].m)) > maxr) maxr = r;
	    if (urole.rank[i].f && (r = strlen(urole.rank[i].f)) > maxr) maxr = r;
	}
	mrank_sz = maxr;
	return;
}

#endif /* OVLB */
#ifdef OVL0

#ifdef SCORE_ON_BOTL
long
botl_score()
{
    int deepest = deepest_lev_reached(FALSE);
#ifndef GOLDOBJ
    long ugold = u.ugold + hidden_gold();

    if ((ugold -= u.ugold0) < 0L) ugold = 0L;
    return ugold + u.urexp + (long)(50 * (deepest - 1))
#else
    long umoney = money_cnt(invent) + hidden_gold();

    if ((umoney -= u.umoney0) < 0L) umoney = 0L;
    return umoney + u.urexp + (long)(50 * (deepest - 1))
#endif
			  + (long)(deepest > 30 ? 10000 :
				   deepest > 20 ? 1000*(deepest - 20) : 0);
}
#endif

#ifdef DUMP_LOG
void bot1str(char *newbot1)
#else
STATIC_OVL void
bot1()
#endif
{
#ifndef DUMP_LOG
	char newbot1[MAXCO];
#endif
	register char *nb;
	register int i,j;

	Strcpy(newbot1, plname);
	if('a' <= newbot1[0] && newbot1[0] <= 'z') newbot1[0] += 'A'-'a';
	newbot1[10] = 0;
	Sprintf(nb = eos(newbot1)," the ");

	if (Upolyd) {
		char mbot[BUFSZ];
		int k = 0;

		Strcpy(mbot, mons[u.umonnum].mname);
		while(mbot[k] != 0) {
		    if ((k == 0 || (k > 0 && mbot[k-1] == ' ')) &&
					'a' <= mbot[k] && mbot[k] <= 'z')
			mbot[k] += 'A' - 'a';
		    k++;
		}
		Sprintf(nb = eos(nb), "%s", mbot);
	} else
		Sprintf(nb = eos(nb), "%s", rank());

	Sprintf(nb = eos(nb),"  ");
	i = mrank_sz + 15;
	j = (nb + 2) - newbot1; /* aka strlen(newbot1) but less computation */
	if((i - j) > 0)
		Sprintf(nb = eos(nb),"%*s", i-j, " ");	/* pad with spaces */
	if (ACURR(A_STR) > 18) {
		if (ACURR(A_STR) > STR18(100))
		    Sprintf(nb = eos(nb),"St:%2d ",ACURR(A_STR)-100);
		else if (ACURR(A_STR) < STR18(100))
		    Sprintf(nb = eos(nb), "St:18/%02d ",ACURR(A_STR)-18);
		else
		    Sprintf(nb = eos(nb),"St:18/** ");
	} else
		Sprintf(nb = eos(nb), "St:%-1d ",ACURR(A_STR));
	Sprintf(nb = eos(nb),
		"Dx:%-1d Co:%-1d In:%-1d Wi:%-1d Ch:%-1d",
		ACURR(A_DEX), ACURR(A_CON), ACURR(A_INT), ACURR(A_WIS), ACURR(A_CHA));
	Sprintf(nb = eos(nb), (u.ualign.type == A_CHAOTIC) ? "  Chaotic" :
			(u.ualign.type == A_NEUTRAL) ? "  Neutral" : "  Lawful");
#ifdef SCORE_ON_BOTL
	if (flags.showscore)
	    Sprintf(nb = eos(nb), " S:%ld", botl_score());
#endif
#ifdef DUMP_LOG
}
STATIC_OVL void
bot1()
{
	char newbot1[MAXCO];

	bot1str(newbot1);
#endif
	curs(WIN_STATUS, 1, 0);
	putstr(WIN_STATUS, 0, newbot1);
}

/* provide the name of the current level for display by various ports */
const char* short_dgn_names[] = {
	"Dungeons of Doom",
	"Gehennom",
	"Gnomish Mines",
	"Quest Levels",	 /* Placeholder; this is overridden below */
	"Sokoban",
	"Fort Ludios",		 /* this too */
	"Vlad's Tower",
	"End Game"			 /* and this */
};

int
describe_level(buf)
char *buf;
{
	int ret = 1;

	/* TODO:	Add in dungeon name */
	if (Is_knox(&u.uz))
		Sprintf(buf, "%s ", dungeons[u.uz.dnum].dname);
	else if (In_quest(&u.uz))
		Sprintf(buf, "Home %d ", dunlev(&u.uz));
	else if (In_endgame(&u.uz))
		Sprintf(buf,
			Is_astralevel(&u.uz) ? "Astral Plane " : "End Game ");
	else {
		/* ports with more room may expand this one */
		Sprintf(buf, "%s:%-2d ", 
				iflags.show_dgn_name ? short_dgn_names[u.uz.dnum] : "Dlvl",
				depth(&u.uz));
		ret = 0;
	}
	return ret;
}

#ifdef DUMP_LOG
void bot2str(newbot2)
char* newbot2;
#else
STATIC_OVL void
bot2()
#endif
{
#ifndef DUMP_LOG
	char  newbot2[MAXCO];
#endif
	register char *nb;
	int hp, hpmax;
	int cap = near_capacity();
#if defined(STATUS_COLORS) && defined(TEXTCOLOR)
	struct color_option color_option;
	int save_botlx = flags.botlx;
#endif

	int x = 1;

	hp = Upolyd ? u.mh : u.uhp;
	hpmax = Upolyd ? u.mhmax : u.uhpmax;

	if(hp < 0) hp = 0;
	(void) describe_level(newbot2);
	Sprintf(nb = eos(newbot2), "%c:%-2ld", oc_syms[COIN_CLASS],
#ifndef GOLDOBJ
		u.ugold
#else
		money_cnt(invent)
#endif
	       );

#if defined(STATUS_COLORS) && defined(TEXTCOLOR)
	Strcat(nb = eos(newbot2), " HP:");
	curs(WIN_STATUS, x, 1);
	putstr(WIN_STATUS, 0, newbot2);
	x += strlen(newbot2);

	Sprintf(nb = eos(nb), "%d(%d)", hp, hpmax);

	apply_color_option(percentage_color_of(hp, hpmax, hp_colors), nb, x);
	x += strlen(nb);
#else
	Sprintf(nb = eos(nb), " HP:%d(%d)", hp, hpmax);
#endif
#if defined(STATUS_COLORS) && defined(TEXTCOLOR)
	Strcat(nb = eos(nb), " Pw:");
	curs(WIN_STATUS, x, 1);
	putstr(WIN_STATUS, 0, nb);

	x += strlen(nb);

	Sprintf(nb = eos(nb), "%d(%d)", u.uen, u.uenmax);
	apply_color_option(percentage_color_of(u.uen, u.uenmax, pw_colors), nb, x);
	x += strlen(nb);
#else
	Sprintf(nb = eos(nb), " Pw:%d(%d)", u.uen, u.uenmax);
#endif
	Sprintf(nb = eos(nb), " AC:%-2d", u.uac);
	curs(WIN_STATUS, x, 1);
	putstr(WIN_STATUS, 0, nb);
	x += strlen(nb);
	if (Upolyd) {
		Sprintf(nb = eos(nb), " HD:%d", mons[u.umonnum].mlevel);
		curs(WIN_STATUS, x, 1);
		putstr(WIN_STATUS, 0, nb);
		x += strlen(nb);
	}
#ifdef EXP_ON_BOTL
	else if(flags.showexp) {
		Sprintf(nb = eos(nb), " Xp:%u/%-1ld", u.ulevel,u.uexp);
		curs(WIN_STATUS, x, 1);
		putstr(WIN_STATUS, 0, nb);
		x += strlen(nb);
	}
#endif
	else {
		Sprintf(nb = eos(nb), " Exp:%u", u.ulevel);
		curs(WIN_STATUS, x, 1);
		putstr(WIN_STATUS, 0, nb);
		x += strlen(nb);
	}
	if(flags.time) {
	    Sprintf(nb = eos(nb), " T:%ld", moves);
	    curs(WIN_STATUS, x, 1);
	    putstr(WIN_STATUS, 0, nb);
	    x += strlen(nb);
	}
#ifdef REALTIME_ON_BOTL
  if(iflags.showrealtime) {
    time_t currenttime = get_realtime();
    Sprintf(nb = eos(nb), " %d:%2.2d", currenttime / 3600, (currenttime % 3600) / 60);
  }
#endif
	x++;
 	if(strcmp(hu_stat[u.uhs], "        ")) {
#if defined(STATUS_COLORS) && defined(TEXTCOLOR)
	    add_colored_text(hu_stat[u.uhs], x);
#endif
	    Sprintf(nb = eos(nb), " %s", hu_stat[u.uhs]);
	    x += strlen(nb);
	}
 	if(Confusion) {
#if defined(STATUS_COLORS) && defined(TEXTCOLOR)
	    add_colored_text("Conf", x);
#endif
	    Strcat(nb = eos(nb), " Conf");
	    x += strlen(nb);
	}
	if(Sick) {
	    if (u.usick_type & SICK_VOMITABLE) {
#if defined(STATUS_COLORS) && defined(TEXTCOLOR)
		add_colored_text("FoodPois", x);
#endif
		Strcat(nb = eos(nb), " FoodPois");
		x += strlen(nb);
	    }
	    if (u.usick_type & SICK_NONVOMITABLE) {
#if defined(STATUS_COLORS) && defined(TEXTCOLOR)
		add_colored_text("Ill", x);
#endif
		Strcat(nb = eos(nb), " Ill");
		x += strlen(nb);
	    }
	}
	if(Slow) {
#if defined(STATUS_COLORS) && defined(TEXTCOLOR)
	    add_colored_text("Slow", x);
#endif
	    Strcat(nb = eos(nb)," Slow");
	    x += strlen(nb);
	}
	if(Blind) {
#if defined(STATUS_COLORS) && defined(TEXTCOLOR)
	    add_colored_text("Blind", x);
#endif
	    Strcat(nb = eos(nb), " Blind");
	    x += strlen(nb);
	}
        if(Levitation) {
#if defined(STATUS_COLORS) && defined(TEXTCOLOR)
            add_colored_text("Lev", x);
#endif
            Strcat(nb = eos(nb), " Lev");
            x += strlen(nb);
        }
        if(Flying) {
#if defined(STATUS_COLORS) && defined(TEXTCOLOR)
            add_colored_text("Fly", x);
#endif
            Strcat(nb = eos(nb), " Fly");
            x += strlen(nb);
        }
        if(u.usteed) {
#if defined(STATUS_COLORS) && defined(TEXTCOLOR)
            add_colored_text("Ride", x);
#endif
            Strcat(nb = eos(nb), " Ride");
            x += strlen(nb);
        }
	if(Stunned) {
#if defined(STATUS_COLORS) && defined(TEXTCOLOR)
	    add_colored_text("Stun", x);
#endif
	    Strcat(nb = eos(nb), " Stun");
	    x += strlen(nb);
	}
	if(Hallucination) {
#if defined(STATUS_COLORS) && defined(TEXTCOLOR)
	    add_colored_text("Hallu", x);
#endif
	    Strcat(nb = eos(nb), " Hallu");
            x += strlen(nb);
        }
	if(Slimed) {
#if defined(STATUS_COLORS) && defined(TEXTCOLOR)
	    add_colored_text("Slime", x);
#endif
	    Strcat(nb = eos(nb), " Slime");
	    x += strlen(nb);
	}
        if(Stoned) {
#if defined(STATUS_COLORS) && defined(TEXTCOLOR)
            add_colored_text("Stone", x);
#endif
            Strcat(nb = eos(nb), " Stone");
            x += strlen(nb);
        }
        if(Strangled) {
#if defined(STATUS_COLORS) && defined(TEXTCOLOR)
            add_colored_text("Strngl", x);
#endif
            Strcat(nb = eos(nb), " Strngl");
            x += strlen(nb);
        }
	if(cap > UNENCUMBERED) {
#if defined(STATUS_COLORS) && defined(TEXTCOLOR)
	    add_colored_text(enc_stat[cap], x);
	    flags.botlx = save_botlx;
#endif
	    Sprintf(nb = eos(nb), " %s", enc_stat[cap]);
	    x += strlen(nb);
	}
#ifdef DUMP_LOG
}
STATIC_OVL void
bot2()
{
	char newbot2[MAXCO];
	bot2str(newbot2);
#endif
#if defined(STATUS_COLORS) && defined(TEXTCOLOR)
#else
	curs(WIN_STATUS, 1, 1);
	putstr(WIN_STATUS, 0, newbot2);
#endif
}

void
bot()
{
	bot1();
	bot2();
	flags.botl = flags.botlx = 0;
}

#endif /* OVL0 */

/*botl.c*/
