/*
 from http://home.flash.net/~dtribble/text/fpattern.htm
 2000/12/12
 by Perry
 License statement is in comments below
*/

/******************************************************************************
* fpattern.c
*	Functions for matching filename patterns to filenames.
*
* Usage
*	(See "fpattern.h".)
*
* Notes
*	These pattern matching capabilities are modeled after those found in
*	the UNIX command shells.
*
*	`DELIM' must be defined to 1 if pathname separators are to be handled
*	explicitly.
*
* History
*	1.00 1997-01-03 David Tribble.
*		First cut.
*	1.01 1997-01-03 David Tribble.
*		Added SUB pattern character.
*		Added fpattern_matchn().
*	1.02 1997-01-12 David Tribble.
*		Fixed missing lowercase matching cases.
*	1.03 1997-01-13 David Tribble.
*		Pathname separator code is now controlled by DELIM macro.
*	1.04 1997-01-14 David Tribble.
*		Added QUOTE macro.
*	1.05 1997-01-15 David Tribble.
*		Handles special case of empty pattern and empty filename.
*	1.06 1997-01-26 David Tribble.
*		Changed range negation character from '^' to '!', ala Unix.
*	1.07 1997-08-02 David Tribble.
*		Uses the 'FPAT_XXX' constants.
*	1.08 1998-06-28 David Tribble.
*		Minor fixed for MS-VC++ (5.0).
*
* Limitations
*	This code is copyrighted by the author, but permission is hereby
*	granted for its unlimited use provided that the original copyright
*	and authorship notices are retained intact.
*
*	Other queries can be sent to:
*	    dtribble@technologist.com
*	    david.tribble@beasys.com
*	    dtribble@flash.net
*
* Copyright 1997-1998 by David R. Tribble, all rights reserved.
* $Id: fpattern.c 3612 2006-07-22 09:59:46Z thiadmer $
*/


/* Identification */
#if 0
static const char	id[] =
    "@(#)lib/fpattern.c 1.08";

static const char	copyright[] =
    "Copyright 1997-1998 David R. Tribble\n";
#endif


/* System includes */

#include <ctype.h>
#include <stddef.h>

#if TEST
#include <locale.h>
#include <stdio.h>
#include <string.h>
#if defined _WIN32 || defined __WIN32__ || defined WIN32
#include <stdlib.h>
#define sleep(q) Sleep(q*1000)
#include <windows.h>
#endif
#endif

#if defined(unix) || defined(_unix) || defined(__unix) || \
    defined(__unix__) || ( defined(__MACH__) && defined(__APPLE_CC__) )
#define UNIX	1
#define DOS	0
#elif defined(__MSDOS__) || defined(_WIN32) || defined(__WIN32__) || defined(WIN32)
#define UNIX	0
#define DOS	1
#else
#error Cannot ascertain O/S from predefined macros
#endif


/* Local includes */

#include "fpattern.h"


/* Local constants */

#ifndef NULL
# define NULL		((void *) 0)
#endif

#ifndef FALSE
# define FALSE		0
#endif

#ifndef TRUE
# define TRUE		1
#endif

#if TEST
# define SUB		'~'
#else
# define SUB		FPAT_CLOSP
#endif

#ifndef DELIM
# define DELIM		0
#endif

#ifndef SUBCLOS
# define SUBCLOS	0
#endif

#define DEL		FPAT_DEL

#if UNIX
#define DEL2		FPAT_DEL
#else /*DOS*/
#define DEL2		FPAT_DEL2
#endif

#if UNIX
#define QUOTE		FPAT_QUOTE
#else /*DOS*/
#define QUOTE		FPAT_QUOTE2
#endif


/* Local function macros */

/* Warning,  while this code was originally filename search code
 * which was case insensitive on DOS and case sensitive on UNIX
 * it's being used for name searching and should behave the same everywhere
 */
static int lowercase(int c, int keepcase)
{
    if (keepcase)
        return c;
    return tolower(c);
}

/*-----------------------------------------------------------------------------
* fpattern_isvalid()
*	Checks that filename pattern 'pat' is a well-formed pattern.
*
* Returns
*	1 (true) if 'pat' is a valid filename pattern, otherwise 0 (false).
*
* Caveats
*	If 'pat' is null, 0 (false) is returned.
*
*	If 'pat' is empty (""), 1 (true) is returned, and it is considered a
*	valid (but degenerate) pattern (the only filename it matches is the
*	empty ("") string).
*/

int fpattern_isvalid(const char *pat)
{
    int		len;
    char	close;

    /* Check args */
    if (pat == NULL)
        return (FALSE);

    /* Verify that the pattern is valid */
    for (len = 0;  pat[len] != '\0';  len++)
    {
        switch (pat[len])
        {
        case FPAT_SET_L:
        case FPAT_MSET_L:
            /* Char set */
            close = (char)((pat[len] == FPAT_SET_L) ? FPAT_SET_R : FPAT_MSET_R);
            len++;
            if (pat[len] == FPAT_SET_NOT)
                len++;			/* Set negation */

            while (pat[len] != close)
            {
                if (pat[len] == QUOTE)
                    len++;		/* Quoted char */
                if (pat[len] == '\0')
                    return (FALSE);	/* Missing closing bracket */
                len++;

                if (pat[len] == FPAT_SET_THRU)
                {
                    /* Char range */
                    len++;
                    if (pat[len] == QUOTE)
                        len++;		/* Quoted char */
                    if (pat[len] == '\0')
                        return (FALSE);	/* Missing closing bracket */
                    len++;
                }

                if (pat[len] == '\0')
                    return (FALSE);	/* Missing closing bracket */
            }
            break;

        case QUOTE:
            /* Quoted char */
            len++;
            if (pat[len] == '\0')
                return (FALSE);		/* Missing quoted char */
            break;

        case FPAT_NOT:
            /* Negated pattern */
            len++;
            if (pat[len] == '\0')
                return (FALSE);		/* Missing subpattern */
            break;

        default:
            /* Valid character */
            break;
        }
    }

    return (TRUE);
}


/*-----------------------------------------------------------------------------
* fpattern_submatch()
*	Attempts to match subpattern 'pat' to subfilename 'fname'.
*
* Returns
*	1 (true) if the subfilename matches, otherwise 0 (false).
*
* Caveats
*	This does not assume that 'pat' is well-formed.
*
*	If 'pat' is empty (""), the only filename it matches is the empty ("")
*	string.
*
*	Some non-empty patterns (e.g., "") will match an empty filename ("").
*/

static int fpattern_submatch(const char *pat, const char *fname, int flength, int keepcase)
{
    int		fch;
    int		pch;
    int		i;
    int		yes, match;
    int		lo, hi;

    /* Attempt to match subpattern against subfilename */
    while (*pat != '\0')
    {
        fch = *fname;
        pch = *pat;
        pat++;

        switch (pch)
        {
        case FPAT_ANY:
            /* Match a single char */
        #if DELIM
            if (fch == DEL  ||  fch == DEL2  ||  fch == '\0')
                return (FALSE);
        #else
            if (flength == 0)
                return (FALSE);
        #endif
            fname++;
            flength--;
            break;

        case FPAT_CLOS:
            /* Match zero or more chars */
        #if DELIM
            i = 0;
            while (fname[i] != '\0'  &&
                    fname[i] != DEL  &&  fname[i] != DEL2)
                i++;
        #else
            i = flength;
        #endif
            while (i >= 0)
            {
                if (fpattern_submatch(pat, fname+i, flength-i, keepcase))
                    return (TRUE);
                i--;
            }
            return (FALSE);

	#if SUBCLOS
        case SUB:
            /* Match zero or more chars */
            i = 0;
            while (i < flength  &&
        #if DELIM
                    fname[i] != DEL  &&  fname[i] != DEL2  &&
        #endif
                    fname[i] != '.')
                i++;
            while (i >= 0)
            {
                if (fpattern_submatch(pat, fname+i, flength-i, keepcase))
                    return (TRUE);
                i--;
            }
            return (FALSE);
	#endif

        case QUOTE:
            /* Match a quoted char */
            pch = *pat;
            if (lowercase(fch, keepcase) != lowercase(pch, keepcase)  ||  pch == '\0')
                return (FALSE);
            fname++;
            flength--;
            pat++;
            break;

        case FPAT_SET_L:
            /* Match char set/range */
            yes = TRUE;
            if (*pat == FPAT_SET_NOT)
            {
               pat++;
               yes = FALSE;	/* Set negation */
            }

            /* Look for [s], [-], [abc], [a-c] */
            match = !yes;
            while (*pat != FPAT_SET_R  &&  *pat != '\0')
            {
                if (*pat == QUOTE)
                    pat++;	/* Quoted char */

                if (*pat == '\0')
                    break;
                lo = *pat++;
                hi = lo;

                if (*pat == FPAT_SET_THRU)
                {
                    /* Range */
                    pat++;

                    if (*pat == QUOTE)
                        pat++;	/* Quoted char */

                    if (*pat == '\0')
                        break;
                    hi = *pat++;
                }

                if (*pat == '\0')
                    break;

                /* Compare character to set range */
                if (lowercase(fch, keepcase) >= lowercase(lo, keepcase)  &&
                    lowercase(fch, keepcase) <= lowercase(hi, keepcase))
                    match = yes;
            }

            if (!match)
                return (FALSE);

            if (*pat == '\0')
                return (FALSE);		/* Missing closing bracket */

            fname++;
            flength--;
            pat++;
            break;

        case FPAT_MSET_L:
            /* Match zero or more characters in a char set/range */
            yes = TRUE;
            if (*pat == FPAT_SET_NOT)
            {
               pat++;
               yes = FALSE;	/* Set negation */
            }

            do {
                const char *org_pat = pat;
                /* Look for [s], [-], [abc], [a-c] */
                match = !yes;
                while (*pat != FPAT_MSET_R  &&  *pat != '\0')
                {
                    if (*pat == QUOTE)
                        pat++;	/* Quoted char */

                    if (*pat == '\0')
                        break;
                    lo = *pat++;
                    hi = lo;

                    if (*pat == FPAT_SET_THRU)
                    {
                        /* Range */
                        pat++;

                        if (*pat == QUOTE)
                            pat++;	/* Quoted char */

                        if (*pat == '\0')
                            break;
                        hi = *pat++;
                    }

                    if (*pat == '\0')
                        break;

                    /* Compare character to set range */
                    if (lowercase(fch, keepcase) >= lowercase(lo, keepcase)  &&
                        lowercase(fch, keepcase) <= lowercase(hi, keepcase)) {
                        match = yes;
                        while (*pat != FPAT_MSET_R && *pat != '\0')
                            pat++;
                        break;
                    } /* if */
                }

                if (*pat == '\0')
                    return (FALSE);	/* Missing closing bracket */

                if (match) {
                    fname++;
                    flength--;
                    fch = *fname;
                    if (flength > 0)
                        pat = org_pat;
                }

            } while (match && flength > 0);

            pat++;
            break;

        case FPAT_NOT:
            /* Match only if rest of pattern does not match */
            if (*pat == '\0')
                return (FALSE);		/* Missing subpattern */
            i = fpattern_submatch(pat, fname, flength, keepcase);
            return !i;

#if DELIM
        case DEL:
    #if DEL2 != DEL
        case DEL2:
    #endif
            /* Match path delimiter char */
            if (fch != DEL  &&  fch != DEL2)
                return (FALSE);
            fname++;
            flength--;
            break;
#endif

        default:
            /* Match a (non-null) char exactly */
            if (lowercase(fch, keepcase) != lowercase(pch, keepcase))
                return (FALSE);
            fname++;
            flength--;
            break;
        }
    }

    /* Check for complete match */
    if (flength != 0)
        return (FALSE);

    /* Successful match */
    return (TRUE);
}


/*-----------------------------------------------------------------------------
* fpattern_match()
*	Attempts to match pattern 'pat' to filename 'fname'. The comparison is case
* sensitive if 'keepcase' is true, and case insensitive otherwise. The 'flength'
* parameter allows to check partial strings, or to check strings with embedded
* zero bytes. When 'flength' is -1, it is set to the string length.
*
* Returns
*	1 (true) if the filename matches, otherwise 0 (false).
*
* Caveats
*	If 'fname' is null, zero (false) is returned.
*
*	If 'pat' is null, zero (false) is returned.
*
*	If 'pat' is empty (""), the only filename it matches is the empty
*	string ("").
*
*	If 'fname' is empty, the only pattern that will match it is the empty
*	string ("").
*
*	If 'pat' is not a well-formed pattern, zero (false) is returned.
*
*	Upper and lower case letters are treated the same; alphabetic
*	characters are converted to lower case before matching occurs.
*	Conversion to lower case is dependent upon the current locale setting.
*/

int fpattern_match(const char *pat, const char *fname, int flength, int keepcase)
{
    int		rc;

    /* Check args */
    if (fname == NULL)
        return (FALSE);

    if (pat == NULL)
        return (FALSE);

    /* Verify that the pattern is valid, and get its length */
    if (!fpattern_isvalid(pat))
        return (FALSE);

    /* Attempt to match pattern against filename */
    if (flength < 0)
        flength = strlen(fname);
    if (flength == 0)
        return (pat[0] == '\0');	/* Special case */
    rc = fpattern_submatch(pat, fname, flength, keepcase);

    return (rc);
}


/*-----------------------------------------------------------------------------
* fpattern_matchn()
*	Attempts to match pattern 'pat' to filename 'fname'.
*	This operates like fpattern_match() except that it does not verify that
*	pattern 'pat' is well-formed, assuming that it has been checked by a
*	prior call to fpattern_isvalid().
*
* Returns
*	1 (true) if the filename matches, otherwise 0 (false).
*
* Caveats
*	If 'fname' is null, zero (false) is returned.
*
*	If 'pat' is null, zero (false) is returned.
*
*	If 'pat' is empty (""), the only filename it matches is the empty ("")
*	string.
*
*	If 'pat' is not a well-formed pattern, unpredictable results may occur.
*
*	Upper and lower case letters are treated the same; alphabetic
*	characters are converted to lower case before matching occurs.
*	Conversion to lower case is dependent upon the current locale setting.
*
* See also
*	fpattern_match().
*/

int fpattern_matchn(const char *pat, const char *fname, int flength, int keepcase)
{
    int		rc;

    /* Check args */
    if (fname == NULL)
        return (FALSE);

    if (pat == NULL)
        return (FALSE);

    /* Assume that pattern is well-formed */

    /* Attempt to match pattern against filename */
    if (flength < 0)
        flength = strlen(fname);
    rc = fpattern_submatch(pat, fname, flength, keepcase);

    return (rc);
}

/* returns the largest packet that matches the pattern */
int fpattern_matchcount(const char *pat, const char *fname, int flength, int keepcase)
{
    int len;

    if (fname == NULL)
        return (FALSE);

    if (pat == NULL)
        return (FALSE);

    /* Assume that pattern is well-formed */

    /* Attempt to match pattern against filename */
    if (flength < 0)
        flength = strlen(fname);

    for (len = flength; len >= 0; len--)
        if (fpattern_submatch(pat, fname, flength, keepcase))
            break;

    return len;
}


/*---------------------------------------------------------------------------*/
/*---------------------------------------------------------------------------*/
/*---------------------------------------------------------------------------*/

#if TEST


/* Local variables */

static int	count =	0;
static int	fails =	0;
static int	stop_on_fail = FALSE;


/*-----------------------------------------------------------------------------
* test()
*/

static void test(int expect, const char *fname, const char *pat)
{
    int		failed;
    int		result;
    char	fbuf[80+1];
    char	pbuf[80+1];

    count++;
    printf("%3d. ", count);

    if (fname == NULL)
    {
        printf("<null>\n");
    }
    else
    {
        strcpy(fbuf, fname);
        printf("\"%s\"\n", fbuf);
    }

    if (pat == NULL)
    {
        printf("     <null>\n");
    }
    else
    {
        strcpy(pbuf, pat);
        printf("     \"%s\"\n", pbuf);
    }

    result = fpattern_match(pat == NULL ? NULL : pbuf,
                            fname == NULL ? NULL : fbuf,
                            -1, FALSE);

    failed = (result != expect);
    printf("    -> %c, expected %c: %s\n",
        "FT"[!!result], "FT"[!!expect], failed ? "FAIL ***" : "pass");

    if (failed)
    {
        fails++;

        if (stop_on_fail)
            exit(1);
        sleep(1);
    }

    printf("\n");
}


/*-----------------------------------------------------------------------------
* main()
*	Test driver.
*/

int main(int argc, char **argv)
{
    (void) argc;	/* Shut up lint */
    (void) argv;	/* Shut up lint */

#if DEBUG
    dbg_f = stdout;
#endif

    printf("==========================================\n");

    setlocale(LC_CTYPE, "");

#if UNIX
    printf("[O/S is UNIX]\n");
#elif DOS
    printf("[O/S is DOS]\n");
#else
    printf("[O/S is unknown]\n");
#endif

#if 1	/* Set to nonzero to stop on first failure */
    stop_on_fail = TRUE;
#endif

    test(0,	NULL,	NULL);
    test(0,	NULL,	"");
    test(0,	NULL,	"abc");
    test(0,	"",	NULL);
    test(0,	"abc",	NULL);

    test(1,	"abc",		"abc");
    test(0,	"ab",		"abc");
    test(0,	"abcd",		"abc");
    test(0,	"Foo.txt",	"Foo.x");
    test(1,	"Foo.txt",	"Foo.txt");
    test(1,	"Foo.txt",	"foo.txt");
    test(1,	"FOO.txt",	"foo.TXT");

    test(1,	"a",		"?");
    test(1,	"foo.txt",	"f??.txt");
    test(1,	"foo.txt",	"???????");
    test(0,	"foo.txt",	"??????");
    test(0,	"foo.txt",	"????????");

    test(1,	"a",		"`a");
    test(1,	"AB",		"a`b");
    test(0,	"aa",		"a`b");
    test(1,	"a`x",		"a``x");
    test(1,	"a`x",		"`a```x");
    test(1,	"a*x",		"a`*x");

#if DELIM
    test(0,	"",		"/");
    test(0,	"",		"\\");
    test(1,	"/",		"/");
    test(1,	"/",		"\\");
    test(1,	"\\",		"/");
    test(1,	"\\",		"\\");

    test(1,	"a/b",		"a/b");
    test(1,	"a/b",		"a\\b");

    test(1,	"/",		"*/*");
    test(1,	"foo/a.c",	"f*/*.?");
    test(1,	"foo/a.c",	"*/*");
    test(0,	"foo/a.c",	"/*/*");
    test(0,	"foo/a.c",	"*/*/");

    test(1,	"/",		"~/~");
    test(1,	"foo/a.c",	"f~/~.?");
    test(0,	"foo/a.c",	"~/~");
    test(1,	"foo/abc",	"~/~");
    test(0,	"foo/a.c",	"/~/~");
    test(0,	"foo/a.c",	"~/~/");
#endif

    test(0,	"",		"*");
    test(1,	"a",		"*");
    test(1,	"ab",		"*");
    test(1,	"abc",		"**");
    test(1,	"ab.c",		"*.?");
    test(1,	"ab.c",		"*.*");
    test(1,	"ab.c",		"*?");
    test(1,	"ab.c",		"?*");
    test(1,	"ab.c",		"?*?");
    test(1,	"ab.c",		"?*?*");
    test(1,	"ac",		"a*c");
    test(1,	"axc",		"a*c");
    test(1,	"ax-yyy.c",	"a*c");
    test(1,	"ax-yyy.c",	"a*x-yyy.c");
    test(1,	"axx/yyy.c",	"a*x/*c");

#if SUBCLOS
    test(0,	"",		"~");
    test(1,	"a",		"~");
    test(1,	"ab",		"~");
    test(1,	"abc",		"~~");
    test(1,	"ab.c",		"~.?");
    test(1,	"ab.c",		"~.~");
    test(0,	"ab.c",		"~?");
    test(0,	"ab.c",		"?~");
    test(0,	"ab.c",		"?~?");
    test(1,	"ab.c",		"?~.?");
    test(1,	"ab.c",		"?~?~");
    test(1,	"ac",		"a~c");
    test(1,	"axc",		"a~c");
    test(0,	"ax-yyy.c",	"a~c");
    test(1,	"ax-yyyvc",	"a~c");
    test(1,	"ax-yyy.c",	"a~x-yyy.c");
    test(0,	"axx/yyy.c",	"a~x/~c");
    test(1,	"axx/yyyvc",	"a~x/~c");
#endif

    test(0,	"a",		"!");
    test(0,	"a",		"!a");
    test(1,	"a",		"!b");
    test(1,	"abc",		"!abb");
    test(0,	"a",		"!*");
    test(1,	"abc",		"!*.?");
    test(1,	"abc",		"!*.*");
    test(0,	"",		"!*");		/*!*/
    test(0,	"",		"!*?");		/*!*/
    test(0,	"a",		"!*?");
    test(0,	"a",		"a!*");
    test(1,	"a",		"a!?");
    test(1,	"a",		"a!*?");
    test(1,	"ab",		"*!?");
    test(1,	"abc",		"*!?");
    test(0,	"ab",		"?!?");
    test(1,	"abc",		"?!?");
    test(0,	"a-b",		"!a[-]b");
    test(0,	"a-b",		"!a[x-]b");
    test(0,	"a=b",		"!a[x-]b");
    test(0,	"a-b",		"!a[x`-]b");
    test(1,	"a=b",		"!a[x`-]b");
    test(0,	"a-b",		"!a[x---]b");
    test(1,	"a=b",		"!a[x---]b");

    test(1,	"abc",		"a[b]c");
    test(1,	"aBc",		"a[b]c");
    test(1,	"abc",		"a[bB]c");
    test(1,	"abc",		"a[bcz]c");
    test(1,	"azc",		"a[bcz]c");
    test(0,	"ab",		"a[b]c");
    test(0,	"ac",		"a[b]c");
    test(0,	"axc",		"a[b]c");

    test(0,	"abc",		"a[!b]c");
    test(0,	"abc",		"a[!bcz]c");
    test(0,	"azc",		"a[!bcz]c");
    test(0,	"ab",		"a[!b]c");
    test(0,	"ac",		"a[!b]c");
    test(1,	"axc",		"a[!b]c");
    test(1,	"axc",		"a[!bcz]c");

    test(1,	"a1z",		"a[0-9]z");
    test(0,	"a1",		"a[0-9]z");
    test(0,	"az",		"a[0-9]z");
    test(0,	"axz",		"a[0-9]z");
    test(1,	"a2z",		"a[-0-9]z");
    test(1,	"a-z",		"a[-0-9]z");
    test(1,	"a-b",		"a[-]b");
    test(0,	"a-b",		"a[x-]b");
    test(0,	"a=b",		"a[x-]b");
    test(1,	"a-b",		"a[x`-]b");
    test(0,	"a=b",		"a[x`-]b");
    test(1,	"a-b",		"a[x---]b");
    test(0,	"a=b",		"a[x---]b");

    test(0,	"a0z",		"a[!0-9]z");
    test(1,	"aoz",		"a[!0-9]z");
    test(0,	"a1",		"a[!0-9]z");
    test(0,	"az",		"a[!0-9]z");
    test(0,	"a9Z",		"a[!0-9]z");
    test(1,	"acz",		"a[!-0-9]z");
    test(0,	"a7z",		"a[!-0-9]z");
    test(0,	"a-z",		"a[!-0-9]z");
    test(0,	"a-b",		"a[!-]b");
    test(0,	"a-b",		"a[!x-]b");
    test(0,	"a=b",		"a[!x-]b");
    test(0,	"a-b",		"a[!x`-]b");
    test(1,	"a=b",		"a[!x`-]b");
    test(0,	"a-b",		"a[!x---]b");
    test(1,	"a=b",		"a[!x---]b");

    test(1,	"a!z",		"a[`!0-9]z");
    test(1,	"a3Z",		"a[`!0-9]z");
    test(0,	"A3Z",		"a[`!0`-9]z");
    test(1,	"a9z",		"a[`!0`-9]z");
    test(1,	"a-z",		"a[`!0`-9]z");

    test(1,	"ac",     "a{b}c");
    test(1,	"abc",		"a{b}c");
    test(1,	"abbc",		"a{b}c");
    test(1,	"aBbBc",	"a{b}c");
    test(1,	"abc",		"a{bB}c");
    test(1,	"abc",		"a{bpz}c");
    test(1,	"azc",		"a{bcz}");
    test(0,	"ab",     "a{b}c");
    test(0,	"axc",		"a{b}c");

    printf("%d tests, %d failures\n", count, fails);
    return (fails == 0 ? 0 : 1);
}


#endif /* TEST */

/* End fpattern.c */


