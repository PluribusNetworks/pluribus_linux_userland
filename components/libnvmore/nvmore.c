/*
 * COPYRIGHT 2012 Pluribus Networks Inc.
 *
 * All rights reserved. This copyright notice is Copyright Management
 * Information under 17 USC 1202 and is included to protect this work and
 * deter copyright infringement.  Removal or alteration of this Copyright
 * Management Information without the express written permission from
 * Pluribus Networks Inc is prohibited, and any such unauthorized removal
 * or alteration will be a violation of federal law.
 */

/*
 * CDDL HEADER START
 *
 * The contents of this file are subject to the terms of the
 * Common Development and Distribution License (the "License").
 * You may not use this file except in compliance with the License.
 *
 * You can obtain a copy of the license at usr/src/OPENSOLARIS.LICENSE
 * or http://www.opensolaris.org/os/licensing.
 * See the License for the specific language governing permissions
 * and limitations under the License.
 *
 * When distributing Covered Code, include this CDDL HEADER in each
 * file and include the License file at usr/src/OPENSOLARIS.LICENSE.
 * If applicable, add the following below this CDDL HEADER, with the
 * fields enclosed by brackets "[]" replaced with your own identifying
 * information: Portions Copyright [yyyy] [name of copyright owner]
 *
 * CDDL HEADER END
 */
/*
 * Copyright 2006 Sun Microsystems, Inc.  All rights reserved.
 * Use is subject to license terms.
 */

/*	Copyright (c) 1984, 1986, 1987, 1988, 1989 AT&T	*/
/*	  All Rights Reserved  	*/

/*	Copyright (c) 1987, 1988 Microsoft Corporation	*/
/*	  All Rights Reserved	*/

/*
 * University Copyright- Copyright (c) 1982, 1986, 1988
 * The Regents of the University of California
 * All Rights Reserved
 *
 * University Acknowledgment- Portions of this document are derived from
 * software developed by the University of California, Berkeley, and its
 * contributors.
 */

/*
 *      @(#) more.c 1.1 88/03/29 more:more.c
 */

/*
** more.c - General purpose tty output filter and file perusal program
**
**      by Eric Shienbrood, UC Berkeley
**
**      modified by Geoff Peck, UCB to add underlining, single spacing
**      modified by John Foderaro, UCB to add -c and MORE environment variable
**      modified by Hans Spiller, Microsoft to handle \r better July 23, 82
**              added ? help command, and -w
**
**      vwh     11 Jan 83       M001
**              modified to handle x.out magic number and magic number
**              byte ordering OTHER than the vax and pdp11.
**      JJD     19 Jan 83       M002
**              - fix distributed on USENET
**                From decvax!ucbvax!dist2 Sun Dec  6 02:58:31 1981
**                Subject: FIXED:  bug in src/more/more.c
**              - fixed bug on terminal with "magic cookie" standout
**                sequences.
**      JJD     14 Feb 83       M003
**              - fix exit status of more
**              - Made first letter of "no more" message uppercase
**      andyp   03 Aug 83       M004    3.0 upgrade
**      - moved <local/uparm.h> to cmd/include.
**      - use UCB, rather than XENIX, stty(2).
**      andyp   30 Nov 83       M005
**      - (thanks to reubenb).  Changed frame variable to static, it is
**        used as a global buffer.  We never saw the bug before because
**        of the depth of the stack.
**      barrys  03 Jul 84       M006
**      - Updated the usage message to include the 's' and 'w' options
**        and to make the 'n' option a separate entry (uncommented).
**      ericc   26 Dec 84       M007
**      - Replaced the constant 0x7fffffffffffffffL with MAXLONG.
**      ericc   25 Jul 85       M008
**      - made "-r" option display control characters as '^x', as documented.
**      - fixed processing of '\b' so that more doesn't terminate when
**        the sequence "\b\n" is encountered.
**      - changed "Hit Rubout ..." to "Hit Del ...", for ibm keyboards.
**	davidby 9 March 1988	Unmarked
**	- replaced all locally defined functions with library equivalents,
**	- changed from termcap to terminfo
**	- included <values.h> for MAXLONG value
**	- removed most ifdef code for V6, V7, and BSD
**	- added /etc/magic support for file type checking
*/

#include <ctype.h>
#include <signal.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <curses.h>
#include <term.h>
#include <sys/ioctl.h>
#include <setjmp.h>
#include <sys/stat.h>
#include <values.h>
#include <stdlib.h>
#include <stdarg.h>
#include <string.h>
#include <unistd.h>
#include <libgen.h>
#ifdef sun
#include <euc.h>
#include <getwidth.h>
#include <locale.h>
#include <widec.h>
#endif
#include <regex.h>
#include <wctype.h>
#include <limits.h>
#ifdef sun
eucwidth_t wp;
#endif
int     cw[4];
int     scw[4];
#include <locale.h>

#ifndef sun
#define	gettext(txt) txt
#endif

/* Help file will eventually go in libpath(more.help) on all systems */

#ifdef INGRES
#define HELPFILE        "/mntp/doucette/more/more.help"
#define LOCAL_HELP	"/usr/lib/locale/%s/LC_MESSAGES/more.help"
#endif

#ifndef INGRES
#ifndef HELPFILE
#define HELPFILE        "/usr/lib/nvmore.help"
#define LOCAL_HELP	"/usr/lib/locale/%s/LC_MESSAGES/nvmore.help"
#endif
#endif

#define Fopen(s,m)      (Currline = 0,file_pos=0,fopen(s,m))
#define Ftell(f)        file_pos
#define Fseek(f,off)    (file_pos=off,fseeko(f,off,0))
#define Getc(f)         (++file_pos, getc(f))
#define Ungetc(c,f)     (--file_pos, ungetc(c,f))

#define pr(s1)		fputs(s1, stdout)
#define clreos()	putp(clr_eos)
#define cleareol()	putp(clr_eol)
#define home()		putp(cursor_home)

#define LINSIZ  512
#define ctrl(letter)    ((letter) & 077)
#define RUBOUT  '\177'
#define ESC     '\033'
#define QUIT    '\034'

struct termio   otty;           /* old tty modes */
struct termio   ntty;           /* new tty modes */
off_t           file_pos, file_size;
off_t		OFF_T_MAX = ~0;
int             fnum, no_intty, no_tty;
int             dum_opt;
off_t           dlines;
void end_it(int sig);
void onquit(int sig);
void chgwinsz(int sig);
#ifdef SIGTSTP
void             onsusp(int sig);
#endif
int             nscroll = 11;   /* Number of lines scrolled by 'd' */
int             fold_opt = 1;   /* Fold long lines */
int             stop_opt = 1;   /* Stop after form feeds */
int             ssp_opt = 0;    /* Suppress white space */
int             ul_opt = 1;     /* Underline as best we can */
int             cr_opt = 0;     /* show ctrl characters as '^c' */
int             wait_opt = 0;   /* prompt for exit at eof */
int             promptlen;
off_t		Currline;       /* Line we are currently at */
int             startup = 1;
int             firstf = 1;
int             notell = 1;
int             inwait, Pause, errors;
int             within; /* true if we are within a file,
                        false if we are between files */
int             hard, dumb, noscroll, hardtabs, clreol;
int             catch_susp;     /* We should catch the SIGTSTP signal */
char            **fnames;       /* The list of file names */
int             nfiles;         /* Number of files left to process */
int		has_more = 0;	/* We have more stuff to show */	
char            *shell;         /* The name of the shell to use */
int             shellp;         /* A previous shell command exists */
char            ch;
jmp_buf         restore;
char            obuf[BUFSIZ];   /* stdout buffer */
char            Line[LINSIZ];   /* Line buffer */
int             Lpp = 24;       /* lines per page */
char            *ULenter, *ULexit;      /* enter and exit underline mode */
int             Mcol = 80;      /* number of columns */
int             Wrap = 1;       /* set if automargins */
int		fseeko();
struct {
    off_t chrctr, line;
} context, screen_start;
int             exitstat = 0;   /* status to use when exiting more */   /*M003*/

static void error(char *mess);
static void wait_eof(void);
static void prompt(char *filename);
static void argscan(char *s);
static void copy_file(register FILE *f);
static void initterm(void);
static FILE *checkf(register char *fs, int *clearfirst);
static void screen(register FILE *f, register off_t num_lines);
static void skiplns(register off_t n, register FILE *f);
static void skipf(register int nskip);
static int readch(void);
static void prmpt_erase(register int col);
static void kill_line(void);
static void prbuf(register char *s, register int n);
static void search(char buf[], FILE *file, register off_t n);
static void doclear(void);
static void ttyin(char buf[], register int nmax, char pchar);
static void show(register char ch);
static void set_tty(void);
static void reset_tty(void);
static void rdline(register FILE *f);
static off_t command(char *filename, register FILE *f);
static int nvm_getline(register FILE *f, int *length);
static int number(char *cmd);
static int colon(char *filename, int cmd, off_t nlines);

int
main(int argc, char *argv[])
{
    register FILE       *f;
    register char       *s;
    register char       *p;
    register int	ch;
    register off_t      left;
    int                 prnames = 0;
    int                 initopt = 0;
    int                 srchopt = 0;
    int                 clearit = 0;
    off_t               initline = 0;
    char                initbuf[80];

    setlocale( LC_ALL, "" );
#ifdef sun
    getwidth(&wp);
#endif
    cw[0] = 1;
#ifdef sun
    cw[1] = wp._eucw1;
    cw[2] = wp._eucw2+1;
    cw[3] = wp._eucw3+1;
#endif
    scw[0] = 1;
#ifdef sun
    scw[1] = wp._scrw1;
    scw[2] = wp._scrw2;
    scw[3] = wp._scrw3;
#endif

    nfiles = argc;
    fnames = argv;

    (void) setlocale(LC_ALL,"");
#if !defined(TEXT_DOMAIN)	/* Should be defined by cc -D */
#define TEXT_DOMAIN "SYS_TEST"	/* Use this only if it weren't */
#endif
#ifdef sun
	(void) textdomain(TEXT_DOMAIN);
#endif

    initterm ();
    if((s = getenv("MORE"))) argscan(s);
    while (--nfiles > 0) {
        if ((ch = (*++fnames)[0]) == '-') {
            argscan(*fnames+1);
        }
        else if (ch == '+') {
            s = *fnames;
            if (*++s == '/') {
                srchopt++;
                for (++s, p = initbuf; p < initbuf + 79 && *s != '\0';)
                    *p++ = *s++;
                *p = '\0';
            }
            else {
                initopt++;
                for (initline = 0; *s != '\0'; s++)
                    if (isdigit (*s))
                        initline = initline*10 + *s -'0';
                --initline;
            }
        }
        else break;
    }
    /* allow clreol only if cursor_home and clr_eol and clr_eos strings are
     *  defined, and in that case, make sure we are in noscroll mode
     */
    if(clreol)
    {
        if (!cursor_home || !clr_eol || !clr_eos) {
           	clreol = 0;
	}
        else noscroll = 1;
    }

    if (dlines == 0)
        dlines =(off_t) (Lpp - (noscroll ? 1 : 2));
    left = dlines;
    if (nfiles > 1)
        prnames++;
    if (!no_intty && nfiles == 0) {
	fprintf(stderr, gettext("Usage: %s\
 [-cdflrsuw] [-lines] [+linenumber] [+/pattern] [filename ...].\n")
	, argv[0]);
        exit(1);
    }
    else
        f = stdin;
    if (!no_tty) {
        signal(SIGQUIT, onquit);
        signal(SIGINT, end_it);
        signal(SIGWINCH, chgwinsz);
#ifdef SIGTSTP
        if (signal (SIGTSTP, SIG_IGN) == SIG_DFL) {
            signal(SIGTSTP, onsusp);
            catch_susp++;
        }
#endif
    }
    if (no_intty) {
        if (no_tty)
            copy_file (stdin);
        else {
            if ((ch = Getc (f)) == '\f')
                doclear();
            else {
                Ungetc (ch, f);
                if (noscroll && (ch != EOF)) {
                    if (clreol)
                        home ();
                    else
			doclear ();
                }
            }
            if (!setjmp(restore)) {
                if (srchopt) {
                    search (initbuf, stdin,(off_t) 1);
                    if (noscroll)
                        left--;
                    }
                else if (initopt)
                    skiplns (initline, stdin);
                }
            else
                left = command(NULL, f);
            screen (stdin, left);
        }
        no_intty = 0;
        prnames++;
        firstf = 0;
    }

    while (fnum < nfiles) {
        if ((f = checkf (fnames[fnum], &clearit)) != NULL) {
            context.line = context.chrctr = 0;
            Currline = 0;
            if (firstf) setjmp (restore);
            if (firstf) {
                firstf = 0;
                if (srchopt)
                {
                    search (initbuf, f,(off_t) 1);
                    if (noscroll)
                        left--;
                }
                else if (initopt)
                    skiplns (initline, f);
            }
            else if (fnum < nfiles && !no_tty) {
                setjmp (restore);
                left = command (fnames[fnum], f);
            }
            if (left != 0) {
		if ((noscroll || clearit) && (file_size != OFF_T_MAX)) {
                    if (clreol)
                        home ();
                    else
                        doclear ();
		}
                if (prnames) {
                    if (ceol_standout_glitch)
                        prmpt_erase (0);
                    if (clreol)
                        cleareol ();
                    pr("::::::::::::::");
                    if (promptlen > 14)
                        prmpt_erase (14);
                    printf ("\n");
                    if(clreol) cleareol();
                    printf("%s\n", fnames[fnum]);
                    if(clreol) cleareol();
                    pr("::::::::::::::\n");
                    if (left > (off_t)(Lpp - 4))
                        left =(off_t)(Lpp - 4);
                }
                if (no_tty)
                    copy_file (f);
                else {
                    within++;
                    screen(f, left);
                    within = 0;
                }
            }
            setjmp (restore);
            fflush(stdout);
            fclose(f);
            screen_start.line = screen_start.chrctr = 0LL;
            context.line = context.chrctr = 0LL;
        } else
            exitstat |= 1;                      /*M003*/
        fnum++;
        firstf = 0;
    }
    if (wait_opt) 
	wait_eof();
    if (has_more)
    	reset_tty ();
    return (exitstat);                             /*M003*/
}

static void
argscan(char *s)
{
    int seen_num = 0;
    for (; *s != '\0'; s++) {
        if (isdigit(*s)) {
            if (!seen_num) {
                dlines = 0;
                seen_num = 1;
            }
            dlines = dlines*10 + *s - '0';
        }
        else if (*s == 'd')
            dum_opt = 1;
        else if (*s == 'l')
            stop_opt = 0;
        else if (*s == 'f')
            fold_opt = 0;
        else if (*s == 'p')
            noscroll++;
        else if (*s == 'c')
            clreol++;
        else if (*s == 's')
            ssp_opt = 1;
        else if (*s == 'u')
            ul_opt = 0;
        else if (*s == 'r')
            cr_opt = 1;
        else if (*s == 'w')
            wait_opt = 1;
    }
}


/*
** Check whether the file named by fs is a file which the user may
** access.  If it is, return the opened file. Otherwise return NULL.
*/

static FILE *
checkf(register char *fs, int *clearfirst)
{
    struct stat stbuf;
    register FILE *f;
    int c;

    if (stat (fs, &stbuf) == -1) {
        fflush(stdout);
        if (clreol)
            cleareol ();
        perror(fs);
        return (NULL);
    }
    if ((stbuf.st_mode & S_IFMT) == S_IFDIR) {
        printf(gettext("\n*** %s: directory ***\n\n"), fs);
        return (NULL);
    }
    if ((f=Fopen(fs, "r")) == NULL) {
        fflush(stdout);
        perror(fs);
        return (NULL);
    }

    if ((c = Getc(f)) == '\f')                  /* end M001 */
        *clearfirst = 1;
    else {
        *clearfirst = 0;
        Ungetc (c, f);
    }
    if ((file_size = (off_t)stbuf.st_size) == 0)
        file_size = OFF_T_MAX;
    return (f);
}

/*
** Print out the contents of the file f, one screenful at a time.
*/

#define STOP -10

static void
screen(register FILE *f, register off_t num_lines)
{
    register int c;
    register int nchars;
    int length;                 /* length of current line */
    static int prev_len = 1;    /* length of previous line */

    for (;;) {
        while (num_lines > 0 && !Pause) {
            if ((nchars = nvm_getline (f, &length)) == EOF)
            {
                if (clreol) clreos();
                return;
            }
            if (ssp_opt && length == 0 && prev_len == 0)
                continue;
            prev_len = length;
            if (ceol_standout_glitch ||
		((enter_standout_mode && *enter_standout_mode == ' ')
		&& promptlen > 0))
                prmpt_erase (0);
            /* must clear before drawing line since tabs on some terminals
             * do not erase what they tab over.
             */
            if (clreol)
                cleareol ();
            prbuf (Line, length);
            if (nchars < promptlen)
                prmpt_erase (nchars); /* prmpt_erase () sets promptlen to 0 */
            else promptlen = 0;
            /* is this needed?
             * if (clreol)
             *  cleareol(); */    /* must clear again in case we wrapped */

            if (nchars < Mcol || !fold_opt)
                putchar('\n');
            if (nchars == STOP)
                break;
            num_lines--;
        }
        fflush(stdout);
        if ((c = Getc(f)) == EOF)
        {
            if (clreol) clreos ();
            return;
        }

        if (Pause && clreol)
            clreos ();
        Ungetc (c, f);
        setjmp (restore);
        Pause = 0; startup = 0;
        if ((num_lines = command (NULL, f)) == 0)
            return;
        if (hard && promptlen > 0)
                prmpt_erase (0);
        if (noscroll && num_lines == dlines) {
            if (clreol)
                home();
            else
		doclear ();
        }
        screen_start.line = Currline;
        screen_start.chrctr = Ftell (f);
    }
}

/*
** Come here if a quit signal is received
*/
/*
 * sig is put in as a dummy arg to have the compiler not to complain
 */

/* ARGSUSED */
void
onquit(int sig)
{
    signal(SIGQUIT, SIG_IGN);
    if (!inwait) {
        putchar ('\n');
        if (!startup) {
            signal(SIGQUIT, onquit);
            longjmp (restore, 1);
        }
        else
            Pause++;
    }
    else if (!dum_opt && notell) {
        fputs(gettext("[Use q or Q to quit]"), stderr);
        promptlen += 20;
        notell = 0;
    }
    signal(SIGQUIT, onquit);
}

/*
** Come here if a signal for a window size change is received
*/
/*ARGSUSED*/
void
chgwinsz(int sig)
{
    struct winsize win;

    (void) signal(SIGWINCH, SIG_IGN);
    if (ioctl(fileno(stdout), TIOCGWINSZ, &win) != -1) {
	if (win.ws_row != 0) {
	    Lpp = win.ws_row;
	    nscroll = Lpp/2 - 1;
	    if (nscroll <= 0)
		nscroll = 1;
	    dlines = (off_t)(Lpp - (noscroll ? 1 : 2));
	}
	if (win.ws_col != 0)
	    Mcol = win.ws_col;
    }
    (void) signal(SIGWINCH, chgwinsz);
}

/*
** Clean up terminal state and exit. Also come here if interrupt signal received
*/

/*
 * sig is put in as a dummy arg to have the compiler not to complain
 */

/* ARGSUSED */
void
end_it(int sig)
{

    if (has_more)
    	reset_tty ();
    if (clreol) {
        putchar ('\r');
        clreos ();
        fflush (stdout);
    }
    else if (!clreol && (promptlen > 0)) {
        kill_line ();
        fflush (stdout);
    }
    else
        fputs("\n", stderr);
    _exit(exitstat);                    /*M003*/
}

static void
copy_file(register FILE *f)
{
    register int c;

    while ((c = getc(f)) != EOF)
        putchar(c);
}

static char Bell = ctrl('G');


/* See whether the last component of the path name "path" is equal to the
** string "string"
*/

int
tailequ(char *path, char *string)
{
	return (!strcmp(basename(path), string));
}

static void
prompt(char *filename)
{
    if (clreol)
        cleareol ();
    else if (promptlen > 0)
        kill_line ();
    if (!hard) {
        promptlen = 8;
        if (enter_standout_mode && exit_standout_mode)
            putp (enter_standout_mode);
        if (clreol)
            cleareol ();
        pr(gettext("--More--"));
        if (filename != NULL) {
            promptlen += printf (gettext("(Next file: %s)"), filename);
        }
        else if (!no_intty) {
            promptlen += printf ("(%d%%)", (int)((file_pos * 100) / file_size));
        }
        if (dum_opt) {
            promptlen += pr(gettext("[Hit space to continue, Del to abort]"));
        }
        if (enter_standout_mode && exit_standout_mode)
            putp (exit_standout_mode);
        if (clreol) clreos ();
        fflush(stdout);
    }
    else
        fputs(&Bell, stderr);
    inwait++;
}

/*
 * when run from another program or a shell script, it is
 * sometimes useful to prevent the next program from scrolling
 * us off the screen before we get a chance to read this page.
 *                      -Hans, July 24, 1982
 */
static void
wait_eof(void)
{
        if (enter_standout_mode && exit_standout_mode)
                putp (enter_standout_mode);
        promptlen = pr(gettext("--End--"));          /*M003*/
        if (dum_opt)
                promptlen += pr(gettext("[Hit any key to continue]"));
        if (enter_standout_mode && exit_standout_mode)
                putp(exit_standout_mode);
        if (clreol) clreos();
        fflush(stdout);
        readch();
        prmpt_erase (0);
        fflush(stdout);
}

/*
** Get a logical line
*/

static int
nvm_getline(register FILE *f, int *length)
{
    register int        c;
    register char       *p;
    register int        column;
    static int          colflg;
    register int        oldcolumn;
    int			csno;

    p = Line;
    column = 0;
    oldcolumn = 0;
    c = Getc (f);
    if (colflg && c == '\n') {
        Currline++;
        c = Getc (f);
    }
    while (p < &Line[LINSIZ - 1]) {
#ifdef sun
	csno = csetno(c);
#else
	csno = 0;
#endif
        if (c == EOF) {
            if (p > Line) {
                *p = '\0';
                *length = p - Line;
                return (column);
            }
            *length = p - Line;
            return (EOF);
        }
	if (!csno) {
	        if (c == '\n') {
	            /* detect \r\n.  -Hans */
	            if (p>Line && p[-1] == '\r') {
	                column = oldcolumn;
	                p--;
	            }
	            Currline++;
	            break;
	        }
	        *p++ = c;
	        if (c == '\t')
	            if (hardtabs && column < promptlen && !hard) {
	                if (clr_eol && !dumb) {
	                    column = 1 + (column | 7);
	                    putp (clr_eol);
	                    promptlen = 0;
	                }
	                else {
	                    for (--p; column & 7 && p < &Line[LINSIZ - 1]; column++) {
	                        *p++ = ' ';
	                    }
	                    if (column >= promptlen) promptlen = 0;
	                }
	            }
	            else
	                column = 1 + (column | 7);
	        else if ((c == '\b') && (ul_opt || !cr_opt) && (column > 0))  /* M008 */
	                column--;

	        /* this is sort of strange.  what was here before was that
	           \r always set column to zero, and the hack above to
	           detect \r\n didnt exist.  the net effect is to make
	           the current line be overwritten by the prompt if it
	           had a \r at the end, and the line start after the \r
	           otherwise.  I suppose this is useful for overstriking
	           on hard copy terminals, but not on anything glass
	           -Hans */

	        else if ((c == '\r') && !cr_opt) {
	                oldcolumn = column;
	                column = 0;
	        }
	        else if (c == '\f' && stop_opt) {
	                p[-1] = '^';
	                *p++ = 'L';
	                column += 2;
	                Pause++;
	        }
	        else if (c == EOF) {
	            *length = p - Line;
	            return (column);
	        }
	        else if (c < ' ' && cr_opt){                    /* M008 begin */
	                p[-1] = '^';
	                *p++ = c | ('A' - 1);
	                column += 2;
	        }                                               /* M008 end */
	        else if (c >= ' ' && c != RUBOUT)
	            column++;
	} /* end of code set 0 */
	else {
		column += scw[csno];
		if ( column > Mcol && fold_opt ) {
		    column -= scw[csno];
		    while ( column < Mcol ) {
		        column++;
		        *p++ = ' ';
		    }
		    column = Mcol;
		    Ungetc(c,f);
		} else {
		    int i;
		    *p++ = c;
		    for(i=1; i<cw[csno];i++)
			*p++ = Getc(f);
		}
	} /* end of codeset 1 ~ 3 */
        if (column >= Mcol && fold_opt) break;
        c = Getc (f);
    }
    if (column >= Mcol && Mcol > 0) {
        if (!Wrap) {
            *p++ = '\n';
        }
    }
    colflg = column == Mcol && fold_opt;
    if (colflg && eat_newline_glitch && Wrap) {
	*p++ = '\n'; /* simulate normal wrap */
    }
    *length = p - Line;
    *p = 0;
    return (column);
}

/*
** Erase the rest of the prompt, assuming we are starting at column col.
*/

static void
prmpt_erase(register int col)
{

    if (promptlen == 0)
        return;
    if (hard) {
        putchar ('\n');
    }
    else {
        if (col == 0)
            putchar ('\r');
        if (!dumb && clr_eol)
            putp (clr_eol);
        else
            for (col = promptlen - col; col > 0; col--)
                putchar (' ');
    }
    promptlen = 0;
}

/*
** Erase the current line entirely
*/

static void
kill_line(void)
{
    prmpt_erase (0);
    if (!clr_eol || dumb) putchar ('\r');
}

/* Print a buffer of n characters */

static void
prbuf(register char *s, register int n)
{
    char c;                             /* next ouput character */
    register int state = 0;             /* next output char's UL state */
    static int pstate = 0;              /* current terminal UL state (off) */

    while (--n >= 0)
        if (!ul_opt)
            putchar (*s++);
        else {
            if (n >= 2 && s[0] == '_' && s[1] == '\b') {
                n -= 2;
                s += 2;
                c = *s++;
                state = 1;
            } else if (n >= 2 && s[1] == '\b' && s[2] == '_') {
                n -= 2;
                c = *s++;
                s += 2;
                state = 1;
            } else {
                c = *s++;
                state = 0;
            }
            if (state != pstate)
                putp(state ? ULenter : ULexit);
            pstate = state;
            putchar(c);
            if (state && underline_char) {
                putp(cursor_left);
                putp(underline_char);
            }
        }
    /*
     * M002
     * You don't want to stay in standout mode at the end of the line;
     * on some terminals, this will leave all of the remaining blank
     * space on the line in standout mode.
     */
    if (state && !underline_char) {                       /*M002*/
            putp(ULexit);                    /*M002*/
            pstate = 0;                                 /*M002*/
    }                                                   /*M002*/
}

/*
**  Clear the screen
*/

static void
doclear(void)
{
	/*
	 * This function gets called because noscroll is
	 * set to so that column headers on multiple output screens
	 * always stay at the top. However we want to skip this
	 * because it will clear the screen everytime before
	 * showing the output.
	 */
	if (1) {
		return;
	}

    if (clear_screen && !hard) {
        putp(clear_screen);

        /* Put out carriage return so that system doesn't
        ** get confused by escape sequences when expanding tabs
        */
        putchar ('\r');
        promptlen = 0;
    }
}


static int lastcmd, lastp;
static off_t lastarg;
static int lastcolon;
char shell_line[PATH_MAX];

/*
** Read a command and do it. A command consists of an optional integer
** argument followed by the command character.  Return the number of lines
** to display in the next screenful.  If there is nothing more to display
** in the current file, zero is returned.
*/

static off_t 
command(char *filename, register FILE *f)
{
    register off_t nlines;
    register off_t retval = 0;
    register int c;
    char colonch;
    FILE *helpf;
    int done;
    char comchar, cmdbuf[80];
    char filebuf[128];
    char *loc;

#define ret(val) retval=val;done++;break

    /* we are in here because there are more stuff to show */
    has_more = 1;

    if (has_more) {
        set_tty();
    }

    done = 0;
    if (!errors)
        prompt (filename);
    else
        errors = 0;
    for (;;) {
        nlines = number (&comchar);
        lastp = colonch = 0;
        if (comchar == '.') {   /* Repeat last command */
                lastp++;
                comchar = lastcmd;
                nlines = lastarg;
                if (lastcmd == ':')
                        colonch = lastcolon;
        }
        lastcmd = comchar;
        lastarg = nlines;
	if((comchar != RUBOUT) && !dum_opt) {
        	if (comchar == otty.c_cc[VERASE]) {
           		 kill_line ();
            		prompt (filename);
            		continue;
        	}
	}
        switch (comchar) {
        case ':':
            retval = colon (filename, colonch, nlines);
            if (retval >= 0)
                done++;
            break;
	case 'b':
	case ctrl('B'):
	    {
		register off_t initline;

		if (no_intty) {
		    fputs(&Bell, stderr);
		    return (-1);
		}

		if (nlines == 0) nlines++;

		putchar ('\r');
		prmpt_erase (0);
		printf ("\n");
		if (clreol)
			cleareol ();
		printf (gettext("...back %lld page"), (long long)nlines);
		if (nlines > 1)
			pr ("s\n");
		else
			pr ("\n");

		if (clreol)
			cleareol ();
		pr ("\n");

		initline = Currline - dlines * (nlines + 1);
		if (! noscroll)
		    --initline;
		if (initline < 0) initline = 0;
		Fseek(f, 0LL);
		Currline = 0;	/* skiplns() will make Currline correct */
		skiplns(initline, f);
		if (! noscroll) {
		    ret(dlines + 1);
		}
		else {
		    ret(dlines);
		}
	    }
        case ' ':
        case 'z':
            if (nlines == 0) nlines = dlines;
            else if (comchar == 'z') dlines = nlines;
            ret (nlines);
        case 'd':
        case ctrl('D'):
            if (nlines != 0) nscroll = nlines;
            ret (nscroll);
        case RUBOUT:
        case 'q':
        case 'Q':
            end_it(0);
	    /*NOTREACHED*/
        case 's':
        case 'f':
            if (nlines == 0) nlines++;
            if (comchar == 'f')
                nlines *= dlines;
            putchar ('\r');
            prmpt_erase (0);
            printf ("\n");
            if (clreol)
                cleareol ();
            printf (gettext("...skipping %lld line"), (long long)nlines);
            if (nlines > 1)
                pr ("s\n");
            else
                pr ("\n");

            if (clreol)
                cleareol ();
            pr ("\n");

            while (nlines > 0) {
                while ((c = Getc (f)) != '\n') {
                    if (c == EOF) {
                        retval = 0;
                        done++;
                        goto endsw;
                    }
		}
                Currline++;
                nlines--;
            }
            ret (dlines);
        case '\n':
	    if (nlines != 0)
                dlines = nlines;
            else
                nlines = 1;
            ret (nlines);
        case '\f':
            if (!no_intty) {
		doclear ();
                Fseek (f, screen_start.chrctr);
                Currline = screen_start.line;
                ret (dlines);
            }
            else {
		fputs(&Bell, stderr);
                break;
            }
        case '\'':
            if (!no_intty) {
                kill_line ();
                pr (gettext("\n***Back***\n\n"));
                Fseek (f, context.chrctr);
                Currline = context.line;
                ret (dlines);
            }
            else {
		fputs(&Bell, stderr);
                break;
            }
        case '=':
            kill_line ();
            promptlen = printf ("%lld", (long long)Currline);
            fflush (stdout);
            break;
        case 'n':
            lastp++;
	    /*FALLTHROUGH*/
        case '/':
            if (nlines == 0) nlines++;
            kill_line ();
            pr ("/");
            promptlen = 1;
            fflush (stdout);
            if (lastp) {
		fputs("\r", stderr);
                search (NULL, f, nlines);       /* Use previous r.e. */
            }
            else {
                ttyin (cmdbuf, 78, '/');
		fputs("\r", stderr);
                search (cmdbuf, f, nlines);
            }
            ret (dlines-1);
        case 'h':
        case '?':
	    /*
	     * First get local help file.
	     */
	    loc = setlocale(LC_MESSAGES, 0);
	    sprintf(filebuf, LOCAL_HELP, loc);

            if  ((strcmp(loc, "C") == 0) || (helpf = fopen (filebuf, "r")) == NULL) {
		    if  ((helpf = fopen (HELPFILE, "r")) == NULL)
			error (gettext("Can't open help file"));
	    }
            if (noscroll) doclear ();
            copy_file (helpf);
            fclose (helpf);
            prompt (filename);
            break;
        default:
		if (dum_opt) {
			kill_line ();
		    	promptlen = pr(gettext("[Press 'h' for instructions.]"));
			fflush (stdout);
	    	}
	    	else
			fputs(&Bell, stderr);
            break;
        }
        if (done) break;
    }
    putchar ('\r');
endsw:
    inwait = 0;
    notell++;

    if (has_more)
    	reset_tty ();

    return (retval);
}

char ch;

/*
 * Execute a colon-prefixed command.
 * Returns <0 if not a command that should cause
 * more of the file to be printed.
 */

static int
colon(char *filename, int cmd, off_t nlines)
{
        if (cmd == 0)
                ch = readch ();
        else
                ch = cmd;
        lastcolon = ch;
        switch (ch) {
        case 'n':
                if (nlines == 0) {
                        if (fnum >= nfiles - 1)
                                end_it(0);
                        nlines++;
                }
                putchar ('\r');
                prmpt_erase (0);
                skipf ((int)nlines);
                return (0);
        case 'p':
                if (no_intty) {
			fputs(&Bell, stderr);
                        return (-1);
                }
                putchar ('\r');
                prmpt_erase (0);
                if (nlines == 0)
                        nlines++;
                skipf ((int)-nlines);
                return (0);
        case 'q':
        case 'Q':
                end_it(0);
        default:
		fputs(&Bell, stderr);
                return (-1);
        }
}

/*
** Read a decimal number from the terminal. Set cmd to the non-digit which
** terminates the number.
*/

static int
number(char *cmd)
{
        register int i;

        i = 0; ch = otty.c_cc[VKILL];
        for (;;) {
                ch = readch ();
                if (ch >= '0' && ch <= '9') {
                        i = i*10 + ch - '0';
                } else if (ch == RUBOUT) {
                        i = 0;
                        *cmd = ch;
                        break;
                } else if (ch == otty.c_cc[VKILL]) {
                        i = 0;
                } else {
                        *cmd = ch;
                        break;
                }
        }
        return (i);
}

/*
** Search for nth ocurrence of regular expression contained in buf in the file
*/

static void
search(char buf[], FILE *file, register off_t n)
{
    off_t startline = Ftell (file);
    register off_t line1 = startline;
    register off_t line2 = startline;
    register off_t line3 = startline;
    register off_t lncount;
    off_t saveln;
    static regex_t re;
    static int have_re = 0;
    static char lastbuf[80];

    if (buf != NULL) {
	if (have_re) {
		regfree(&re);
		have_re = 0;
	}
	if (*buf != '\0') {
		if (regcomp(&re, buf, REG_NOSUB | REG_EXTENDED) != 0) {
			error(gettext("Regular expression botch"));
		} else {
			have_re = 1;
			strcpy(lastbuf, buf);
		}
	} else {
		if (regcomp(&re, lastbuf, REG_NOSUB | REG_EXTENDED) != 0) {
			error(gettext("No previous regular expression"));
		} else {
			have_re = 1;
		}
	}
    } else {
	if (!have_re)
	    error(gettext("No previous regular expression"));
    }
    context.line = saveln = Currline;
    context.chrctr = startline;
    lncount = 0;
    while (!feof (file)) {
        line3 = line2;
        line2 = line1;
        line1 = Ftell (file);
        rdline (file);
        lncount++;
        if (regexec(&re, Line, 0, NULL, 0) == 0)
                if (--n == 0) {
                    if (lncount > 3 || (lncount > 1 && no_intty))
                    {
                        pr ("\n");
                        if (clreol)
                            cleareol ();
                        pr(gettext("...skipping\n"));
                    }
                    if (!no_intty) {
                        Currline -= (lncount >= 3 ? 3 : lncount);
                        Fseek (file, line3);
                        if (noscroll) {
                            if (clreol) {
                                home ();
                                cleareol ();
                            }
                            else
                                doclear ();
			}
                    }
                    else {
                        kill_line ();
                        if (noscroll) {
                            if (clreol) {
                                home ();
                                cleareol ();
                            } else
                                doclear ();
			}
                        pr (Line);
                        putchar ('\n');
                    }
                    break;
                }
    }
    if (feof (file)) {
        if (!no_intty) {
            Currline = saveln;
            Fseek (file, startline);
        }
        else {
            pr (gettext("\nPattern not found\n"));
            end_it (0);
        }
        error (gettext("Pattern not found"));
    }
}

/*
** Skip n lines in the file f
*/

static void
skiplns(register off_t n, register FILE *f)
{
    register int c;

    while (n > 0) {
        while ((c = Getc (f)) != '\n') {
            if (c == EOF)
                return;
	}
        n--;
        Currline++;
    }
}

/*
** Skip nskip files in the file list (from the command line). Nskip may be
** negative.
*/

static void
skipf(register int nskip)
{
    if (nskip == 0) return;
    if (nskip > 0) {
        if (fnum + nskip > nfiles - 1)
            nskip = nfiles - fnum - 1;
    }
    else if (within)
        ++fnum;
    fnum += nskip;
    if (fnum < 0)
        fnum = 0;
    pr (gettext("\n...Skipping "));
    pr ("\n");
    if (clreol)
        cleareol ();
    if (nskip > 0)
	printf(gettext("...Skipping to file %s\n"), fnames[fnum]);
    else
	printf(gettext("...Skipping back to file %s\n"), fnames[fnum]);
    if (clreol)
        cleareol ();
    pr ("\n");
    --fnum;
}

/*----------------------------- Terminal I/O -------------------------------*/

static void
initterm(void)
{
    int         erret = 0;
    char	*term = getenv("TERM");

    setbuf(stdout, obuf);
    if (!(no_tty = ioctl(1, TCGETA, &otty))) {
	if (setupterm(term, 1, &erret) != OK) {
            dumb++; ul_opt = 0;
        }
        else {
	    reset_shell_mode();
            if (((Lpp = lines) < 0) || hard_copy) {
                hard++; /* Hard copy terminal */
                Lpp = 24;
            }
            if (tailequ(fnames[0], "page") || (!hard && (scroll_forward == NULL)))
                noscroll++;
            if ((Mcol = columns) < 0)
                Mcol = 80;
            Wrap = tigetflag("am");
            /*
             *  Set up for underlining:  some terminals don't need it;
             *  others have start/stop sequences, still others have an
             *  underline char sequence which is assumed to move the
             *  cursor forward one character.  If underline sequence
             *  isn't available, settle for standout sequence.
             */

            if (transparent_underline || over_strike)
                ul_opt = 0;
            if ((ULenter = tigetstr("smul")) == NULL &&
                (!underline_char) && (ULenter = tigetstr("smso")) == NULL)
                ULenter = "";
            if ((ULexit = tigetstr("rmul")) == NULL &&
                (!underline_char) && (ULexit = tigetstr("rmso")) == NULL)
                ULexit = "";
        }
	shell = "/dev/null";
    }
    no_intty = ioctl(0, TCGETA, &otty);
    ioctl(2, TCGETA, &otty);
    hardtabs = !(otty.c_oflag & TAB3);
}

static int
readch(void)
{
        char ch;
        extern int errno;

        if (read (2, &ch, 1) <= 0) {
                if (errno != EINTR)
                        end_it(0);		/* clean up before exiting */
                else
                        ch = otty.c_cc[VKILL];
	}
        return (ch);
}

static char BS = '\b';
static char CARAT = '^';

static void
ttyin(char buf[], register int nmax, char pchar)
{
    register char *sptr;
    register unsigned char ch;
    int LengthBuffer[80];
    int *BufferPointer;
    int csno;
    register int slash = 0;
    int maxlen;
    char cbuf;

    BufferPointer = LengthBuffer;
    sptr = buf;
    maxlen = 0;
    while (sptr - buf < nmax) {
        if (promptlen > maxlen)
	    maxlen = promptlen;
        ch = readch ();
#ifdef sun
        csno = csetno(ch);
#else
	csno = 0;
#endif
        if (!csno) {
            if (ch == '\\') {
                slash++;
            } else if ((ch == otty.c_cc[VERASE]) && !slash) {
                if (sptr > buf) {
                    --promptlen;
		    fputs(&BS, stderr);
		    sptr -= (*--BufferPointer);
                    if ((*sptr < ' ' && *sptr != '\n') || *sptr == RUBOUT) {
                        --promptlen;
			fputs(&BS, stderr);
                    }
                    continue;
                } else {
                    if (!clr_eol)
			promptlen = maxlen;
                    longjmp (restore, 1);
                }
            } else if ((ch == otty.c_cc[VKILL]) && !slash) {
                if (hard) {
                    show(ch);
                    putchar ('\n');
                    putchar (pchar);
                } else {
                    putchar ('\r');
		    putchar (pchar);
                    if (clr_eol)
                        prmpt_erase (1);
                    promptlen = 1;
                }
                sptr = buf;
                fflush (stdout);
                continue;
            }
            if (slash && (ch == otty.c_cc[VKILL] || ch == otty.c_cc[VERASE])) {
		fputs(&BS, stderr);
	        sptr -= (*--BufferPointer);
            }
            if (ch != '\\')
                slash = 0;
	    *BufferPointer++ = 1;
            *sptr++ = ch;
            if ((ch < ' ' && ch != '\n' && ch != ESC) || ch == RUBOUT) {
                ch += ch == RUBOUT ? -0100 : 0100;
		fputs(&CARAT, stderr);
                promptlen++;
            }
            cbuf = ch;
            if (ch != '\n' && ch != ESC) {
		fputs(&cbuf, stderr);
                promptlen++;
            } else
                break;
            /* end of code set 0 */
        } else {
	    int i;
	    u_char buffer[5];

	    *BufferPointer++ = cw[csno];
	    buffer[0] = *sptr++ = ch;
	    for(i=1; i<cw[csno]; i++) {
	        buffer[i] = *sptr++ = readch();
	    }
	    buffer[i]='\0';
	    fputs(buffer, stderr);
	}
    }
    *--sptr = '\0';
    if (!clr_eol) promptlen = maxlen;
    if (sptr - buf >= nmax - 1)
        error (gettext("Line too long"));
}

static void
show(register char ch)
{
    char cbuf;

    if ((ch < ' ' && ch != '\n' && ch != ESC) || ch == RUBOUT) {
        ch += ch == RUBOUT ? -0100 : 0100;
	fputs(&CARAT, stderr);
        promptlen++;
    }
    cbuf = ch;
    fputs(&cbuf, stderr);
    promptlen++;
}

static void
error (char *mess)
{
    if (clreol)
        cleareol ();
    else
        kill_line ();
    promptlen += strlen (mess);
    if (enter_standout_mode && exit_standout_mode) {
        putp (enter_standout_mode);
        pr(mess);
        putp (exit_standout_mode);
    }
    else
        pr (mess);
    fflush(stdout);
    errors++;
    longjmp (restore, 1);
}


static void
set_tty(void)
{
        ioctl(2, TCGETA, &otty);     /* save old tty modes */
        ioctl(2, TCGETA, &ntty);
        ntty.c_lflag &= ~ECHO & ~ICANON;
        ntty.c_cc[VMIN] = (char)1;
        ntty.c_cc[VTIME] = (char)0;
        ioctl (2, TCSETAF, &ntty);        /* set new tty modes */
}

static void
reset_tty(void)
{
        ioctl (2, TCSETAF, &otty);        /* reset tty modes */
}

static void
rdline(register FILE *f)
{
    register int c;
    register char *p;

    p = Line;
    while ((c = Getc (f)) != '\n' && c != EOF && p - Line < LINSIZ - 1)
        *p++ = c;
    if (c == '\n')
        Currline++;
    *p = '\0';
}

/* Come here when we get a suspend signal from the terminal */

/*
 * sig is put in as a dummy arg to have the compiler not to complain
 */
#ifdef SIGTSTP
/* ARGSUSED */
void
onsusp(int sig)
{
    /* ignore SIGTTOU so we don't get stopped if csh grabs the tty */
    signal(SIGTTOU, SIG_IGN);
    if (has_more)
    	reset_tty ();
    fflush (stdout);
    signal(SIGTTOU, SIG_DFL);

    /* Send the TSTP signal to suspend our process group */
    kill (0, SIGTSTP);
    /* Pause for station break */

    /* We're back */
    signal (SIGTSTP, onsusp);
    if (has_more)
        set_tty ();
    if (inwait)
            longjmp (restore, 1);
}
#endif
