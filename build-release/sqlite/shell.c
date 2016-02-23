/*
** 2001 September 15
**
** The author disclaims copyright to this source code.  In place of
** a legal notice, here is a blessing:
**
**    May you do good and not evil.
**    May you find forgiveness for yourself and forgive others.
**    May you share freely, never taking more than you give.
**
*************************************************************************
** This file contains code to implement the "sqlite" command line
** utility for accessing SQLite databases.
*/
#if (defined(_WIN32) || defined(WIN32)) && !defined(_CRT_SECURE_NO_WARNINGS)
/* This needs to come before any includes for MSVC compiler */
#define _CRT_SECURE_NO_WARNINGS
#endif

/*
** If requested, include the SQLite compiler options file for MSVC.
*/
#if defined(INCLUDE_MSVC_H)
#include "msvc.h"
#endif

/*
** No support for loadable extensions in VxWorks.
*/
#if (defined(__RTP__) || defined(_WRS_KERNEL)) && !SQLITE_OMIT_LOAD_EXTENSION
# define SQLITE_OMIT_LOAD_EXTENSION 1
#endif

/*
** Enable large-file support for fopen() and friends on unix.
*/
#ifndef SQLITE_DISABLE_LFS
# define _LARGE_FILE       1
# ifndef _FILE_OFFSET_BITS
#   define _FILE_OFFSET_BITS 64
# endif
# define _LARGEFILE_SOURCE 1
#endif

#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <assert.h>
#include "sqlite3.h"
#if SQLITE_USER_AUTHENTICATION
# include "sqlite3userauth.h"
#endif
#include <ctype.h>
#include <stdarg.h>

#if !defined(_WIN32) && !defined(WIN32)
# include <signal.h>
# if !defined(__RTP__) && !defined(_WRS_KERNEL)
#  include <pwd.h>
# endif
# include <unistd.h>
# include <sys/types.h>
#endif

#if HAVE_READLINE
# include <readline/readline.h>
# include <readline/history.h>
#endif

#if HAVE_EDITLINE
# include <editline/readline.h>
#endif

#if HAVE_EDITLINE || HAVE_READLINE

# define shell_add_history(X) add_history(X)
# define shell_read_history(X) read_history(X)
# define shell_write_history(X) write_history(X)
# define shell_stifle_history(X) stifle_history(X)
# define shell_readline(X) readline(X)

#elif HAVE_LINENOISE

# include "linenoise.h"
# define shell_add_history(X) linenoiseHistoryAdd(X)
# define shell_read_history(X) linenoiseHistoryLoad(X)
# define shell_write_history(X) linenoiseHistorySave(X)
# define shell_stifle_history(X) linenoiseHistorySetMaxLen(X)
# define shell_readline(X) linenoise(X)

#else

# define shell_read_history(X) 
# define shell_write_history(X)
# define shell_stifle_history(X)

# define SHELL_USE_LOCAL_GETLINE 1
#endif


#if defined(_WIN32) || defined(WIN32)
# include <io.h>
# include <fcntl.h>
#define isatty(h) _isatty(h)
#ifndef access
# define access(f,m) _access((f),(m))
#endif
#undef popen
#define popen _popen
#undef pclose
#define pclose _pclose
#else
/* Make sure isatty() has a prototype.
*/
extern int isatty(int);

#if !defined(__RTP__) && !defined(_WRS_KERNEL)
  /* popen and pclose are not C89 functions and so are sometimes omitted from
  ** the <stdio.h> header */
  extern FILE *popen(const char*,const char*);
  extern int pclose(FILE*);
#else
# define SQLITE_OMIT_POPEN 1
#endif

#endif

#if defined(_WIN32_WCE)
/* Windows CE (arm-wince-mingw32ce-gcc) does not provide isatty()
 * thus we always assume that we have a console. That can be
 * overridden with the -batch command line option.
 */
#define isatty(x) 1
#endif

/* ctype macros that work with signed characters */
#define IsSpace(X)  isspace((unsigned char)X)
#define IsDigit(X)  isdigit((unsigned char)X)
#define ToLower(X)  (char)tolower((unsigned char)X)

/* On Windows, we normally run with output mode of TEXT so that \n characters
** are automatically translated into \r\n.  However, this behavior needs
** to be disabled in some cases (ex: when generating CSV output and when
** rendering quoted strings that contain \n characters).  The following
** routines take care of that.
*/
#if defined(_WIN32) || defined(WIN32)
static void setBinaryMode(FILE *out){
  fflush(out);
  _setmode(_fileno(out), _O_BINARY);
}
static void setTextMode(FILE *out){
  fflush(out);
  _setmode(_fileno(out), _O_TEXT);
}
#else
# define setBinaryMode(X)
# define setTextMode(X)
#endif


/* True if the timer is enabled */
static int enableTimer = 0;

/* Return the current wall-clock time */
static sqlite3_int64 timeOfDay(void){
  static sqlite3_vfs *clockVfs = 0;
  sqlite3_int64 t;
  if( clockVfs==0 ) clockVfs = sqlite3_vfs_find(0);
  if( clockVfs->iVersion>=1 && clockVfs->xCurrentTimeInt64!=0 ){
    clockVfs->xCurrentTimeInt64(clockVfs, &t);
  }else{
    double r;
    clockVfs->xCurrentTime(clockVfs, &r);
    t = (sqlite3_int64)(r*86400000.0);
  }
  return t;
}

#if !defined(_WIN32) && !defined(WIN32) && !defined(__minux)
#include <sys/time.h>
#include <sys/resource.h>

/* VxWorks does not support getrusage() as far as we can determine */
#if defined(_WRS_KERNEL) || defined(__RTP__)
struct rusage {
  struct timeval ru_utime; /* user CPU time used */
  struct timeval ru_stime; /* system CPU time used */
};
#define getrusage(A,B) memset(B,0,sizeof(*B))
#endif

/* Saved resource information for the beginning of an operation */
static struct rusage sBegin;  /* CPU time at start */
static sqlite3_int64 iBegin;  /* Wall-clock time at start */

/*
** Begin timing an operation
*/
static void beginTimer(void){
  if( enableTimer ){
    getrusage(RUSAGE_SELF, &sBegin);
    iBegin = timeOfDay();
  }
}

/* Return the difference of two time_structs in seconds */
static double timeDiff(struct timeval *pStart, struct timeval *pEnd){
  return (pEnd->tv_usec - pStart->tv_usec)*0.000001 + 
         (double)(pEnd->tv_sec - pStart->tv_sec);
}

/*
** Print the timing results.
*/
static void endTimer(void){
  if( enableTimer ){
    sqlite3_int64 iEnd = timeOfDay();
    struct rusage sEnd;
    getrusage(RUSAGE_SELF, &sEnd);
    printf("Run Time: real %.3f user %f sys %f\n",
       (iEnd - iBegin)*0.001,
       timeDiff(&sBegin.ru_utime, &sEnd.ru_utime),
       timeDiff(&sBegin.ru_stime, &sEnd.ru_stime));
  }
}

#define BEGIN_TIMER beginTimer()
#define END_TIMER endTimer()
#define HAS_TIMER 1

#elif (defined(_WIN32) || defined(WIN32))

#include <windows.h>

/* Saved resource information for the beginning of an operation */
static HANDLE hProcess;
static FILETIME ftKernelBegin;
static FILETIME ftUserBegin;
static sqlite3_int64 ftWallBegin;
typedef BOOL (WINAPI *GETPROCTIMES)(HANDLE, LPFILETIME, LPFILETIME,
                                    LPFILETIME, LPFILETIME);
static GETPROCTIMES getProcessTimesAddr = NULL;

/*
** Check to see if we have timer support.  Return 1 if necessary
** support found (or found previously).
*/
static int hasTimer(void){
  if( getProcessTimesAddr ){
    return 1;
  } else {
    /* GetProcessTimes() isn't supported in WIN95 and some other Windows
    ** versions. See if the version we are running on has it, and if it
    ** does, save off a pointer to it and the current process handle.
    */
    hProcess = GetCurrentProcess();
    if( hProcess ){
      HINSTANCE hinstLib = LoadLibrary(TEXT("Kernel32.dll"));
      if( NULL != hinstLib ){
        getProcessTimesAddr =
            (GETPROCTIMES) GetProcAddress(hinstLib, "GetProcessTimes");
        if( NULL != getProcessTimesAddr ){
          return 1;
        }
        FreeLibrary(hinstLib); 
      }
    }
  }
  return 0;
}

/*
** Begin timing an operation
*/
static void beginTimer(void){
  if( enableTimer && getProcessTimesAddr ){
    FILETIME ftCreation, ftExit;
    getProcessTimesAddr(hProcess,&ftCreation,&ftExit,
                        &ftKernelBegin,&ftUserBegin);
    ftWallBegin = timeOfDay();
  }
}

/* Return the difference of two FILETIME structs in seconds */
static double timeDiff(FILETIME *pStart, FILETIME *pEnd){
  sqlite_int64 i64Start = *((sqlite_int64 *) pStart);
  sqlite_int64 i64End = *((sqlite_int64 *) pEnd);
  return (double) ((i64End - i64Start) / 10000000.0);
}

/*
** Print the timing results.
*/
static void endTimer(void){
  if( enableTimer && getProcessTimesAddr){
    FILETIME ftCreation, ftExit, ftKernelEnd, ftUserEnd;
    sqlite3_int64 ftWallEnd = timeOfDay();
    getProcessTimesAddr(hProcess,&ftCreation,&ftExit,&ftKernelEnd,&ftUserEnd);
    printf("Run Time: real %.3f user %f sys %f\n",
       (ftWallEnd - ftWallBegin)*0.001,
       timeDiff(&ftUserBegin, &ftUserEnd),
       timeDiff(&ftKernelBegin, &ftKernelEnd));
  }
}

#define BEGIN_TIMER beginTimer()
#define END_TIMER endTimer()
#define HAS_TIMER hasTimer()

#else
#define BEGIN_TIMER 
#define END_TIMER
#define HAS_TIMER 0
#endif

/*
** Used to prevent warnings about unused parameters
*/
#define UNUSED_PARAMETER(x) (void)(x)

/*
** If the following flag is set, then command execution stops
** at an error if we are not interactive.
*/
static int bail_on_error = 0;

/*
** Threat stdin as an interactive input if the following variable
** is true.  Otherwise, assume stdin is connected to a file or pipe.
*/
static int stdin_is_interactive = 1;

/*
** The following is the open SQLite database.  We make a pointer
** to this database a static variable so that it can be accessed
** by the SIGINT handler to interrupt database processing.
*/
static sqlite3 *globalDb = 0;

/*
** True if an interrupt (Control-C) has been received.
*/
static volatile int seenInterrupt = 0;

/*
** This is the name of our program. It is set in main(), used
** in a number of other places, mostly for error messages.
*/
static char *Argv0;

/*
** Prompt strings. Initialized in main. Settable with
**   .prompt main continue
*/
static char mainPrompt[20];     /* First line prompt. default: "sqlite> "*/
static char continuePrompt[20]; /* Continuation prompt. default: "   ...> " */

/*
** Write I/O traces to the following stream.
*/
#ifdef SQLITE_ENABLE_IOTRACE
static FILE *iotrace = 0;
#endif

/*
** This routine works like printf in that its first argument is a
** format string and subsequent arguments are values to be substituted
** in place of % fields.  The result of formatting this string
** is written to iotrace.
*/
#ifdef SQLITE_ENABLE_IOTRACE
static void SQLITE_CDECL iotracePrintf(const char *zFormat, ...){
  va_list ap;
  char *z;
  if( iotrace==0 ) return;
  va_start(ap, zFormat);
  z = sqlite3_vmprintf(zFormat, ap);
  va_end(ap);
  fprintf(iotrace, "%s", z);
  sqlite3_free(z);
}
#endif


/*
** Determines if a string is a number of not.
*/
static int isNumber(const char *z, int *realnum){
  if( *z=='-' || *z=='+' ) z++;
  if( !IsDigit(*z) ){
    return 0;
  }
  z++;
  if( realnum ) *realnum = 0;
  while( IsDigit(*z) ){ z++; }
  if( *z=='.' ){
    z++;
    if( !IsDigit(*z) ) return 0;
    while( IsDigit(*z) ){ z++; }
    if( realnum ) *realnum = 1;
  }
  if( *z=='e' || *z=='E' ){
    z++;
    if( *z=='+' || *z=='-' ) z++;
    if( !IsDigit(*z) ) return 0;
    while( IsDigit(*z) ){ z++; }
    if( realnum ) *realnum = 1;
  }
  return *z==0;
}

/*
** A global char* and an SQL function to access its current value 
** from within an SQL statement. This program used to use the 
** sqlite_exec_printf() API to substitue a string into an SQL statement.
** The correct way to do this with sqlite3 is to use the bind API, but
** since the shell is built around the callback paradigm it would be a lot
** of work. Instead just use this hack, which is quite harmless.
*/
static const char *zShellStatic = 0;
static void shellstaticFunc(
  sqlite3_context *context,
  int argc,
  sqlite3_value **argv
){
  assert( 0==argc );
  assert( zShellStatic );
  UNUSED_PARAMETER(argc);
  UNUSED_PARAMETER(argv);
  sqlite3_result_text(context, zShellStatic, -1, SQLITE_STATIC);
}


/*
** This routine reads a line of text from FILE in, stores
** the text in memory obtained from malloc() and returns a pointer
** to the text.  NULL is returned at end of file, or if malloc()
** fails.
**
** If zLine is not NULL then it is a malloced buffer returned from
** a previous call to this routine that may be reused.
*/
static char *local_getline(char *zLine, FILE *in){
  int nLine = zLine==0 ? 0 : 100;
  int n = 0;

  while( 1 ){
    if( n+100>nLine ){
      nLine = nLine*2 + 100;
      zLine = realloc(zLine, nLine);
      if( zLine==0 ) return 0;
    }
    if( fgets(&zLine[n], nLine - n, in)==0 ){
      if( n==0 ){
        free(zLine);
        return 0;
      }
      zLine[n] = 0;
      break;
    }
    while( zLine[n] ) n++;
    if( n>0 && zLine[n-1]=='\n' ){
      n--;
      if( n>0 && zLine[n-1]=='\r' ) n--;
      zLine[n] = 0;
      break;
    }
  }
  return zLine;
}

/*
** Retrieve a single line of input text.
**
** If in==0 then read from standard input and prompt before each line.
** If isContinuation is true, then a continuation prompt is appropriate.
** If isContinuation is zero, then the main prompt should be used.
**
** If zPrior is not NULL then it is a buffer from a prior call to this
** routine that can be reused.
**
** The result is stored in space obtained from malloc() and must either
** be freed by the caller or else passed back into this routine via the
** zPrior argument for reuse.
*/
static char *one_input_line(FILE *in, char *zPrior, int isContinuation){
  char *zPrompt;
  char *zResult;
  if( in!=0 ){
    zResult = local_getline(zPrior, in);
  }else{
    zPrompt = isContinuation ? continuePrompt : mainPrompt;
#if SHELL_USE_LOCAL_GETLINE
    printf("%s", zPrompt);
    fflush(stdout);
    zResult = local_getline(zPrior, stdin);
#else
    free(zPrior);
    zResult = shell_readline(zPrompt);
    if( zResult && *zResult ) shell_add_history(zResult);
#endif
  }
  return zResult;
}

/*
** Shell output mode information from before ".explain on", 
** saved so that it can be restored by ".explain off"
*/
typedef struct SavedModeInfo SavedModeInfo;
struct SavedModeInfo {
  int valid;          /* Is there legit data in here? */
  int mode;           /* Mode prior to ".explain on" */
  int showHeader;     /* The ".header" setting prior to ".explain on" */
  int colWidth[100];  /* Column widths prior to ".explain on" */
};

/*
** State information about the database connection is contained in an
** instance of the following structure.
*/
typedef struct ShellState ShellState;
struct ShellState {
  sqlite3 *db;           /* The database */
  int echoOn;            /* True to echo input commands */
  int autoEQP;           /* Run EXPLAIN QUERY PLAN prior to seach SQL stmt */
  int statsOn;           /* True to display memory stats before each finalize */
  int scanstatsOn;       /* True to display scan stats before each finalize */
  int backslashOn;       /* Resolve C-style \x escapes in SQL input text */
  int outCount;          /* Revert to stdout when reaching zero */
  int cnt;               /* Number of records displayed so far */
  FILE *out;             /* Write results here */
  FILE *traceOut;        /* Output for sqlite3_trace() */
  int nErr;              /* Number of errors seen */
  int mode;              /* An output mode setting */
  int writableSchema;    /* True if PRAGMA writable_schema=ON */
  int showHeader;        /* True to show column names in List or Column mode */
  unsigned shellFlgs;    /* Various flags */
  char *zDestTable;      /* Name of destination table when MODE_Insert */
  char colSeparator[20]; /* Column separator character for several modes */
  char rowSeparator[20]; /* Row separator character for MODE_Ascii */
  int colWidth[100];     /* Requested width of each column when in column mode*/
  int actualWidth[100];  /* Actual width of each column */
  char nullValue[20];    /* The text to print when a NULL comes back from
                         ** the database */
  SavedModeInfo normalMode;/* Holds the mode just before .explain ON */
  char outfile[FILENAME_MAX]; /* Filename for *out */
  const char *zDbFilename;    /* name of the database file */
  char *zFreeOnClose;         /* Filename to free when closing */
  const char *zVfs;           /* Name of VFS to use */
  sqlite3_stmt *pStmt;   /* Current statement if any. */
  FILE *pLog;            /* Write log output here */
  int *aiIndent;         /* Array of indents used in MODE_Explain */
  int nIndent;           /* Size of array aiIndent[] */
  int iIndent;           /* Index of current op in aiIndent[] */
};

/*
** These are the allowed shellFlgs values
*/
#define SHFLG_Scratch     0x00001     /* The --scratch option is used */
#define SHFLG_Pagecache   0x00002     /* The --pagecache option is used */
#define SHFLG_Lookaside   0x00004     /* Lookaside memory is used */

/*
** These are the allowed modes.
*/
#define MODE_Line     0  /* One column per line.  Blank line between records */
#define MODE_Column   1  /* One record per line in neat columns */
#define MODE_List     2  /* One record per line with a separator */
#define MODE_Semi     3  /* Same as MODE_List but append ";" to each line */
#define MODE_Html     4  /* Generate an XHTML table */
#define MODE_Insert   5  /* Generate SQL "insert" statements */
#define MODE_Tcl      6  /* Generate ANSI-C or TCL quoted elements */
#define MODE_Csv      7  /* Quote strings, numbers are plain */
#define MODE_Explain  8  /* Like MODE_Column, but do not truncate data */
#define MODE_Ascii    9  /* Use ASCII unit and record separators (0x1F/0x1E) */

static const char *modeDescr[] = {
  "line",
  "column",
  "list",
  "semi",
  "html",
  "insert",
  "tcl",
  "csv",
  "explain",
  "ascii",
};

/*
** These are the column/row/line separators used by the various
** import/export modes.
*/
#define SEP_Column    "|"
#define SEP_Row       "\n"
#define SEP_Tab       "\t"
#define SEP_Space     " "
#define SEP_Comma     ","
#define SEP_CrLf      "\r\n"
#define SEP_Unit      "\x1F"
#define SEP_Record    "\x1E"

/*
** Number of elements in an array
*/
#define ArraySize(X)  (int)(sizeof(X)/sizeof(X[0]))

/*
** Compute a string length that is limited to what can be stored in
** lower 30 bits of a 32-bit signed integer.
*/
static int strlen30(const char *z){
  const char *z2 = z;
  while( *z2 ){ z2++; }
  return 0x3fffffff & (int)(z2 - z);
}

/*
** A callback for the sqlite3_log() interface.
*/
static void shellLog(void *pArg, int iErrCode, const char *zMsg){
  ShellState *p = (ShellState*)pArg;
  if( p->pLog==0 ) return;
  fprintf(p->pLog, "(%d) %s\n", iErrCode, zMsg);
  fflush(p->pLog);
}

/*
** Output the given string as a hex-encoded blob (eg. X'1234' )
*/
static void output_hex_blob(FILE *out, const void *pBlob, int nBlob){
  int i;
  char *zBlob = (char *)pBlob;
  fprintf(out,"X'");
  for(i=0; i<nBlob; i++){ fprintf(out,"%02x",zBlob[i]&0xff); }
  fprintf(out,"'");
}

/*
** Output the given string as a quoted string using SQL quoting conventions.
*/
static void output_quoted_string(FILE *out, const char *z){
  int i;
  int nSingle = 0;
  setBinaryMode(out);
  for(i=0; z[i]; i++){
    if( z[i]=='\'' ) nSingle++;
  }
  if( nSingle==0 ){
    fprintf(out,"'%s'",z);
  }else{
    fprintf(out,"'");
    while( *z ){
      for(i=0; z[i] && z[i]!='\''; i++){}
      if( i==0 ){
        fprintf(out,"''");
        z++;
      }else if( z[i]=='\'' ){
        fprintf(out,"%.*s''",i,z);
        z += i+1;
      }else{
        fprintf(out,"%s",z);
        break;
      }
    }
    fprintf(out,"'");
  }
  setTextMode(out);
}

/*
** Output the given string as a quoted according to C or TCL quoting rules.
*/
static void output_c_string(FILE *out, const char *z){
  unsigned int c;
  fputc('"', out);
  while( (c = *(z++))!=0 ){
    if( c=='\\' ){
      fputc(c, out);
      fputc(c, out);
    }else if( c=='"' ){
      fputc('\\', out);
      fputc('"', out);
    }else if( c=='\t' ){
      fputc('\\', out);
      fputc('t', out);
    }else if( c=='\n' ){
      fputc('\\', out);
      fputc('n', out);
    }else if( c=='\r' ){
      fputc('\\', out);
      fputc('r', out);
    }else if( !isprint(c&0xff) ){
      fprintf(out, "\\%03o", c&0xff);
    }else{
      fputc(c, out);
    }
  }
  fputc('"', out);
}

/*
** Output the given string with characters that are special to
** HTML escaped.
*/
static void output_html_string(FILE *out, const char *z){
  int i;
  if( z==0 ) z = "";
  while( *z ){
    for(i=0;   z[i] 
            && z[i]!='<' 
            && z[i]!='&' 
            && z[i]!='>' 
            && z[i]!='\"' 
            && z[i]!='\'';
        i++){}
    if( i>0 ){
      fprintf(out,"%.*s",i,z);
    }
    if( z[i]=='<' ){
      fprintf(out,"&lt;");
    }else if( z[i]=='&' ){
      fprintf(out,"&amp;");
    }else if( z[i]=='>' ){
      fprintf(out,"&gt;");
    }else if( z[i]=='\"' ){
      fprintf(out,"&quot;");
    }else if( z[i]=='\'' ){
      fprintf(out,"&#39;");
    }else{
      break;
    }
    z += i + 1;
  }
}

/*
** If a field contains any character identified by a 1 in the following
** array, then the string must be quoted for CSV.
*/
static const char needCsvQuote[] = {
  1, 1, 1, 1, 1, 1, 1, 1,   1, 1, 1, 1, 1, 1, 1, 1,   
  1, 1, 1, 1, 1, 1, 1, 1,   1, 1, 1, 1, 1, 1, 1, 1,   
  1, 0, 1, 0, 0, 0, 0, 1,   0, 0, 0, 0, 0, 0, 0, 0, 
  0, 0, 0, 0, 0, 0, 0, 0,   0, 0, 0, 0, 0, 0, 0, 0, 
  0, 0, 0, 0, 0, 0, 0, 0,   0, 0, 0, 0, 0, 0, 0, 0, 
  0, 0, 0, 0, 0, 0, 0, 0,   0, 0, 0, 0, 0, 0, 0, 0, 
  0, 0, 0, 0, 0, 0, 0, 0,   0, 0, 0, 0, 0, 0, 0, 0, 
  0, 0, 0, 0, 0, 0, 0, 0,   0, 0, 0, 0, 0, 0, 0, 1, 
  1, 1, 1, 1, 1, 1, 1, 1,   1, 1, 1, 1, 1, 1, 1, 1,   
  1, 1, 1, 1, 1, 1, 1, 1,   1, 1, 1, 1, 1, 1, 1, 1,   
  1, 1, 1, 1, 1, 1, 1, 1,   1, 1, 1, 1, 1, 1, 1, 1,   
  1, 1, 1, 1, 1, 1, 1, 1,   1, 1, 1, 1, 1, 1, 1, 1,   
  1, 1, 1, 1, 1, 1, 1, 1,   1, 1, 1, 1, 1, 1, 1, 1,   
  1, 1, 1, 1, 1, 1, 1, 1,   1, 1, 1, 1, 1, 1, 1, 1,   
  1, 1, 1, 1, 1, 1, 1, 1,   1, 1, 1, 1, 1, 1, 1, 1,   
  1, 1, 1, 1, 1, 1, 1, 1,   1, 1, 1, 1, 1, 1, 1, 1,   
};

/*
** Output a single term of CSV.  Actually, p->colSeparator is used for
** the separator, which may or may not be a comma.  p->nullValue is
** the null value.  Strings are quoted if necessary.  The separator
** is only issued if bSep is true.
*/
static void output_csv(ShellState *p, const char *z, int bSep){
  FILE *out = p->out;
  if( z==0 ){
    fprintf(out,"%s",p->nullValue);
  }else{
    int i;
    int nSep = strlen30(p->colSeparator);
    for(i=0; z[i]; i++){
      if( needCsvQuote[((unsigned char*)z)[i]] 
         || (z[i]==p->colSeparator[0] && 
             (nSep==1 || memcmp(z, p->colSeparator, nSep)==0)) ){
        i = 0;
        break;
      }
    }
    if( i==0 ){
      putc('"', out);
      for(i=0; z[i]; i++){
        if( z[i]=='"' ) putc('"', out);
        putc(z[i], out);
      }
      putc('"', out);
    }else{
      fprintf(out, "%s", z);
    }
  }
  if( bSep ){
    fprintf(p->out, "%s", p->colSeparator);
  }
}

#ifdef SIGINT
/*
** This routine runs when the user presses Ctrl-C
*/
static void interrupt_handler(int NotUsed){
  UNUSED_PARAMETER(NotUsed);
  seenInterrupt++;
  if( seenInterrupt>2 ) exit(1);
  if( globalDb ) sqlite3_interrupt(globalDb);
}
#endif

/*
** This is the callback routine that the shell
** invokes for each row of a query result.
*/
static int shell_callback(
  void *pArg,
  int nArg,        /* Number of result columns */
  char **azArg,    /* Text of each result column */
  char **azCol,    /* Column names */
  int *aiType      /* Column types */
){
  int i;
  ShellState *p = (ShellState*)pArg;

  switch( p->mode ){
    case MODE_Line: {
      int w = 5;
      if( azArg==0 ) break;
      for(i=0; i<nArg; i++){
        int len = strlen30(azCol[i] ? azCol[i] : "");
        if( len>w ) w = len;
      }
      if( p->cnt++>0 ) fprintf(p->out, "%s", p->rowSeparator);
      for(i=0; i<nArg; i++){
        fprintf(p->out,"%*s = %s%s", w, azCol[i],
                azArg[i] ? azArg[i] : p->nullValue, p->rowSeparator);
      }
      break;
    }
    case MODE_Explain:
    case MODE_Column: {
      if( p->cnt++==0 ){
        for(i=0; i<nArg; i++){
          int w, n;
          if( i<ArraySize(p->colWidth) ){
            w = p->colWidth[i];
          }else{
            w = 0;
          }
          if( w==0 ){
            w = strlen30(azCol[i] ? azCol[i] : "");
            if( w<10 ) w = 10;
            n = strlen30(azArg && azArg[i] ? azArg[i] : p->nullValue);
            if( w<n ) w = n;
          }
          if( i<ArraySize(p->actualWidth) ){
            p->actualWidth[i] = w;
          }
          if( p->showHeader ){
            if( w<0 ){
              fprintf(p->out,"%*.*s%s",-w,-w,azCol[i],
                      i==nArg-1 ? p->rowSeparator : "  ");
            }else{
              fprintf(p->out,"%-*.*s%s",w,w,azCol[i],
                      i==nArg-1 ? p->rowSeparator : "  ");
            }
          }
        }
        if( p->showHeader ){
          for(i=0; i<nArg; i++){
            int w;
            if( i<ArraySize(p->actualWidth) ){
               w = p->actualWidth[i];
               if( w<0 ) w = -w;
            }else{
               w = 10;
            }
            fprintf(p->out,"%-*.*s%s",w,w,"-----------------------------------"
                   "----------------------------------------------------------",
                    i==nArg-1 ? p->rowSeparator : "  ");
          }
        }
      }
      if( azArg==0 ) break;
      for(i=0; i<nArg; i++){
        int w;
        if( i<ArraySize(p->actualWidth) ){
           w = p->actualWidth[i];
        }else{
           w = 10;
        }
        if( p->mode==MODE_Explain && azArg[i] && strlen30(azArg[i])>w ){
          w = strlen30(azArg[i]);
        }
        if( i==1 && p->aiIndent && p->pStmt ){
          if( p->iIndent<p->nIndent ){
            fprintf(p->out, "%*.s", p->aiIndent[p->iIndent], "");
          }
          p->iIndent++;
        }
        if( w<0 ){
          fprintf(p->out,"%*.*s%s",-w,-w,
              azArg[i] ? azArg[i] : p->nullValue,
              i==nArg-1 ? p->rowSeparator : "  ");
        }else{
          fprintf(p->out,"%-*.*s%s",w,w,
              azArg[i] ? azArg[i] : p->nullValue,
              i==nArg-1 ? p->rowSeparator : "  ");
        }
      }
      break;
    }
    case MODE_Semi:
    case MODE_List: {
      if( p->cnt++==0 && p->showHeader ){
        for(i=0; i<nArg; i++){
          fprintf(p->out,"%s%s",azCol[i],
                  i==nArg-1 ? p->rowSeparator : p->colSeparator);
        }
      }
      if( azArg==0 ) break;
      for(i=0; i<nArg; i++){
        char *z = azArg[i];
        if( z==0 ) z = p->nullValue;
        fprintf(p->out, "%s", z);
        if( i<nArg-1 ){
          fprintf(p->out, "%s", p->colSeparator);
        }else if( p->mode==MODE_Semi ){
          fprintf(p->out, ";%s", p->rowSeparator);
        }else{
          fprintf(p->out, "%s", p->rowSeparator);
        }
      }
      break;
    }
    case MODE_Html: {
      if( p->cnt++==0 && p->showHeader ){
        fprintf(p->out,"<TR>");
        for(i=0; i<nArg; i++){
          fprintf(p->out,"<TH>");
          output_html_string(p->out, azCol[i]);
          fprintf(p->out,"</TH>\n");
        }
        fprintf(p->out,"</TR>\n");
      }
      if( azArg==0 ) break;
      fprintf(p->out,"<TR>");
      for(i=0; i<nArg; i++){
        fprintf(p->out,"<TD>");
        output_html_string(p->out, azArg[i] ? azArg[i] : p->nullValue);
        fprintf(p->out,"</TD>\n");
      }
      fprintf(p->out,"</TR>\n");
      break;
    }
    case MODE_Tcl: {
      if( p->cnt++==0 && p->showHeader ){
        for(i=0; i<nArg; i++){
          output_c_string(p->out,azCol[i] ? azCol[i] : "");
          if(i<nArg-1) fprintf(p->out, "%s", p->colSeparator);
        }
        fprintf(p->out, "%s", p->rowSeparator);
      }
      if( azArg==0 ) break;
      for(i=0; i<nArg; i++){
        output_c_string(p->out, azArg[i] ? azArg[i] : p->nullValue);
        if(i<nArg-1) fprintf(p->out, "%s", p->colSeparator);
      }
      fprintf(p->out, "%s", p->rowSeparator);
      break;
    }
    case MODE_Csv: {
      setBinaryMode(p->out);
      if( p->cnt++==0 && p->showHeader ){
        for(i=0; i<nArg; i++){
          output_csv(p, azCol[i] ? azCol[i] : "", i<nArg-1);
        }
        fprintf(p->out, "%s", p->rowSeparator);
      }
      if( nArg>0 ){
        for(i=0; i<nArg; i++){
          output_csv(p, azArg[i], i<nArg-1);
        }
        fprintf(p->out, "%s", p->rowSeparator);
      }
      setTextMode(p->out);
      break;
    }
    case MODE_Insert: {
      p->cnt++;
      if( azArg==0 ) break;
      fprintf(p->out,"INSERT INTO %s",p->zDestTable);
      if( p->showHeader ){
        fprintf(p->out,"(");
        for(i=0; i<nArg; i++){
          char *zSep = i>0 ? ",": "";
          fprintf(p->out, "%s%s", zSep, azCol[i]);
        }
        fprintf(p->out,")");
      }
      fprintf(p->out," VALUES(");
      for(i=0; i<nArg; i++){
        char *zSep = i>0 ? ",": "";
        if( (azArg[i]==0) || (aiType && aiType[i]==SQLITE_NULL) ){
          fprintf(p->out,"%sNULL",zSep);
        }else if( aiType && aiType[i]==SQLITE_TEXT ){
          if( zSep[0] ) fprintf(p->out,"%s",zSep);
          output_quoted_string(p->out, azArg[i]);
        }else if( aiType && (aiType[i]==SQLITE_INTEGER
                             || aiType[i]==SQLITE_FLOAT) ){
          fprintf(p->out,"%s%s",zSep, azArg[i]);
        }else if( aiType && aiType[i]==SQLITE_BLOB && p->pStmt ){
          const void *pBlob = sqlite3_column_blob(p->pStmt, i);
          int nBlob = sqlite3_column_bytes(p->pStmt, i);
          if( zSep[0] ) fprintf(p->out,"%s",zSep);
          output_hex_blob(p->out, pBlob, nBlob);
        }else if( isNumber(azArg[i], 0) ){
          fprintf(p->out,"%s%s",zSep, azArg[i]);
        }else{
          if( zSep[0] ) fprintf(p->out,"%s",zSep);
          output_quoted_string(p->out, azArg[i]);
        }
      }
      fprintf(p->out,");\n");
      break;
    }
    case MODE_Ascii: {
      if( p->cnt++==0 && p->showHeader ){
        for(i=0; i<nArg; i++){
          if( i>0 ) fprintf(p->out, "%s", p->colSeparator);
          fprintf(p->out,"%s",azCol[i] ? azCol[i] : "");
        }
        fprintf(p->out, "%s", p->rowSeparator);
      }
      if( azArg==0 ) break;
      for(i=0; i<nArg; i++){
        if( i>0 ) fprintf(p->out, "%s", p->colSeparator);
        fprintf(p->out,"%s",azArg[i] ? azArg[i] : p->nullValue);
      }
      fprintf(p->out, "%s", p->rowSeparator);
      break;
    }
  }
  return 0;
}

/*
** This is the callback routine that the SQLite library
** invokes for each row of a query result.
*/
static int callback(void *pArg, int nArg, char **azArg, char **azCol){
  /* since we don't have type info, call the shell_callback with a NULL value */
  return shell_callback(pArg, nArg, azArg, azCol, NULL);
}

/*
** Set the destination table field of the ShellState structure to
** the name of the table given.  Escape any quote characters in the
** table name.
*/
static void set_table_name(ShellState *p, const char *zName){
  int i, n;
  int needQuote;
  char *z;

  if( p->zDestTable ){
    free(p->zDestTable);
    p->zDestTable = 0;
  }
  if( zName==0 ) return;
  needQuote = !isalpha((unsigned char)*zName) && *zName!='_';
  for(i=n=0; zName[i]; i++, n++){
    if( !isalnum((unsigned char)zName[i]) && zName[i]!='_' ){
      needQuote = 1;
      if( zName[i]=='\'' ) n++;
    }
  }
  if( needQuote ) n += 2;
  z = p->zDestTable = malloc( n+1 );
  if( z==0 ){
    fprintf(stderr,"Error: out of memory\n");
    exit(1);
  }
  n = 0;
  if( needQuote ) z[n++] = '\'';
  for(i=0; zName[i]; i++){
    z[n++] = zName[i];
    if( zName[i]=='\'' ) z[n++] = '\'';
  }
  if( needQuote ) z[n++] = '\'';
  z[n] = 0;
}

/* zIn is either a pointer to a NULL-terminated string in memory obtained
** from malloc(), or a NULL pointer. The string pointed to by zAppend is
** added to zIn, and the result returned in memory obtained from malloc().
** zIn, if it was not NULL, is freed.
**
** If the third argument, quote, is not '\0', then it is used as a 
** quote character for zAppend.
*/
static char *appendText(char *zIn, char const *zAppend, char quote){
  int len;
  int i;
  int nAppend = strlen30(zAppend);
  int nIn = (zIn?strlen30(zIn):0);

  len = nAppend+nIn+1;
  if( quote ){
    len += 2;
    for(i=0; i<nAppend; i++){
      if( zAppend[i]==quote ) len++;
    }
  }

  zIn = (char *)realloc(zIn, len);
  if( !zIn ){
    return 0;
  }

  if( quote ){
    char *zCsr = &zIn[nIn];
    *zCsr++ = quote;
    for(i=0; i<nAppend; i++){
      *zCsr++ = zAppend[i];
      if( zAppend[i]==quote ) *zCsr++ = quote;
    }
    *zCsr++ = quote;
    *zCsr++ = '\0';
    assert( (zCsr-zIn)==len );
  }else{
    memcpy(&zIn[nIn], zAppend, nAppend);
    zIn[len-1] = '\0';
  }

  return zIn;
}


/*
** Execute a query statement that will generate SQL output.  Print
** the result columns, comma-separated, on a line and then add a
** semicolon terminator to the end of that line.
**
** If the number of columns is 1 and that column contains text "--"
** then write the semicolon on a separate line.  That way, if a 
** "--" comment occurs at the end of the statement, the comment
** won't consume the semicolon terminator.
*/
static int run_table_dump_query(
  ShellState *p,           /* Query context */
  const char *zSelect,     /* SELECT statement to extract content */
  const char *zFirstRow    /* Print before first row, if not NULL */
){
  sqlite3_stmt *pSelect;
  int rc;
  int nResult;
  int i;
  const char *z;
  rc = sqlite3_prepare_v2(p->db, zSelect, -1, &pSelect, 0);
  if( rc!=SQLITE_OK || !pSelect ){
    fprintf(p->out, "/**** ERROR: (%d) %s *****/\n", rc, sqlite3_errmsg(p->db));
    if( (rc&0xff)!=SQLITE_CORRUPT ) p->nErr++;
    return rc;
  }
  rc = sqlite3_step(pSelect);
  nResult = sqlite3_column_count(pSelect);
  while( rc==SQLITE_ROW ){
    if( zFirstRow ){
      fprintf(p->out, "%s", zFirstRow);
      zFirstRow = 0;
    }
    z = (const char*)sqlite3_column_text(pSelect, 0);
    fprintf(p->out, "%s", z);
    for(i=1; i<nResult; i++){ 
      fprintf(p->out, ",%s", sqlite3_column_text(pSelect, i));
    }
    if( z==0 ) z = "";
    while( z[0] && (z[0]!='-' || z[1]!='-') ) z++;
    if( z[0] ){
      fprintf(p->out, "\n;\n");
    }else{
      fprintf(p->out, ";\n");
    }    
    rc = sqlite3_step(pSelect);
  }
  rc = sqlite3_finalize(pSelect);
  if( rc!=SQLITE_OK ){
    fprintf(p->out, "/**** ERROR: (%d) %s *****/\n", rc, sqlite3_errmsg(p->db));
    if( (rc&0xff)!=SQLITE_CORRUPT ) p->nErr++;
  }
  return rc;
}

/*
** Allocate space and save off current error string.
*/
static char *save_err_msg(
  sqlite3 *db            /* Database to query */
){
  int nErrMsg = 1+strlen30(sqlite3_errmsg(db));
  char *zErrMsg = sqlite3_malloc64(nErrMsg);
  if( zErrMsg ){
    memcpy(zErrMsg, sqlite3_errmsg(db), nErrMsg);
  }
  return zErrMsg;
}

/*
** Display memory stats.
*/
static int display_stats(
  sqlite3 *db,                /* Database to query */
  ShellState *pArg,           /* Pointer to ShellState */
  int bReset                  /* True to reset the stats */
){
  int iCur;
  int iHiwtr;

  if( pArg && pArg->out ){
    
    iHiwtr = iCur = -1;
    sqlite3_status(SQLITE_STATUS_MEMORY_USED, &iCur, &iHiwtr, bReset);
    fprintf(pArg->out,
            "Memory Used:                         %d (max %d) bytes\n",
            iCur, iHiwtr);
    iHiwtr = iCur = -1;
    sqlite3_status(SQLITE_STATUS_MALLOC_COUNT, &iCur, &iHiwtr, bReset);
    fprintf(pArg->out, "Number of Outstanding Allocations:   %d (max %d)\n",
            iCur, iHiwtr);
    if( pArg->shellFlgs & SHFLG_Pagecache ){
      iHiwtr = iCur = -1;
      sqlite3_status(SQLITE_STATUS_PAGECACHE_USED, &iCur, &iHiwtr, bReset);
      fprintf(pArg->out,
              "Number of Pcache Pages Used:         %d (max %d) pages\n",
              iCur, iHiwtr);
    }
    iHiwtr = iCur = -1;
    sqlite3_status(SQLITE_STATUS_PAGECACHE_OVERFLOW, &iCur, &iHiwtr, bReset);
    fprintf(pArg->out,
            "Number of Pcache Overflow Bytes:     %d (max %d) bytes\n",
            iCur, iHiwtr);
    if( pArg->shellFlgs & SHFLG_Scratch ){
      iHiwtr = iCur = -1;
      sqlite3_status(SQLITE_STATUS_SCRATCH_USED, &iCur, &iHiwtr, bReset);
      fprintf(pArg->out, "Number of Scratch Allocations Used:  %d (max %d)\n",
              iCur, iHiwtr);
    }
    iHiwtr = iCur = -1;
    sqlite3_status(SQLITE_STATUS_SCRATCH_OVERFLOW, &iCur, &iHiwtr, bReset);
    fprintf(pArg->out,
            "Number of Scratch Overflow Bytes:    %d (max %d) bytes\n",
            iCur, iHiwtr);
    iHiwtr = iCur = -1;
    sqlite3_status(SQLITE_STATUS_MALLOC_SIZE, &iCur, &iHiwtr, bReset);
    fprintf(pArg->out, "Largest Allocation:                  %d bytes\n",
            iHiwtr);
    iHiwtr = iCur = -1;
    sqlite3_status(SQLITE_STATUS_PAGECACHE_SIZE, &iCur, &iHiwtr, bReset);
    fprintf(pArg->out, "Largest Pcache Allocation:           %d bytes\n",
            iHiwtr);
    iHiwtr = iCur = -1;
    sqlite3_status(SQLITE_STATUS_SCRATCH_SIZE, &iCur, &iHiwtr, bReset);
    fprintf(pArg->out, "Largest Scratch Allocation:          %d bytes\n",
            iHiwtr);
#ifdef YYTRACKMAXSTACKDEPTH
    iHiwtr = iCur = -1;
    sqlite3_status(SQLITE_STATUS_PARSER_STACK, &iCur, &iHiwtr, bReset);
    fprintf(pArg->out, "Deepest Parser Stack:                %d (max %d)\n",
            iCur, iHiwtr);
#endif
  }

  if( pArg && pArg->out && db ){
    if( pArg->shellFlgs & SHFLG_Lookaside ){
      iHiwtr = iCur = -1;
      sqlite3_db_status(db, SQLITE_DBSTATUS_LOOKASIDE_USED,
                        &iCur, &iHiwtr, bReset);
      fprintf(pArg->out, "Lookaside Slots Used:                %d (max %d)\n",
              iCur, iHiwtr);
      sqlite3_db_status(db, SQLITE_DBSTATUS_LOOKASIDE_HIT,
                        &iCur, &iHiwtr, bReset);
      fprintf(pArg->out, "Successful lookaside attempts:       %d\n", iHiwtr);
      sqlite3_db_status(db, SQLITE_DBSTATUS_LOOKASIDE_MISS_SIZE,
                        &iCur, &iHiwtr, bReset);
      fprintf(pArg->out, "Lookaside failures due to size:      %d\n", iHiwtr);
      sqlite3_db_status(db, SQLITE_DBSTATUS_LOOKASIDE_MISS_FULL,
                        &iCur, &iHiwtr, bReset);
      fprintf(pArg->out, "Lookaside failures due to OOM:       %d\n", iHiwtr);
    }
    iHiwtr = iCur = -1;
    sqlite3_db_status(db, SQLITE_DBSTATUS_CACHE_USED, &iCur, &iHiwtr, bReset);
    fprintf(pArg->out, "Pager Heap Usage:                    %d bytes\n",iCur);
    iHiwtr = iCur = -1;
    sqlite3_db_status(db, SQLITE_DBSTATUS_CACHE_HIT, &iCur, &iHiwtr, 1);
    fprintf(pArg->out, "Page cache hits:                     %d\n", iCur);
    iHiwtr = iCur = -1;
    sqlite3_db_status(db, SQLITE_DBSTATUS_CACHE_MISS, &iCur, &iHiwtr, 1);
    fprintf(pArg->out, "Page cache misses:                   %d\n", iCur); 
    iHiwtr = iCur = -1;
    sqlite3_db_status(db, SQLITE_DBSTATUS_CACHE_WRITE, &iCur, &iHiwtr, 1);
    fprintf(pArg->out, "Page cache writes:                   %d\n", iCur); 
    iHiwtr = iCur = -1;
    sqlite3_db_status(db, SQLITE_DBSTATUS_SCHEMA_USED, &iCur, &iHiwtr, bReset);
    fprintf(pArg->out, "Schema Heap Usage:                   %d bytes\n",iCur); 
    iHiwtr = iCur = -1;
    sqlite3_db_status(db, SQLITE_DBSTATUS_STMT_USED, &iCur, &iHiwtr, bReset);
    fprintf(pArg->out, "Statement Heap/Lookaside Usage:      %d bytes\n",iCur); 
  }

  if( pArg && pArg->out && db && pArg->pStmt ){
    iCur = sqlite3_stmt_status(pArg->pStmt, SQLITE_STMTSTATUS_FULLSCAN_STEP,
                               bReset);
    fprintf(pArg->out, "Fullscan Steps:                      %d\n", iCur);
    iCur = sqlite3_stmt_status(pArg->pStmt, SQLITE_STMTSTATUS_SORT, bReset);
    fprintf(pArg->out, "Sort Operations:                     %d\n", iCur);
    iCur = sqlite3_stmt_status(pArg->pStmt, SQLITE_STMTSTATUS_AUTOINDEX,bReset);
    fprintf(pArg->out, "Autoindex Inserts:                   %d\n", iCur);
    iCur = sqlite3_stmt_status(pArg->pStmt, SQLITE_STMTSTATUS_VM_STEP, bReset);
    fprintf(pArg->out, "Virtual Machine Steps:               %d\n", iCur);
  }

  return 0;
}

/*
** Display scan stats.
*/
static void display_scanstats(
  sqlite3 *db,                    /* Database to query */
  ShellState *pArg                /* Pointer to ShellState */
){
#ifdef SQLITE_ENABLE_STMT_SCANSTATUS
  int i, k, n, mx;
  fprintf(pArg->out, "-------- scanstats --------\n");
  mx = 0;
  for(k=0; k<=mx; k++){
    double rEstLoop = 1.0;
    for(i=n=0; 1; i++){
      sqlite3_stmt *p = pArg->pStmt;
      sqlite3_int64 nLoop, nVisit;
      double rEst;
      int iSid;
      const char *zExplain;
      if( sqlite3_stmt_scanstatus(p, i, SQLITE_SCANSTAT_NLOOP, (void*)&nLoop) ){
        break;
      }
      sqlite3_stmt_scanstatus(p, i, SQLITE_SCANSTAT_SELECTID, (void*)&iSid);
      if( iSid>mx ) mx = iSid;
      if( iSid!=k ) continue;
      if( n==0 ){
        rEstLoop = (double)nLoop;
        if( k>0 ) fprintf(pArg->out, "-------- subquery %d -------\n", k);
      }
      n++;
      sqlite3_stmt_scanstatus(p, i, SQLITE_SCANSTAT_NVISIT, (void*)&nVisit);
      sqlite3_stmt_scanstatus(p, i, SQLITE_SCANSTAT_EST, (void*)&rEst);
      sqlite3_stmt_scanstatus(p, i, SQLITE_SCANSTAT_EXPLAIN, (void*)&zExplain);
      fprintf(pArg->out, "Loop %2d: %s\n", n, zExplain);
      rEstLoop *= rEst;
      fprintf(pArg->out, 
          "         nLoop=%-8lld nRow=%-8lld estRow=%-8lld estRow/Loop=%-8g\n",
          nLoop, nVisit, (sqlite3_int64)(rEstLoop+0.5), rEst
      );
    }
  }
  fprintf(pArg->out, "---------------------------\n");
#endif
}

/*
** Parameter azArray points to a zero-terminated array of strings. zStr
** points to a single nul-terminated string. Return non-zero if zStr
** is equal, according to strcmp(), to any of the strings in the array.
** Otherwise, return zero.
*/
static int str_in_array(const char *zStr, const char **azArray){
  int i;
  for(i=0; azArray[i]; i++){
    if( 0==strcmp(zStr, azArray[i]) ) return 1;
  }
  return 0;
}

/*
** If compiled statement pSql appears to be an EXPLAIN statement, allocate
** and populate the ShellState.aiIndent[] array with the number of
** spaces each opcode should be indented before it is output. 
**
** The indenting rules are:
**
**     * For each "Next", "Prev", "VNext" or "VPrev" instruction, indent
**       all opcodes that occur between the p2 jump destination and the opcode
**       itself by 2 spaces.
**
**     * For each "Goto", if the jump destination is earlier in the program
**       and ends on one of:
**          Yield  SeekGt  SeekLt  RowSetRead  Rewind
**       or if the P1 parameter is one instead of zero,
**       then indent all opcodes between the earlier instruction
**       and "Goto" by 2 spaces.
*/
static void explain_data_prepare(ShellState *p, sqlite3_stmt *pSql){
  const char *zSql;               /* The text of the SQL statement */
  const char *z;                  /* Used to check if this is an EXPLAIN */
  int *abYield = 0;               /* True if op is an OP_Yield */
  int nAlloc = 0;                 /* Allocated size of p->aiIndent[], abYield */
  int iOp;                        /* Index of operation in p->aiIndent[] */

  const char *azNext[] = { "Next", "Prev", "VPrev", "VNext", "SorterNext",
                           "NextIfOpen", "PrevIfOpen", 0 };
  const char *azYield[] = { "Yield", "SeekLT", "SeekGT", "RowSetRead",
                            "Rewind", 0 };
  const char *azGoto[] = { "Goto", 0 };

  /* Try to figure out if this is really an EXPLAIN statement. If this
  ** cannot be verified, return early.  */
  zSql = sqlite3_sql(pSql);
  if( zSql==0 ) return;
  for(z=zSql; *z==' ' || *z=='\t' || *z=='\n' || *z=='\f' || *z=='\r'; z++);
  if( sqlite3_strnicmp(z, "explain", 7) ) return;

  for(iOp=0; SQLITE_ROW==sqlite3_step(pSql); iOp++){
    int i;
    int iAddr = sqlite3_column_int(pSql, 0);
    const char *zOp = (const char*)sqlite3_column_text(pSql, 1);

    /* Set p2 to the P2 field of the current opcode. Then, assuming that
    ** p2 is an instruction address, set variable p2op to the index of that
    ** instruction in the aiIndent[] array. p2 and p2op may be different if
    ** the current instruction is part of a sub-program generated by an
    ** SQL trigger or foreign key.  */
    int p2 = sqlite3_column_int(pSql, 3);
    int p2op = (p2 + (iOp-iAddr));

    /* Grow the p->aiIndent array as required */
    if( iOp>=nAlloc ){
      nAlloc += 100;
      p->aiIndent = (int*)sqlite3_realloc64(p->aiIndent, nAlloc*sizeof(int));
      abYield = (int*)sqlite3_realloc64(abYield, nAlloc*sizeof(int));
    }
    abYield[iOp] = str_in_array(zOp, azYield);
    p->aiIndent[iOp] = 0;
    p->nIndent = iOp+1;

    if( str_in_array(zOp, azNext) ){
      for(i=p2op; i<iOp; i++) p->aiIndent[i] += 2;
    }
    if( str_in_array(zOp, azGoto) && p2op<p->nIndent
     && (abYield[p2op] || sqlite3_column_int(pSql, 2))
    ){
      for(i=p2op+1; i<iOp; i++) p->aiIndent[i] += 2;
    }
  }

  p->iIndent = 0;
  sqlite3_free(abYield);
  sqlite3_reset(pSql);
}

/*
** Free the array allocated by explain_data_prepare().
*/
static void explain_data_delete(ShellState *p){
  sqlite3_free(p->aiIndent);
  p->aiIndent = 0;
  p->nIndent = 0;
  p->iIndent = 0;
}

/*
** Execute a statement or set of statements.  Print 
** any result rows/columns depending on the current mode 
** set via the supplied callback.
**
** This is very similar to SQLite's built-in sqlite3_exec() 
** function except it takes a slightly different callback 
** and callback data argument.
*/
static int shell_exec(
  sqlite3 *db,                              /* An open database */
  const char *zSql,                         /* SQL to be evaluated */
  int (*xCallback)(void*,int,char**,char**,int*),   /* Callback function */
                                            /* (not the same as sqlite3_exec) */
  ShellState *pArg,                         /* Pointer to ShellState */
  char **pzErrMsg                           /* Error msg written here */
){
  sqlite3_stmt *pStmt = NULL;     /* Statement to execute. */
  int rc = SQLITE_OK;             /* Return Code */
  int rc2;
  const char *zLeftover;          /* Tail of unprocessed SQL */

  if( pzErrMsg ){
    *pzErrMsg = NULL;
  }

  while( zSql[0] && (SQLITE_OK == rc) ){
    rc = sqlite3_prepare_v2(db, zSql, -1, &pStmt, &zLeftover);
    if( SQLITE_OK != rc ){
      if( pzErrMsg ){
        *pzErrMsg = save_err_msg(db);
      }
    }else{
      if( !pStmt ){
        /* this happens for a comment or white-space */
        zSql = zLeftover;
        while( IsSpace(zSql[0]) ) zSql++;
        continue;
      }

      /* save off the prepared statment handle and reset row count */
      if( pArg ){
        pArg->pStmt = pStmt;
        pArg->cnt = 0;
      }

      /* echo the sql statement if echo on */
      if( pArg && pArg->echoOn ){
        const char *zStmtSql = sqlite3_sql(pStmt);
        fprintf(pArg->out, "%s\n", zStmtSql ? zStmtSql : zSql);
      }

      /* Show the EXPLAIN QUERY PLAN if .eqp is on */
      if( pArg && pArg->autoEQP ){
        sqlite3_stmt *pExplain;
        char *zEQP = sqlite3_mprintf("EXPLAIN QUERY PLAN %s",
                                     sqlite3_sql(pStmt));
        rc = sqlite3_prepare_v2(db, zEQP, -1, &pExplain, 0);
        if( rc==SQLITE_OK ){
          while( sqlite3_step(pExplain)==SQLITE_ROW ){
            fprintf(pArg->out,"--EQP-- %d,", sqlite3_column_int(pExplain, 0));
            fprintf(pArg->out,"%d,", sqlite3_column_int(pExplain, 1));
            fprintf(pArg->out,"%d,", sqlite3_column_int(pExplain, 2));
            fprintf(pArg->out,"%s\n", sqlite3_column_text(pExplain, 3));
          }
        }
        sqlite3_finalize(pExplain);
        sqlite3_free(zEQP);
      }

      /* If the shell is currently in ".explain" mode, gather the extra
      ** data required to add indents to the output.*/
      if( pArg && pArg->mode==MODE_Explain ){
        explain_data_prepare(pArg, pStmt);
      }

      /* perform the first step.  this will tell us if we
      ** have a result set or not and how wide it is.
      */
      rc = sqlite3_step(pStmt);
      /* if we have a result set... */
      if( SQLITE_ROW == rc ){
        /* if we have a callback... */
        if( xCallback ){
          /* allocate space for col name ptr, value ptr, and type */
          int nCol = sqlite3_column_count(pStmt);
          void *pData = sqlite3_malloc64(3*nCol*sizeof(const char*) + 1);
          if( !pData ){
            rc = SQLITE_NOMEM;
          }else{
            char **azCols = (char **)pData;      /* Names of result columns */
            char **azVals = &azCols[nCol];       /* Results */
            int *aiTypes = (int *)&azVals[nCol]; /* Result types */
            int i, x;
            assert(sizeof(int) <= sizeof(char *)); 
            /* save off ptrs to column names */
            for(i=0; i<nCol; i++){
              azCols[i] = (char *)sqlite3_column_name(pStmt, i);
            }
            do{
              /* extract the data and data types */
              for(i=0; i<nCol; i++){
                aiTypes[i] = x = sqlite3_column_type(pStmt, i);
                if( x==SQLITE_BLOB && pArg && pArg->mode==MODE_Insert ){
                  azVals[i] = "";
                }else{
                  azVals[i] = (char*)sqlite3_column_text(pStmt, i);
                }
                if( !azVals[i] && (aiTypes[i]!=SQLITE_NULL) ){
                  rc = SQLITE_NOMEM;
                  break; /* from for */
                }
              } /* end for */

              /* if data and types extracted successfully... */
              if( SQLITE_ROW == rc ){ 
                /* call the supplied callback with the result row data */
                if( xCallback(pArg, nCol, azVals, azCols, aiTypes) ){
                  rc = SQLITE_ABORT;
                }else{
                  rc = sqlite3_step(pStmt);
                }
              }
            } while( SQLITE_ROW == rc );
            sqlite3_free(pData);
          }
        }else{
          do{
            rc = sqlite3_step(pStmt);
          } while( rc == SQLITE_ROW );
        }
      }

      explain_data_delete(pArg);

      /* print usage stats if stats on */
      if( pArg && pArg->statsOn ){
        display_stats(db, pArg, 0);
      }

      /* print loop-counters if required */
      if( pArg && pArg->scanstatsOn ){
        display_scanstats(db, pArg);
      }

      /* Finalize the statement just executed. If this fails, save a 
      ** copy of the error message. Otherwise, set zSql to point to the
      ** next statement to execute. */
      rc2 = sqlite3_finalize(pStmt);
      if( rc!=SQLITE_NOMEM ) rc = rc2;
      if( rc==SQLITE_OK ){
        zSql = zLeftover;
        while( IsSpace(zSql[0]) ) zSql++;
      }else if( pzErrMsg ){
        *pzErrMsg = save_err_msg(db);
      }

      /* clear saved stmt handle */
      if( pArg ){
        pArg->pStmt = NULL;
      }
    }
  } /* end while */

  return rc;
}


/*
** This is a different callback routine used for dumping the database.
** Each row received by this callback consists of a table name,
** the table type ("index" or "table") and SQL to create the table.
** This routine should print text sufficient to recreate the table.
*/
static int dump_callback(void *pArg, int nArg, char **azArg, char **azCol){
  int rc;
  const char *zTable;
  const char *zType;
  const char *zSql;
  const char *zPrepStmt = 0;
  ShellState *p = (ShellState *)pArg;

  UNUSED_PARAMETER(azCol);
  if( nArg!=3 ) return 1;
  zTable = azArg[0];
  zType = azArg[1];
  zSql = azArg[2];
  
  if( strcmp(zTable, "sqlite_sequence")==0 ){
    zPrepStmt = "DELETE FROM sqlite_sequence;\n";
  }else if( sqlite3_strglob("sqlite_stat?", zTable)==0 ){
    fprintf(p->out, "ANALYZE sqlite_master;\n");
  }else if( strncmp(zTable, "sqlite_", 7)==0 ){
    return 0;
  }else if( strncmp(zSql, "CREATE VIRTUAL TABLE", 20)==0 ){
    char *zIns;
    if( !p->writableSchema ){
      fprintf(p->out, "PRAGMA writable_schema=ON;\n");
      p->writableSchema = 1;
    }
    zIns = sqlite3_mprintf(
       "INSERT INTO sqlite_master(type,name,tbl_name,rootpage,sql)"
       "VALUES('table','%q','%q',0,'%q');",
       zTable, zTable, zSql);
    fprintf(p->out, "%s\n", zIns);
    sqlite3_free(zIns);
    return 0;
  }else{
    fprintf(p->out, "%s;\n", zSql);
  }

  if( strcmp(zType, "table")==0 ){
    sqlite3_stmt *pTableInfo = 0;
    char *zSelect = 0;
    char *zTableInfo = 0;
    char *zTmp = 0;
    int nRow = 0;
   
    zTableInfo = appendText(zTableInfo, "PRAGMA table_info(", 0);
    zTableInfo = appendText(zTableInfo, zTable, '"');
    zTableInfo = appendText(zTableInfo, ");", 0);

    rc = sqlite3_prepare_v2(p->db, zTableInfo, -1, &pTableInfo, 0);
    free(zTableInfo);
    if( rc!=SQLITE_OK || !pTableInfo ){
      return 1;
    }

    zSelect = appendText(zSelect, "SELECT 'INSERT INTO ' || ", 0);
    /* Always quote the table name, even if it appears to be pure ascii,
    ** in case it is a keyword. Ex:  INSERT INTO "table" ... */
    zTmp = appendText(zTmp, zTable, '"');
    if( zTmp ){
      zSelect = appendText(zSelect, zTmp, '\'');
      free(zTmp);
    }
    zSelect = appendText(zSelect, " || ' VALUES(' || ", 0);
    rc = sqlite3_step(pTableInfo);
    while( rc==SQLITE_ROW ){
      const char *zText = (const char *)sqlite3_column_text(pTableInfo, 1);
      zSelect = appendText(zSelect, "quote(", 0);
      zSelect = appendText(zSelect, zText, '"');
      rc = sqlite3_step(pTableInfo);
      if( rc==SQLITE_ROW ){
        zSelect = appendText(zSelect, "), ", 0);
      }else{
        zSelect = appendText(zSelect, ") ", 0);
      }
      nRow++;
    }
    rc = sqlite3_finalize(pTableInfo);
    if( rc!=SQLITE_OK || nRow==0 ){
      free(zSelect);
      return 1;
    }
    zSelect = appendText(zSelect, "|| ')' FROM  ", 0);
    zSelect = appendText(zSelect, zTable, '"');

    rc = run_table_dump_query(p, zSelect, zPrepStmt);
    if( rc==SQLITE_CORRUPT ){
      zSelect = appendText(zSelect, " ORDER BY rowid DESC", 0);
      run_table_dump_query(p, zSelect, 0);
    }
    free(zSelect);
  }
  return 0;
}

/*
** Run zQuery.  Use dump_callback() as the callback routine so that
** the contents of the query are output as SQL statements.
**
** If we get a SQLITE_CORRUPT error, rerun the query after appending
** "ORDER BY rowid DESC" to the end.
*/
static int run_schema_dump_query(
  ShellState *p, 
  const char *zQuery
){
  int rc;
  char *zErr = 0;
  rc = sqlite3_exec(p->db, zQuery, dump_callback, p, &zErr);
  if( rc==SQLITE_CORRUPT ){
    char *zQ2;
    int len = strlen30(zQuery);
    fprintf(p->out, "/****** CORRUPTION ERROR *******/\n");
    if( zErr ){
      fprintf(p->out, "/****** %s ******/\n", zErr);
      sqlite3_free(zErr);
      zErr = 0;
    }
    zQ2 = malloc( len+100 );
    if( zQ2==0 ) return rc;
    sqlite3_snprintf(len+100, zQ2, "%s ORDER BY rowid DESC", zQuery);
    rc = sqlite3_exec(p->db, zQ2, dump_callback, p, &zErr);
    if( rc ){
      fprintf(p->out, "/****** ERROR: %s ******/\n", zErr);
    }else{
      rc = SQLITE_CORRUPT;
    }
    sqlite3_free(zErr);
    free(zQ2);
  }
  return rc;
}

/*
** Text of a help message
*/
static char zHelp[] =
  ".backup ?DB? FILE      Backup DB (default \"main\") to FILE\n"
  ".bail on|off           Stop after hitting an error.  Default OFF\n"
  ".binary on|off         Turn binary output on or off.  Default OFF\n"
  ".clone NEWDB           Clone data into NEWDB from the existing database\n"
  ".databases             List names and files of attached databases\n"
  ".dbinfo ?DB?           Show status information about the database\n"
  ".dump ?TABLE? ...      Dump the database in an SQL text format\n"
  "                         If TABLE specified, only dump tables matching\n"
  "                         LIKE pattern TABLE.\n"
  ".echo on|off           Turn command echo on or off\n"
  ".eqp on|off            Enable or disable automatic EXPLAIN QUERY PLAN\n"
  ".exit                  Exit this program\n"
  ".explain ?on|off?      Turn output mode suitable for EXPLAIN on or off.\n"
  "                         With no args, it turns EXPLAIN on.\n"
  ".fullschema            Show schema and the content of sqlite_stat tables\n"
  ".headers on|off        Turn display of headers on or off\n"
  ".help                  Show this message\n"
  ".import FILE TABLE     Import data from FILE into TABLE\n"
  ".indexes ?TABLE?       Show names of all indexes\n"
  "                         If TABLE specified, only show indexes for tables\n"
  "                         matching LIKE pattern TABLE.\n"
#ifdef SQLITE_ENABLE_IOTRACE
  ".iotrace FILE          Enable I/O diagnostic logging to FILE\n"
#endif
  ".limit ?LIMIT? ?VAL?   Display or change the value of an SQLITE_LIMIT\n"
#ifndef SQLITE_OMIT_LOAD_EXTENSION
  ".load FILE ?ENTRY?     Load an extension library\n"
#endif
  ".log FILE|off          Turn logging on or off.  FILE can be stderr/stdout\n"
  ".mode MODE ?TABLE?     Set output mode where MODE is one of:\n"
  "                         ascii    Columns/rows delimited by 0x1F and 0x1E\n"
  "                         csv      Comma-separated values\n"
  "                         column   Left-aligned columns.  (See .width)\n"
  "                         html     HTML <table> code\n"
  "                         insert   SQL insert statements for TABLE\n"
  "                         line     One value per line\n"
  "                         list     Values delimited by .separator strings\n"
  "                         tabs     Tab-separated values\n"
  "                         tcl      TCL list elements\n"
  ".nullvalue STRING      Use STRING in place of NULL values\n"
  ".once FILENAME         Output for the next SQL command only to FILENAME\n"
  ".open ?FILENAME?       Close existing database and reopen FILENAME\n"
  ".output ?FILENAME?     Send output to FILENAME or stdout\n"
  ".print STRING...       Print literal STRING\n"
  ".prompt MAIN CONTINUE  Replace the standard prompts\n"
  ".quit                  Exit this program\n"
  ".read FILENAME         Execute SQL in FILENAME\n"
  ".restore ?DB? FILE     Restore content of DB (default \"main\") from FILE\n"
  ".save FILE             Write in-memory database into FILE\n"
  ".scanstats on|off      Turn sqlite3_stmt_scanstatus() metrics on or off\n"
  ".schema ?TABLE?        Show the CREATE statements\n"
  "                         If TABLE specified, only show tables matching\n"
  "                         LIKE pattern TABLE.\n"
  ".separator COL ?ROW?   Change the column separator and optionally the row\n"
  "                         separator for both the output mode and .import\n"
  ".shell CMD ARGS...     Run CMD ARGS... in a system shell\n"
  ".show                  Show the current values for various settings\n"
  ".stats on|off          Turn stats on or off\n"
  ".system CMD ARGS...    Run CMD ARGS... in a system shell\n"
  ".tables ?TABLE?        List names of tables\n"
  "                         If TABLE specified, only list tables matching\n"
  "                         LIKE pattern TABLE.\n"
  ".timeout MS            Try opening locked tables for MS milliseconds\n"
  ".timer on|off          Turn SQL timer on or off\n"
  ".trace FILE|off        Output each SQL statement as it is run\n"
  ".vfsname ?AUX?         Print the name of the VFS stack\n"
  ".width NUM1 NUM2 ...   Set column widths for \"column\" mode\n"
  "                         Negative values right-justify\n"
;

/* Forward reference */
static int process_input(ShellState *p, FILE *in);
/*
** Implementation of the "readfile(X)" SQL function.  The entire content
** of the file named X is read and returned as a BLOB.  NULL is returned
** if the file does not exist or is unreadable.
*/
static void readfileFunc(
  sqlite3_context *context,
  int argc,
  sqlite3_value **argv
){
  const char *zName;
  FILE *in;
  long nIn;
  void *pBuf;

  zName = (const char*)sqlite3_value_text(argv[0]);
  if( zName==0 ) return;
  in = fopen(zName, "rb");
  if( in==0 ) return;
  fseek(in, 0, SEEK_END);
  nIn = ftell(in);
  rewind(in);
  pBuf = sqlite3_malloc64( nIn );
  if( pBuf && 1==fread(pBuf, nIn, 1, in) ){
    sqlite3_result_blob(context, pBuf, nIn, sqlite3_free);
  }else{
    sqlite3_free(pBuf);
  }
  fclose(in);
}

/*
** Implementation of the "writefile(X,Y)" SQL function.  The argument Y
** is written into file X.  The number of bytes written is returned.  Or
** NULL is returned if something goes wrong, such as being unable to open
** file X for writing.
*/
static void writefileFunc(
  sqlite3_context *context,
  int argc,
  sqlite3_value **argv
){
  FILE *out;
  const char *z;
  sqlite3_int64 rc;
  const char *zFile;

  zFile = (const char*)sqlite3_value_text(argv[0]);
  if( zFile==0 ) return;
  out = fopen(zFile, "wb");
  if( out==0 ) return;
  z = (const char*)sqlite3_value_blob(argv[1]);
  if( z==0 ){
    rc = 0;
  }else{
    rc = fwrite(z, 1, sqlite3_value_bytes(argv[1]), out);
  }
  fclose(out);
  sqlite3_result_int64(context, rc);
}

/*
** Make sure the database is open.  If it is not, then open it.  If
** the database fails to open, print an error message and exit.
*/
static void open_db(ShellState *p, int keepAlive){
  if( p->db==0 ){
    sqlite3_initialize();
    sqlite3_open(p->zDbFilename, &p->db);
    globalDb = p->db;
    if( p->db && sqlite3_errcode(p->db)==SQLITE_OK ){
      sqlite3_create_function(p->db, "shellstatic", 0, SQLITE_UTF8, 0,
          shellstaticFunc, 0, 0);
    }
    if( p->db==0 || SQLITE_OK!=sqlite3_errcode(p->db) ){
      fprintf(stderr,"Error: unable to open database \"%s\": %s\n", 
          p->zDbFilename, sqlite3_errmsg(p->db));
      if( keepAlive ) return;
      exit(1);
    }
#ifndef SQLITE_OMIT_LOAD_EXTENSION
    sqlite3_enable_load_extension(p->db, 1);
#endif
    sqlite3_create_function(p->db, "readfile", 1, SQLITE_UTF8, 0,
                            readfileFunc, 0, 0);
    sqlite3_create_function(p->db, "writefile", 2, SQLITE_UTF8, 0,
                            writefileFunc, 0, 0);
  }
}

/*
** Do C-language style dequoting.
**
**    \a    -> alarm
**    \b    -> backspace
**    \t    -> tab
**    \n    -> newline
**    \v    -> vertical tab
**    \f    -> form feed
**    \r    -> carriage return
**    \s    -> space
**    \"    -> "
**    \'    -> '
**    \\    -> backslash
**    \NNN  -> ascii character NNN in octal
*/
static void resolve_backslashes(char *z){
  int i, j;
  char c;
  while( *z && *z!='\\' ) z++;
  for(i=j=0; (c = z[i])!=0; i++, j++){
    if( c=='\\' && z[i+1]!=0 ){
      c = z[++i];
      if( c=='a' ){
        c = '\a';
      }else if( c=='b' ){
        c = '\b';
      }else if( c=='t' ){
        c = '\t';
      }else if( c=='n' ){
        c = '\n';
      }else if( c=='v' ){
        c = '\v';
      }else if( c=='f' ){
        c = '\f';
      }else if( c=='r' ){
        c = '\r';
      }else if( c=='"' ){
        c = '"';
      }else if( c=='\'' ){
        c = '\'';
      }else if( c=='\\' ){
        c = '\\';
      }else if( c>='0' && c<='7' ){
        c -= '0';
        if( z[i+1]>='0' && z[i+1]<='7' ){
          i++;
          c = (c<<3) + z[i] - '0';
          if( z[i+1]>='0' && z[i+1]<='7' ){
            i++;
            c = (c<<3) + z[i] - '0';
          }
        }
      }
    }
    z[j] = c;
  }
  if( j<i ) z[j] = 0;
}

/*
** Return the value of a hexadecimal digit.  Return -1 if the input
** is not a hex digit.
*/
static int hexDigitValue(char c){
  if( c>='0' && c<='9' ) return c - '0';
  if( c>='a' && c<='f' ) return c - 'a' + 10;
  if( c>='A' && c<='F' ) return c - 'A' + 10;
  return -1;
}

/*
** Interpret zArg as an integer value, possibly with suffixes.
*/
static sqlite3_int64 integerValue(const char *zArg){
  sqlite3_int64 v = 0;
  static const struct { char *zSuffix; int iMult; } aMult[] = {
    { "KiB", 1024 },
    { "MiB", 1024*1024 },
    { "GiB", 1024*1024*1024 },
    { "KB",  1000 },
    { "MB",  1000000 },
    { "GB",  1000000000 },
    { "K",   1000 },
    { "M",   1000000 },
    { "G",   1000000000 },
  };
  int i;
  int isNeg = 0;
  if( zArg[0]=='-' ){
    isNeg = 1;
    zArg++;
  }else if( zArg[0]=='+' ){
    zArg++;
  }
  if( zArg[0]=='0' && zArg[1]=='x' ){
    int x;
    zArg += 2;
    while( (x = hexDigitValue(zArg[0]))>=0 ){
      v = (v<<4) + x;
      zArg++;
    }
  }else{
    while( IsDigit(zArg[0]) ){
      v = v*10 + zArg[0] - '0';
      zArg++;
    }
  }
  for(i=0; i<ArraySize(aMult); i++){
    if( sqlite3_stricmp(aMult[i].zSuffix, zArg)==0 ){
      v *= aMult[i].iMult;
      break;
    }
  }
  return isNeg? -v : v;
}

/*
** Interpret zArg as either an integer or a boolean value.  Return 1 or 0
** for TRUE and FALSE.  Return the integer value if appropriate.
*/
static int booleanValue(char *zArg){
  int i;
  if( zArg[0]=='0' && zArg[1]=='x' ){
    for(i=2; hexDigitValue(zArg[i])>=0; i++){}
  }else{
    for(i=0; zArg[i]>='0' && zArg[i]<='9'; i++){}
  }
  if( i>0 && zArg[i]==0 ) return (int)(integerValue(zArg) & 0xffffffff);
  if( sqlite3_stricmp(zArg, "on")==0 || sqlite3_stricmp(zArg,"yes")==0 ){
    return 1;
  }
  if( sqlite3_stricmp(zArg, "off")==0 || sqlite3_stricmp(zArg,"no")==0 ){
    return 0;
  }
  fprintf(stderr, "ERROR: Not a boolean value: \"%s\". Assuming \"no\".\n",
          zArg);
  return 0;
}

/*
** Close an output file, assuming it is not stderr or stdout
*/
static void output_file_close(FILE *f){
  if( f && f!=stdout && f!=stderr ) fclose(f);
}

/*
** Try to open an output file.   The names "stdout" and "stderr" are
** recognized and do the right thing.  NULL is returned if the output 
** filename is "off".
*/
static FILE *output_file_open(const char *zFile){
  FILE *f;
  if( strcmp(zFile,"stdout")==0 ){
    f = stdout;
  }else if( strcmp(zFile, "stderr")==0 ){
    f = stderr;
  }else if( strcmp(zFile, "off")==0 ){
    f = 0;
  }else{
    f = fopen(zFile, "wb");
    if( f==0 ){
      fprintf(stderr, "Error: cannot open \"%s\"\n", zFile);
    }
  }
  return f;
}

/*
** A routine for handling output from sqlite3_trace().
*/
static void sql_trace_callback(void *pArg, const char *z){
  FILE *f = (FILE*)pArg;
  if( f ){
    int i = (int)strlen(z);
    while( i>0 && z[i-1]==';' ){ i--; }
    fprintf(f, "%.*s;\n", i, z);
  }
}

/*
** A no-op routine that runs with the ".breakpoint" doc-command.  This is
** a useful spot to set a debugger breakpoint.
*/
static void test_breakpoint(void){
  static int nCall = 0;
  nCall++;
}

/*
** An object used to read a CSV and other files for import.
*/
typedef struct ImportCtx ImportCtx;
struct ImportCtx {
  const char *zFile;  /* Name of the input file */
  FILE *in;           /* Read the CSV text from this input stream */
  char *z;            /* Accumulated text for a field */
  int n;              /* Number of bytes in z */
  int nAlloc;         /* Space allocated for z[] */
  int nLine;          /* Current line number */
  int cTerm;          /* Character that terminated the most recent field */
  int cColSep;        /* The column separator character.  (Usually ",") */
  int cRowSep;        /* The row separator character.  (Usually "\n") */
};

/* Append a single byte to z[] */
static void import_append_char(ImportCtx *p, int c){
  if( p->n+1>=p->nAlloc ){
    p->nAlloc += p->nAlloc + 100;
    p->z = sqlite3_realloc64(p->z, p->nAlloc);
    if( p->z==0 ){
      fprintf(stderr, "out of memory\n");
      exit(1);
    }
  }
  p->z[p->n++] = (char)c;
}

/* Read a single field of CSV text.  Compatible with rfc4180 and extended
** with the option of having a separator other than ",".
**
**   +  Input comes from p->in.
**   +  Store results in p->z of length p->n.  Space to hold p->z comes
**      from sqlite3_malloc64().
**   +  Use p->cSep as the column separator.  The default is ",".
**   +  Use p->rSep as the row separator.  The default is "\n".
**   +  Keep track of the line number in p->nLine.
**   +  Store the character that terminates the field in p->cTerm.  Store
**      EOF on end-of-file.
**   +  Report syntax errors on stderr
*/
static char *SQLITE_CDECL csv_read_one_field(ImportCtx *p){
  int c;
  int cSep = p->cColSep;
  int rSep = p->cRowSep;
  p->n = 0;
  c = fgetc(p->in);
  if( c==EOF || seenInterrupt ){
    p->cTerm = EOF;
    return 0;
  }
  if( c=='"' ){
    int pc, ppc;
    int startLine = p->nLine;
    int cQuote = c;
    pc = ppc = 0;
    while( 1 ){
      c = fgetc(p->in);
      if( c==rSep ) p->nLine++;
      if( c==cQuote ){
        if( pc==cQuote ){
          pc = 0;
          continue;
        }
      }
      if( (c==cSep && pc==cQuote)
       || (c==rSep && pc==cQuote)
       || (c==rSep && pc=='\r' && ppc==cQuote)
       || (c==EOF && pc==cQuote)
      ){
        do{ p->n--; }while( p->z[p->n]!=cQuote );
        p->cTerm = c;
        break;
      }
      if( pc==cQuote && c!='\r' ){
        fprintf(stderr, "%s:%d: unescaped %c character\n",
                p->zFile, p->nLine, cQuote);
      }
      if( c==EOF ){
        fprintf(stderr, "%s:%d: unterminated %c-quoted field\n",
                p->zFile, startLine, cQuote);
        p->cTerm = c;
        break;
      }
      import_append_char(p, c);
      ppc = pc;
      pc = c;
    }
  }else{
    while( c!=EOF && c!=cSep && c!=rSep ){
      import_append_char(p, c);
      c = fgetc(p->in);
    }
    if( c==rSep ){
      p->nLine++;
      if( p->n>0 && p->z[p->n-1]=='\r' ) p->n--;
    }
    p->cTerm = c;
  }
  if( p->z ) p->z[p->n] = 0;
  return p->z;
}

/* Read a single field of ASCII delimited text.
**
**   +  Input comes from p->in.
**   +  Store results in p->z of length p->n.  Space to hold p->z comes
**      from sqlite3_malloc64().
**   +  Use p->cSep as the column separator.  The default is "\x1F".
**   +  Use p->rSep as the row separator.  The default is "\x1E".
**   +  Keep track of the row number in p->nLine.
**   +  Store the character that terminates the field in p->cTerm.  Store
**      EOF on end-of-file.
**   +  Report syntax errors on stderr
*/
static char *SQLITE_CDECL ascii_read_one_field(ImportCtx *p){
  int c;
  int cSep = p->cColSep;
  int rSep = p->cRowSep;
  p->n = 0;
  c = fgetc(p->in);
  if( c==EOF || seenInterrupt ){
    p->cTerm = EOF;
    return 0;
  }
  while( c!=EOF && c!=cSep && c!=rSep ){
    import_append_char(p, c);
    c = fgetc(p->in);
  }
  if( c==rSep ){
    p->nLine++;
  }
  p->cTerm = c;
  if( p->z ) p->z[p->n] = 0;
  return p->z;
}

/*
** Try to transfer data for table zTable.  If an error is seen while
** moving forward, try to go backwards.  The backwards movement won't
** work for WITHOUT ROWID tables.
*/
static void tryToCloneData(
  ShellState *p,
  sqlite3 *newDb,
  const char *zTable
){
  sqlite3_stmt *pQuery = 0; 
  sqlite3_stmt *pInsert = 0;
  char *zQuery = 0;
  char *zInsert = 0;
  int rc;
  int i, j, n;
  int nTable = (int)strlen(zTable);
  int k = 0;
  int cnt = 0;
  const int spinRate = 10000;

  zQuery = sqlite3_mprintf("SELECT * FROM \"%w\"", zTable);
  rc = sqlite3_prepare_v2(p->db, zQuery, -1, &pQuery, 0);
  if( rc ){
    fprintf(stderr, "Error %d: %s on [%s]\n",
            sqlite3_extended_errcode(p->db), sqlite3_errmsg(p->db),
            zQuery);
    goto end_data_xfer;
  }
  n = sqlite3_column_count(pQuery);
  zInsert = sqlite3_malloc64(200 + nTable + n*3);
  if( zInsert==0 ){
    fprintf(stderr, "out of memory\n");
    goto end_data_xfer;
  }
  sqlite3_snprintf(200+nTable,zInsert,
                   "INSERT OR IGNORE INTO \"%s\" VALUES(?", zTable);
  i = (int)strlen(zInsert);
  for(j=1; j<n; j++){
    memcpy(zInsert+i, ",?", 2);
    i += 2;
  }
  memcpy(zInsert+i, ");", 3);
  rc = sqlite3_prepare_v2(newDb, zInsert, -1, &pInsert, 0);
  if( rc ){
    fprintf(stderr, "Error %d: %s on [%s]\n",
            sqlite3_extended_errcode(newDb), sqlite3_errmsg(newDb),
            zQuery);
    goto end_data_xfer;
  }
  for(k=0; k<2; k++){
    while( (rc = sqlite3_step(pQuery))==SQLITE_ROW ){
      for(i=0; i<n; i++){
        switch( sqlite3_column_type(pQuery, i) ){
          case SQLITE_NULL: {
            sqlite3_bind_null(pInsert, i+1);
            break;
          }
          case SQLITE_INTEGER: {
            sqlite3_bind_int64(pInsert, i+1, sqlite3_column_int64(pQuery,i));
            break;
          }
          case SQLITE_FLOAT: {
            sqlite3_bind_double(pInsert, i+1, sqlite3_column_double(pQuery,i));
            break;
          }
          case SQLITE_TEXT: {
            sqlite3_bind_text(pInsert, i+1,
                             (const char*)sqlite3_column_text(pQuery,i),
                             -1, SQLITE_STATIC);
            break;
          }
          case SQLITE_BLOB: {
            sqlite3_bind_blob(pInsert, i+1, sqlite3_column_blob(pQuery,i),
                                            sqlite3_column_bytes(pQuery,i),
                                            SQLITE_STATIC);
            break;
          }
        }
      } /* End for */
      rc = sqlite3_step(pInsert);
      if( rc!=SQLITE_OK && rc!=SQLITE_ROW && rc!=SQLITE_DONE ){
        fprintf(stderr, "Error %d: %s\n", sqlite3_extended_errcode(newDb),
                        sqlite3_errmsg(newDb));
      }
      sqlite3_reset(pInsert);
      cnt++;
      if( (cnt%spinRate)==0 ){
        printf("%c\b", "|/-\\"[(cnt/spinRate)%4]);
        fflush(stdout);
      }
    } /* End while */
    if( rc==SQLITE_DONE ) break;
    sqlite3_finalize(pQuery);
    sqlite3_free(zQuery);
    zQuery = sqlite3_mprintf("SELECT * FROM \"%w\" ORDER BY rowid DESC;",
                             zTable);
    rc = sqlite3_prepare_v2(p->db, zQuery, -1, &pQuery, 0);
    if( rc ){
      fprintf(stderr, "Warning: cannot step \"%s\" backwards", zTable);
      break;
    }
  } /* End for(k=0...) */

end_data_xfer:
  sqlite3_finalize(pQuery);
  sqlite3_finalize(pInsert);
  sqlite3_free(zQuery);
  sqlite3_free(zInsert);
}


/*
** Try to transfer all rows of the schema that match zWhere.  For
** each row, invoke xForEach() on the object defined by that row.
** If an error is encountered while moving forward through the
** sqlite_master table, try again moving backwards.
*/
static void tryToCloneSchema(
  ShellState *p,
  sqlite3 *newDb,
  const char *zWhere,
  void (*xForEach)(ShellState*,sqlite3*,const char*)
){
  sqlite3_stmt *pQuery = 0;
  char *zQuery = 0;
  int rc;
  const unsigned char *zName;
  const unsigned char *zSql;
  char *zErrMsg = 0;

  zQuery = sqlite3_mprintf("SELECT name, sql FROM sqlite_master"
                           " WHERE %s", zWhere);
  rc = sqlite3_prepare_v2(p->db, zQuery, -1, &pQuery, 0);
  if( rc ){
    fprintf(stderr, "Error: (%d) %s on [%s]\n",
                    sqlite3_extended_errcode(p->db), sqlite3_errmsg(p->db),
                    zQuery);
    goto end_schema_xfer;
  }
  while( (rc = sqlite3_step(pQuery))==SQLITE_ROW ){
    zName = sqlite3_column_text(pQuery, 0);
    zSql = sqlite3_column_text(pQuery, 1);
    printf("%s... ", zName); fflush(stdout);
    sqlite3_exec(newDb, (const char*)zSql, 0, 0, &zErrMsg);
    if( zErrMsg ){
      fprintf(stderr, "Error: %s\nSQL: [%s]\n", zErrMsg, zSql);
      sqlite3_free(zErrMsg);
      zErrMsg = 0;
    }
    if( xForEach ){
      xForEach(p, newDb, (const char*)zName);
    }
    printf("done\n");
  }
  if( rc!=SQLITE_DONE ){
    sqlite3_finalize(pQuery);
    sqlite3_free(zQuery);
    zQuery = sqlite3_mprintf("SELECT name, sql FROM sqlite_master"
                             " WHERE %s ORDER BY rowid DESC", zWhere);
    rc = sqlite3_prepare_v2(p->db, zQuery, -1, &pQuery, 0);
    if( rc ){
      fprintf(stderr, "Error: (%d) %s on [%s]\n",
                      sqlite3_extended_errcode(p->db), sqlite3_errmsg(p->db),
                      zQuery);
      goto end_schema_xfer;
    }
    while( (rc = sqlite3_step(pQuery))==SQLITE_ROW ){
      zName = sqlite3_column_text(pQuery, 0);
      zSql = sqlite3_column_text(pQuery, 1);
      printf("%s... ", zName); fflush(stdout);
      sqlite3_exec(newDb, (const char*)zSql, 0, 0, &zErrMsg);
      if( zErrMsg ){
        fprintf(stderr, "Error: %s\nSQL: [%s]\n", zErrMsg, zSql);
        sqlite3_free(zErrMsg);
        zErrMsg = 0;
      }
      if( xForEach ){
        xForEach(p, newDb, (const char*)zName);
      }
      printf("done\n");
    }
  }
end_schema_xfer:
  sqlite3_finalize(pQuery);
  sqlite3_free(zQuery);
}

/*
** Open a new database file named "zNewDb".  Try to recover as much information
** as possible out of the main database (which might be corrupt) and write it
** into zNewDb.
*/
static void tryToClone(ShellState *p, const char *zNewDb){
  int rc;
  sqlite3 *newDb = 0;
  if( access(zNewDb,0)==0 ){
    fprintf(stderr, "File \"%s\" already exists.\n", zNewDb);
    return;
  }
  rc = sqlite3_open(zNewDb, &newDb);
  if( rc ){
    fprintf(stderr, "Cannot create output database: %s\n",
            sqlite3_errmsg(newDb));
  }else{
    sqlite3_exec(p->db, "PRAGMA writable_schema=ON;", 0, 0, 0);
    sqlite3_exec(newDb, "BEGIN EXCLUSIVE;", 0, 0, 0);
    tryToCloneSchema(p, newDb, "type='table'", tryToCloneData);
    tryToCloneSchema(p, newDb, "type!='table'", 0);
    sqlite3_exec(newDb, "COMMIT;", 0, 0, 0);
    sqlite3_exec(p->db, "PRAGMA writable_schema=OFF;", 0, 0, 0);
  }
  sqlite3_close(newDb);
}

/*
** Change the output file back to stdout
*/
static void output_reset(ShellState *p){
  if( p->outfile[0]=='|' ){
#ifndef SQLITE_OMIT_POPEN
    pclose(p->out);
#endif
  }else{
    output_file_close(p->out);
  }
  p->outfile[0] = 0;
  p->out = stdout;
}

/*
** Run an SQL command and return the single integer result.
*/
static int db_int(ShellState *p, const char *zSql){
  sqlite3_stmt *pStmt;
  int res = 0;
  sqlite3_prepare_v2(p->db, zSql, -1, &pStmt, 0);
  if( pStmt && sqlite3_step(pStmt)==SQLITE_ROW ){
    res = sqlite3_column_int(pStmt,0);
  }
  sqlite3_finalize(pStmt);
  return res;
}

/*
** Convert a 2-byte or 4-byte big-endian integer into a native integer
*/
unsigned int get2byteInt(unsigned char *a){
  return (a[0]<<8) + a[1];
}
unsigned int get4byteInt(unsigned char *a){
  return (a[0]<<24) + (a[1]<<16) + (a[2]<<8) + a[3];
}

/*
** Implementation of the ".info" command.
**
** Return 1 on error, 2 to exit, and 0 otherwise.
*/
static int shell_dbinfo_command(ShellState *p, int nArg, char **azArg){
  static const struct { const char *zName; int ofst; } aField[] = {
     { "file change counter:",  24  },
     { "database page count:",  28  },
     { "freelist page count:",  36  },
     { "schema cookie:",        40  },
     { "schema format:",        44  },
     { "default cache size:",   48  },
     { "autovacuum top root:",  52  },
     { "incremental vacuum:",   64  },
     { "text encoding:",        56  },
     { "user version:",         60  },
     { "application id:",       68  },
     { "software version:",     96  },
  };
  static const struct { const char *zName; const char *zSql; } aQuery[] = {
     { "number of tables:",
       "SELECT count(*) FROM %s WHERE type='table'" },
     { "number of indexes:",
       "SELECT count(*) FROM %s WHERE type='index'" },
     { "number of triggers:",
       "SELECT count(*) FROM %s WHERE type='trigger'" },
     { "number of views:",
       "SELECT count(*) FROM %s WHERE type='view'" },
     { "schema size:",
       "SELECT total(length(sql)) FROM %s" },
  };
  sqlite3_file *pFile;
  int i;
  char *zSchemaTab;
  char *zDb = nArg>=2 ? azArg[1] : "main";
  unsigned char aHdr[100];
  open_db(p, 0);
  if( p->db==0 ) return 1;
  sqlite3_file_control(p->db, zDb, SQLITE_FCNTL_FILE_POINTER, &pFile);
  if( pFile==0 || pFile->pMethods==0 || pFile->pMethods->xRead==0 ){
    return 1;
  }
  i = pFile->pMethods->xRead(pFile, aHdr, 100, 0);
  if( i!=SQLITE_OK ){
    fprintf(stderr, "unable to read database header\n");
    return 1;
  }
  i = get2byteInt(aHdr+16);
  if( i==1 ) i = 65536;
  fprintf(p->out, "%-20s %d\n", "database page size:", i);
  fprintf(p->out, "%-20s %d\n", "write format:", aHdr[18]);
  fprintf(p->out, "%-20s %d\n", "read format:", aHdr[19]);
  fprintf(p->out, "%-20s %d\n", "reserved bytes:", aHdr[20]);
  for(i=0; i<sizeof(aField)/sizeof(aField[0]); i++){
    int ofst = aField[i].ofst;
    unsigned int val = get4byteInt(aHdr + ofst);
    fprintf(p->out, "%-20s %u", aField[i].zName, val);
    switch( ofst ){
      case 56: {
        if( val==1 ) fprintf(p->out, " (utf8)"); 
        if( val==2 ) fprintf(p->out, " (utf16le)"); 
        if( val==3 ) fprintf(p->out, " (utf16be)"); 
      }
    }
    fprintf(p->out, "\n");
  }
  if( zDb==0 ){
    zSchemaTab = sqlite3_mprintf("main.sqlite_master");
  }else if( strcmp(zDb,"temp")==0 ){
    zSchemaTab = sqlite3_mprintf("%s", "sqlite_temp_master");
  }else{
    zSchemaTab = sqlite3_mprintf("\"%w\".sqlite_master", zDb);
  }
  for(i=0; i<sizeof(aQuery)/sizeof(aQuery[0]); i++){
    char *zSql = sqlite3_mprintf(aQuery[i].zSql, zSchemaTab);
    int val = db_int(p, zSql);
    sqlite3_free(zSql);
    fprintf(p->out, "%-20s %d\n", aQuery[i].zName, val);
  }
  sqlite3_free(zSchemaTab);
  return 0;
}


/*
** If an input line begins with "." then invoke this routine to
** process that line.
**
** Return 1 on error, 2 to exit, and 0 otherwise.
*/
static int do_meta_command(char *zLine, ShellState *p){
  int h = 1;
  int nArg = 0;
  int n, c;
  int rc = 0;
  char *azArg[50];

  /* Parse the input line into tokens.
  */
  while( zLine[h] && nArg<ArraySize(azArg) ){
    while( IsSpace(zLine[h]) ){ h++; }
    if( zLine[h]==0 ) break;
    if( zLine[h]=='\'' || zLine[h]=='"' ){
      int delim = zLine[h++];
      azArg[nArg++] = &zLine[h];
      while( zLine[h] && zLine[h]!=delim ){ 
        if( zLine[h]=='\\' && delim=='"' && zLine[h+1]!=0 ) h++;
        h++; 
      }
      if( zLine[h]==delim ){
        zLine[h++] = 0;
      }
      if( delim=='"' ) resolve_backslashes(azArg[nArg-1]);
    }else{
      azArg[nArg++] = &zLine[h];
      while( zLine[h] && !IsSpace(zLine[h]) ){ h++; }
      if( zLine[h] ) zLine[h++] = 0;
      resolve_backslashes(azArg[nArg-1]);
    }
  }

  /* Process the input line.
  */
  if( nArg==0 ) return 0; /* no tokens, no error */
  n = strlen30(azArg[0]);
  c = azArg[0][0];
  if( (c=='b' && n>=3 && strncmp(azArg[0], "backup", n)==0)
   || (c=='s' && n>=3 && strncmp(azArg[0], "save", n)==0)
  ){
    const char *zDestFile = 0;
    const char *zDb = 0;
    sqlite3 *pDest;
    sqlite3_backup *pBackup;
    int j;
    for(j=1; j<nArg; j++){
      const char *z = azArg[j];
      if( z[0]=='-' ){
        while( z[0]=='-' ) z++;
        /* No options to process at this time */
        {
          fprintf(stderr, "unknown option: %s\n", azArg[j]);
          return 1;
        }
      }else if( zDestFile==0 ){
        zDestFile = azArg[j];
      }else if( zDb==0 ){
        zDb = zDestFile;
        zDestFile = azArg[j];
      }else{
        fprintf(stderr, "too many arguments to .backup\n");
        return 1;
      }
    }
    if( zDestFile==0 ){
      fprintf(stderr, "missing FILENAME argument on .backup\n");
      return 1;
    }
    if( zDb==0 ) zDb = "main";
    rc = sqlite3_open(zDestFile, &pDest);
    if( rc!=SQLITE_OK ){
      fprintf(stderr, "Error: cannot open \"%s\"\n", zDestFile);
      sqlite3_close(pDest);
      return 1;
    }
    open_db(p, 0);
    pBackup = sqlite3_backup_init(pDest, "main", p->db, zDb);
    if( pBackup==0 ){
      fprintf(stderr, "Error: %s\n", sqlite3_errmsg(pDest));
      sqlite3_close(pDest);
      return 1;
    }
    while(  (rc = sqlite3_backup_step(pBackup,100))==SQLITE_OK ){}
    sqlite3_backup_finish(pBackup);
    if( rc==SQLITE_DONE ){
      rc = 0;
    }else{
      fprintf(stderr, "Error: %s\n", sqlite3_errmsg(pDest));
      rc = 1;
    }
    sqlite3_close(pDest);
  }else

  if( c=='b' && n>=3 && strncmp(azArg[0], "bail", n)==0 ){
    if( nArg==2 ){
      bail_on_error = booleanValue(azArg[1]);
    }else{
      fprintf(stderr, "Usage: .bail on|off\n");
      rc = 1;
    }
  }else

  if( c=='b' && n>=3 && strncmp(azArg[0], "binary", n)==0 ){
    if( nArg==2 ){
      if( booleanValue(azArg[1]) ){
        setBinaryMode(p->out);
      }else{
        setTextMode(p->out);
      }
    }else{
      fprintf(stderr, "Usage: .binary on|off\n");
      rc = 1;
    }
  }else

  /* The undocumented ".breakpoint" command causes a call to the no-op
  ** routine named test_breakpoint().
  */
  if( c=='b' && n>=3 && strncmp(azArg[0], "breakpoint", n)==0 ){
    test_breakpoint();
  }else

  if( c=='c' && strncmp(azArg[0], "clone", n)==0 ){
    if( nArg==2 ){
      tryToClone(p, azArg[1]);
    }else{
      fprintf(stderr, "Usage: .clone FILENAME\n");
      rc = 1;
    }
  }else

  if( c=='d' && n>1 && strncmp(azArg[0], "databases", n)==0 ){
    ShellState data;
    char *zErrMsg = 0;
    open_db(p, 0);
    memcpy(&data, p, sizeof(data));
    data.showHeader = 1;
    data.mode = MODE_Column;
    data.colWidth[0] = 3;
    data.colWidth[1] = 15;
    data.colWidth[2] = 58;
    data.cnt = 0;
    sqlite3_exec(p->db, "PRAGMA database_list; ", callback, &data, &zErrMsg);
    if( zErrMsg ){
      fprintf(stderr,"Error: %s\n", zErrMsg);
      sqlite3_free(zErrMsg);
      rc = 1;
    }
  }else

  if( c=='d' && strncmp(azArg[0], "dbinfo", n)==0 ){
    rc = shell_dbinfo_command(p, nArg, azArg);
  }else

  if( c=='d' && strncmp(azArg[0], "dump", n)==0 ){
    open_db(p, 0);
    /* When playing back a "dump", the content might appear in an order
    ** which causes immediate foreign key constraints to be violated.
    ** So disable foreign-key constraint enforcement to prevent problems. */
    if( nArg!=1 && nArg!=2 ){
      fprintf(stderr, "Usage: .dump ?LIKE-PATTERN?\n");
      rc = 1;
      goto meta_command_exit;
    }
    fprintf(p->out, "PRAGMA foreign_keys=OFF;\n");
    fprintf(p->out, "BEGIN TRANSACTION;\n");
    p->writableSchema = 0;
    sqlite3_exec(p->db, "SAVEPOINT dump; PRAGMA writable_schema=ON", 0, 0, 0);
    p->nErr = 0;
    if( nArg==1 ){
      run_schema_dump_query(p, 
        "SELECT name, type, sql FROM sqlite_master "
        "WHERE sql NOT NULL AND type=='table' AND name!='sqlite_sequence'"
      );
      run_schema_dump_query(p, 
        "SELECT name, type, sql FROM sqlite_master "
        "WHERE name=='sqlite_sequence'"
      );
      run_table_dump_query(p,
        "SELECT sql FROM sqlite_master "
        "WHERE sql NOT NULL AND type IN ('index','trigger','view')", 0
      );
    }else{
      int i;
      for(i=1; i<nArg; i++){
        zShellStatic = azArg[i];
        run_schema_dump_query(p,
          "SELECT name, type, sql FROM sqlite_master "
          "WHERE tbl_name LIKE shellstatic() AND type=='table'"
          "  AND sql NOT NULL");
        run_table_dump_query(p,
          "SELECT sql FROM sqlite_master "
          "WHERE sql NOT NULL"
          "  AND type IN ('index','trigger','view')"
          "  AND tbl_name LIKE shellstatic()", 0
        );
        zShellStatic = 0;
      }
    }
    if( p->writableSchema ){
      fprintf(p->out, "PRAGMA writable_schema=OFF;\n");
      p->writableSchema = 0;
    }
    sqlite3_exec(p->db, "PRAGMA writable_schema=OFF;", 0, 0, 0);
    sqlite3_exec(p->db, "RELEASE dump;", 0, 0, 0);
    fprintf(p->out, p->nErr ? "ROLLBACK; -- due to errors\n" : "COMMIT;\n");
  }else

  if( c=='e' && strncmp(azArg[0], "echo", n)==0 ){
    if( nArg==2 ){
      p->echoOn = booleanValue(azArg[1]);
    }else{
      fprintf(stderr, "Usage: .echo on|off\n");
      rc = 1;
    }
  }else

  if( c=='e' && strncmp(azArg[0], "eqp", n)==0 ){
    if( nArg==2 ){
      p->autoEQP = booleanValue(azArg[1]);
    }else{
      fprintf(stderr, "Usage: .eqp on|off\n");
      rc = 1;
    }   
  }else

  if( c=='e' && strncmp(azArg[0], "exit", n)==0 ){
    if( nArg>1 && (rc = (int)integerValue(azArg[1]))!=0 ) exit(rc);
    rc = 2;
  }else

  if( c=='e' && strncmp(azArg[0], "explain", n)==0 ){
    int val = nArg>=2 ? booleanValue(azArg[1]) : 1;
    if(val == 1) {
      if(!p->normalMode.valid) {
        p->normalMode.valid = 1;
        p->normalMode.mode = p->mode;
        p->normalMode.showHeader = p->showHeader;
        memcpy(p->normalMode.colWidth,p->colWidth,sizeof(p->colWidth));
      }
      /* We could put this code under the !p->explainValid
      ** condition so that it does not execute if we are already in
      ** explain mode. However, always executing it allows us an easy
      ** was to reset to explain mode in case the user previously
      ** did an .explain followed by a .width, .mode or .header
      ** command.
      */
      p->mode = MODE_Explain;
      p->showHeader = 1;
      memset(p->colWidth,0,sizeof(p->colWidth));
      p->colWidth[0] = 4;                  /* addr */
      p->colWidth[1] = 13;                 /* opcode */
      p->colWidth[2] = 4;                  /* P1 */
      p->colWidth[3] = 4;                  /* P2 */
      p->colWidth[4] = 4;                  /* P3 */
      p->colWidth[5] = 13;                 /* P4 */
      p->colWidth[6] = 2;                  /* P5 */
      p->colWidth[7] = 13;                  /* Comment */
    }else if (p->normalMode.valid) {
      p->normalMode.valid = 0;
      p->mode = p->normalMode.mode;
      p->showHeader = p->normalMode.showHeader;
      memcpy(p->colWidth,p->normalMode.colWidth,sizeof(p->colWidth));
    }
  }else

  if( c=='f' && strncmp(azArg[0], "fullschema", n)==0 ){
    ShellState data;
    char *zErrMsg = 0;
    int doStats = 0;
    if( nArg!=1 ){
      fprintf(stderr, "Usage: .fullschema\n");
      rc = 1;
      goto meta_command_exit;
    }
    open_db(p, 0);
    memcpy(&data, p, sizeof(data));
    data.showHeader = 0;
    data.mode = MODE_Semi;
    rc = sqlite3_exec(p->db,
       "SELECT sql FROM"
       "  (SELECT sql sql, type type, tbl_name tbl_name, name name, rowid x"
       "     FROM sqlite_master UNION ALL"
       "   SELECT sql, type, tbl_name, name, rowid FROM sqlite_temp_master) "
       "WHERE type!='meta' AND sql NOTNULL AND name NOT LIKE 'sqlite_%' "
       "ORDER BY rowid",
       callback, &data, &zErrMsg
    );
    if( rc==SQLITE_OK ){
      sqlite3_stmt *pStmt;
      rc = sqlite3_prepare_v2(p->db,
               "SELECT rowid FROM sqlite_master"
               " WHERE name GLOB 'sqlite_stat[134]'",
               -1, &pStmt, 0);
      doStats = sqlite3_step(pStmt)==SQLITE_ROW;
      sqlite3_finalize(pStmt);
    }
    if( doStats==0 ){
      fprintf(p->out, "/* No STAT tables available */\n");
    }else{
      fprintf(p->out, "ANALYZE sqlite_master;\n");
      sqlite3_exec(p->db, "SELECT 'ANALYZE sqlite_master'",
                   callback, &data, &zErrMsg);
      data.mode = MODE_Insert;
      data.zDestTable = "sqlite_stat1";
      shell_exec(p->db, "SELECT * FROM sqlite_stat1",
                 shell_callback, &data,&zErrMsg);
      data.zDestTable = "sqlite_stat3";
      shell_exec(p->db, "SELECT * FROM sqlite_stat3",
                 shell_callback, &data,&zErrMsg);
      data.zDestTable = "sqlite_stat4";
      shell_exec(p->db, "SELECT * FROM sqlite_stat4",
                 shell_callback, &data, &zErrMsg);
      fprintf(p->out, "ANALYZE sqlite_master;\n");
    }
  }else

  if( c=='h' && strncmp(azArg[0], "headers", n)==0 ){
    if( nArg==2 ){
      p->showHeader = booleanValue(azArg[1]);
    }else{
      fprintf(stderr, "Usage: .headers on|off\n");
      rc = 1;
    }
  }else

  if( c=='h' && strncmp(azArg[0], "help", n)==0 ){
    fprintf(p->out, "%s", zHelp);
  }else

  if( c=='i' && strncmp(azArg[0], "import", n)==0 ){
    char *zTable;               /* Insert data into this table */
    char *zFile;                /* Name of file to extra content from */
    sqlite3_stmt *pStmt = NULL; /* A statement */
    int nCol;                   /* Number of columns in the table */
    int nByte;                  /* Number of bytes in an SQL string */
    int i, j;                   /* Loop counters */
    int needCommit;             /* True to COMMIT or ROLLBACK at end */
    int nSep;                   /* Number of bytes in p->colSeparator[] */
    char *zSql;                 /* An SQL statement */
    ImportCtx sCtx;             /* Reader context */
    char *(SQLITE_CDECL *xRead)(ImportCtx*); /* Func to read one value */
    int (SQLITE_CDECL *xCloser)(FILE*);      /* Func to close file */

    if( nArg!=3 ){
      fprintf(stderr, "Usage: .import FILE TABLE\n");
      goto meta_command_exit;
    }
    zFile = azArg[1];
    zTable = azArg[2];
    seenInterrupt = 0;
    memset(&sCtx, 0, sizeof(sCtx));
    open_db(p, 0);
    nSep = strlen30(p->colSeparator);
    if( nSep==0 ){
      fprintf(stderr, "Error: non-null column separator required for import\n");
      return 1;
    }
    if( nSep>1 ){
      fprintf(stderr, "Error: multi-character column separators not allowed"
                      " for import\n");
      return 1;
    }
    nSep = strlen30(p->rowSeparator);
    if( nSep==0 ){
      fprintf(stderr, "Error: non-null row separator required for import\n");
      return 1;
    }
    if( nSep==2 && p->mode==MODE_Csv && strcmp(p->rowSeparator, SEP_CrLf)==0 ){
      /* When importing CSV (only), if the row separator is set to the
      ** default output row separator, change it to the default input
      ** row separator.  This avoids having to maintain different input
      ** and output row separators. */
      sqlite3_snprintf(sizeof(p->rowSeparator), p->rowSeparator, SEP_Row);
      nSep = strlen30(p->rowSeparator);
    }
    if( nSep>1 ){
      fprintf(stderr, "Error: multi-character row separators not allowed"
                      " for import\n");
      return 1;
    }
    sCtx.zFile = zFile;
    sCtx.nLine = 1;
    if( sCtx.zFile[0]=='|' ){
#ifdef SQLITE_OMIT_POPEN
      fprintf(stderr, "Error: pipes are not supported in this OS\n");
      return 1;
#else
      sCtx.in = popen(sCtx.zFile+1, "r");
      sCtx.zFile = "<pipe>";
      xCloser = pclose;
#endif
    }else{
      sCtx.in = fopen(sCtx.zFile, "rb");
      xCloser = fclose;
    }
    if( p->mode==MODE_Ascii ){
      xRead = ascii_read_one_field;
    }else{
      xRead = csv_read_one_field;
    }
    if( sCtx.in==0 ){
      fprintf(stderr, "Error: cannot open \"%s\"\n", zFile);
      return 1;
    }
    sCtx.cColSep = p->colSeparator[0];
    sCtx.cRowSep = p->rowSeparator[0];
    zSql = sqlite3_mprintf("SELECT * FROM %s", zTable);
    if( zSql==0 ){
      fprintf(stderr, "Error: out of memory\n");
      xCloser(sCtx.in);
      return 1;
    }
    nByte = strlen30(zSql);
    rc = sqlite3_prepare_v2(p->db, zSql, -1, &pStmt, 0);
    import_append_char(&sCtx, 0);    /* To ensure sCtx.z is allocated */
    if( rc && sqlite3_strglob("no such table: *", sqlite3_errmsg(p->db))==0 ){
      char *zCreate = sqlite3_mprintf("CREATE TABLE %s", zTable);
      char cSep = '(';
      while( xRead(&sCtx) ){
        zCreate = sqlite3_mprintf("%z%c\n  \"%s\" TEXT", zCreate, cSep, sCtx.z);
        cSep = ',';
        if( sCtx.cTerm!=sCtx.cColSep ) break;
      }
      if( cSep=='(' ){
        sqlite3_free(zCreate);
        sqlite3_free(sCtx.z);
        xCloser(sCtx.in);
        fprintf(stderr,"%s: empty file\n", sCtx.zFile);
        return 1;
      }
      zCreate = sqlite3_mprintf("%z\n)", zCreate);
      rc = sqlite3_exec(p->db, zCreate, 0, 0, 0);
      sqlite3_free(zCreate);
      if( rc ){
        fprintf(stderr, "CREATE TABLE %s(...) failed: %s\n", zTable,
                sqlite3_errmsg(p->db));
        sqlite3_free(sCtx.z);
        xCloser(sCtx.in);
        return 1;
      }
      rc = sqlite3_prepare_v2(p->db, zSql, -1, &pStmt, 0);
    }
    sqlite3_free(zSql);
    if( rc ){
      if (pStmt) sqlite3_finalize(pStmt);
      fprintf(stderr,"Error: %s\n", sqlite3_errmsg(p->db));
      xCloser(sCtx.in);
      return 1;
    }
    nCol = sqlite3_column_count(pStmt);
    sqlite3_finalize(pStmt);
    pStmt = 0;
    if( nCol==0 ) return 0; /* no columns, no error */
    zSql = sqlite3_malloc64( nByte*2 + 20 + nCol*2 );
    if( zSql==0 ){
      fprintf(stderr, "Error: out of memory\n");
      xCloser(sCtx.in);
      return 1;
    }
    sqlite3_snprintf(nByte+20, zSql, "INSERT INTO \"%w\" VALUES(?", zTable);
    j = strlen30(zSql);
    for(i=1; i<nCol; i++){
      zSql[j++] = ',';
      zSql[j++] = '?';
    }
    zSql[j++] = ')';
    zSql[j] = 0;
    rc = sqlite3_prepare_v2(p->db, zSql, -1, &pStmt, 0);
    sqlite3_free(zSql);
    if( rc ){
      fprintf(stderr, "Error: %s\n", sqlite3_errmsg(p->db));
      if (pStmt) sqlite3_finalize(pStmt);
      xCloser(sCtx.in);
      return 1;
    }
    needCommit = sqlite3_get_autocommit(p->db);
    if( needCommit ) sqlite3_exec(p->db, "BEGIN", 0, 0, 0);
    do{
      int startLine = sCtx.nLine;
      for(i=0; i<nCol; i++){
        char *z = xRead(&sCtx);
        /*
        ** Did we reach end-of-file before finding any columns?
        ** If so, stop instead of NULL filling the remaining columns.
        */
        if( z==0 && i==0 ) break;
        /*
        ** Did we reach end-of-file OR end-of-line before finding any
        ** columns in ASCII mode?  If so, stop instead of NULL filling
        ** the remaining columns.
        */
        if( p->mode==MODE_Ascii && (z==0 || z[0]==0) && i==0 ) break;
        sqlite3_bind_text(pStmt, i+1, z, -1, SQLITE_TRANSIENT);
        if( i<nCol-1 && sCtx.cTerm!=sCtx.cColSep ){
          fprintf(stderr, "%s:%d: expected %d columns but found %d - "
                          "filling the rest with NULL\n",
                          sCtx.zFile, startLine, nCol, i+1);
          i += 2;
          while( i<=nCol ){ sqlite3_bind_null(pStmt, i); i++; }
        }
      }
      if( sCtx.cTerm==sCtx.cColSep ){
        do{
          xRead(&sCtx);
          i++;
        }while( sCtx.cTerm==sCtx.cColSep );
        fprintf(stderr, "%s:%d: expected %d columns but found %d - "
                        "extras ignored\n",
                        sCtx.zFile, startLine, nCol, i);
      }
      if( i>=nCol ){
        sqlite3_step(pStmt);
        rc = sqlite3_reset(pStmt);
        if( rc!=SQLITE_OK ){
          fprintf(stderr, "%s:%d: INSERT failed: %s\n", sCtx.zFile, startLine,
                  sqlite3_errmsg(p->db));
        }
      }
    }while( sCtx.cTerm!=EOF );

    xCloser(sCtx.in);
    sqlite3_free(sCtx.z);
    sqlite3_finalize(pStmt);
    if( needCommit ) sqlite3_exec(p->db, "COMMIT", 0, 0, 0);
  }else

  if( c=='i' && (strncmp(azArg[0], "indices", n)==0
                 || strncmp(azArg[0], "indexes", n)==0) ){
    ShellState data;
    char *zErrMsg = 0;
    open_db(p, 0);
    memcpy(&data, p, sizeof(data));
    data.showHeader = 0;
    data.mode = MODE_List;
    if( nArg==1 ){
      rc = sqlite3_exec(p->db,
        "SELECT name FROM sqlite_master "
        "WHERE type='index' AND name NOT LIKE 'sqlite_%' "
        "UNION ALL "
        "SELECT name FROM sqlite_temp_master "
        "WHERE type='index' "
        "ORDER BY 1",
        callback, &data, &zErrMsg
      );
    }else if( nArg==2 ){
      zShellStatic = azArg[1];
      rc = sqlite3_exec(p->db,
        "SELECT name FROM sqlite_master "
        "WHERE type='index' AND tbl_name LIKE shellstatic() "
        "UNION ALL "
        "SELECT name FROM sqlite_temp_master "
        "WHERE type='index' AND tbl_name LIKE shellstatic() "
        "ORDER BY 1",
        callback, &data, &zErrMsg
      );
      zShellStatic = 0;
    }else{
      fprintf(stderr, "Usage: .indexes ?LIKE-PATTERN?\n");
      rc = 1;
      goto meta_command_exit;
    }
    if( zErrMsg ){
      fprintf(stderr,"Error: %s\n", zErrMsg);
      sqlite3_free(zErrMsg);
      rc = 1;
    }else if( rc != SQLITE_OK ){
      fprintf(stderr,"Error: querying sqlite_master and sqlite_temp_master\n");
      rc = 1;
    }
  }else

#ifdef SQLITE_ENABLE_IOTRACE
  if( c=='i' && strncmp(azArg[0], "iotrace", n)==0 ){
    SQLITE_API extern void (SQLITE_CDECL *sqlite3IoTrace)(const char*, ...);
    if( iotrace && iotrace!=stdout ) fclose(iotrace);
    iotrace = 0;
    if( nArg<2 ){
      sqlite3IoTrace = 0;
    }else if( strcmp(azArg[1], "-")==0 ){
      sqlite3IoTrace = iotracePrintf;
      iotrace = stdout;
    }else{
      iotrace = fopen(azArg[1], "w");
      if( iotrace==0 ){
        fprintf(stderr, "Error: cannot open \"%s\"\n", azArg[1]);
        sqlite3IoTrace = 0;
        rc = 1;
      }else{
        sqlite3IoTrace = iotracePrintf;
      }
    }
  }else
#endif
  if( c=='l' && n>=5 && strncmp(azArg[0], "limits", n)==0 ){
    static const struct {
       const char *zLimitName;   /* Name of a limit */
       int limitCode;            /* Integer code for that limit */
    } aLimit[] = {
      { "length",                SQLITE_LIMIT_LENGTH                    },
      { "sql_length",            SQLITE_LIMIT_SQL_LENGTH                },
      { "column",                SQLITE_LIMIT_COLUMN                    },
      { "expr_depth",            SQLITE_LIMIT_EXPR_DEPTH                },
      { "compound_select",       SQLITE_LIMIT_COMPOUND_SELECT           },
      { "vdbe_op",               SQLITE_LIMIT_VDBE_OP                   },
      { "function_arg",          SQLITE_LIMIT_FUNCTION_ARG              },
      { "attached",              SQLITE_LIMIT_ATTACHED                  },
      { "like_pattern_length",   SQLITE_LIMIT_LIKE_PATTERN_LENGTH       },
      { "variable_number",       SQLITE_LIMIT_VARIABLE_NUMBER           },
      { "trigger_depth",         SQLITE_LIMIT_TRIGGER_DEPTH             },
      { "worker_threads",        SQLITE_LIMIT_WORKER_THREADS            },
    };
    int i, n2;
    open_db(p, 0);
    if( nArg==1 ){
      for(i=0; i<sizeof(aLimit)/sizeof(aLimit[0]); i++){
        printf("%20s %d\n", aLimit[i].zLimitName, 
               sqlite3_limit(p->db, aLimit[i].limitCode, -1));
      }
    }else if( nArg>3 ){
      fprintf(stderr, "Usage: .limit NAME ?NEW-VALUE?\n");
      rc = 1;
      goto meta_command_exit;
    }else{
      int iLimit = -1;
      n2 = strlen30(azArg[1]);
      for(i=0; i<sizeof(aLimit)/sizeof(aLimit[0]); i++){
        if( sqlite3_strnicmp(aLimit[i].zLimitName, azArg[1], n2)==0 ){
          if( iLimit<0 ){
            iLimit = i;
          }else{
            fprintf(stderr, "ambiguous limit: \"%s\"\n", azArg[1]);
            rc = 1;
            goto meta_command_exit;
          }
        }
      }
      if( iLimit<0 ){
        fprintf(stderr, "unknown limit: \"%s\"\n"
                        "enter \".limits\" with no arguments for a list.\n",
                         azArg[1]);
        rc = 1;
        goto meta_command_exit;
      }
      if( nArg==3 ){
        sqlite3_limit(p->db, aLimit[iLimit].limitCode,
                      (int)integerValue(azArg[2]));
      }
      printf("%20s %d\n", aLimit[iLimit].zLimitName,
             sqlite3_limit(p->db, aLimit[iLimit].limitCode, -1));
    }
  }else

#ifndef SQLITE_OMIT_LOAD_EXTENSION
  if( c=='l' && strncmp(azArg[0], "load", n)==0 ){
    const char *zFile, *zProc;
    char *zErrMsg = 0;
    if( nArg<2 ){
      fprintf(stderr, "Usage: .load FILE ?ENTRYPOINT?\n");
      rc = 1;
      goto meta_command_exit;
    }
    zFile = azArg[1];
    zProc = nArg>=3 ? azArg[2] : 0;
    open_db(p, 0);
    rc = sqlite3_load_extension(p->db, zFile, zProc, &zErrMsg);
    if( rc!=SQLITE_OK ){
      fprintf(stderr, "Error: %s\n", zErrMsg);
      sqlite3_free(zErrMsg);
      rc = 1;
    }
  }else
#endif

  if( c=='l' && strncmp(azArg[0], "log", n)==0 ){
    if( nArg!=2 ){
      fprintf(stderr, "Usage: .log FILENAME\n");
      rc = 1;
    }else{
      const char *zFile = azArg[1];
      output_file_close(p->pLog);
      p->pLog = output_file_open(zFile);
    }
  }else

  if( c=='m' && strncmp(azArg[0], "mode", n)==0 ){
    const char *zMode = nArg>=2 ? azArg[1] : "";
    int n2 = (int)strlen(zMode);
    int c2 = zMode[0];
    if( c2=='l' && n2>2 && strncmp(azArg[1],"lines",n2)==0 ){
      p->mode = MODE_Line;
    }else if( c2=='c' && strncmp(azArg[1],"columns",n2)==0 ){
      p->mode = MODE_Column;
    }else if( c2=='l' && n2>2 && strncmp(azArg[1],"list",n2)==0 ){
      p->mode = MODE_List;
    }else if( c2=='h' && strncmp(azArg[1],"html",n2)==0 ){
      p->mode = MODE_Html;
    }else if( c2=='t' && strncmp(azArg[1],"tcl",n2)==0 ){
      p->mode = MODE_Tcl;
      sqlite3_snprintf(sizeof(p->colSeparator), p->colSeparator, SEP_Space);
    }else if( c2=='c' && strncmp(azArg[1],"csv",n2)==0 ){
      p->mode = MODE_Csv;
      sqlite3_snprintf(sizeof(p->colSeparator), p->colSeparator, SEP_Comma);
      sqlite3_snprintf(sizeof(p->rowSeparator), p->rowSeparator, SEP_CrLf);
    }else if( c2=='t' && strncmp(azArg[1],"tabs",n2)==0 ){
      p->mode = MODE_List;
      sqlite3_snprintf(sizeof(p->colSeparator), p->colSeparator, SEP_Tab);
    }else if( c2=='i' && strncmp(azArg[1],"insert",n2)==0 ){
      p->mode = MODE_Insert;
      set_table_name(p, nArg>=3 ? azArg[2] : "table");
    }else if( c2=='a' && strncmp(azArg[1],"ascii",n2)==0 ){
      p->mode = MODE_Ascii;
      sqlite3_snprintf(sizeof(p->colSeparator), p->colSeparator, SEP_Unit);
      sqlite3_snprintf(sizeof(p->rowSeparator), p->rowSeparator, SEP_Record);
    }else {
      fprintf(stderr,"Error: mode should be one of: "
         "ascii column csv html insert line list tabs tcl\n");
      rc = 1;
    }
  }else

  if( c=='n' && strncmp(azArg[0], "nullvalue", n)==0 ){
    if( nArg==2 ){
      sqlite3_snprintf(sizeof(p->nullValue), p->nullValue,
                       "%.*s", (int)ArraySize(p->nullValue)-1, azArg[1]);
    }else{
      fprintf(stderr, "Usage: .nullvalue STRING\n");
      rc = 1;
    }
  }else

  if( c=='o' && strncmp(azArg[0], "open", n)==0 && n>=2 ){
    sqlite3 *savedDb = p->db;
    const char *zSavedFilename = p->zDbFilename;
    char *zNewFilename = 0;
    p->db = 0;
    if( nArg>=2 ){
      p->zDbFilename = zNewFilename = sqlite3_mprintf("%s", azArg[1]);
    }
    open_db(p, 1);
    if( p->db!=0 ){
      sqlite3_close(savedDb);
      sqlite3_free(p->zFreeOnClose);
      p->zFreeOnClose = zNewFilename;
    }else{
      sqlite3_free(zNewFilename);
      p->db = savedDb;
      p->zDbFilename = zSavedFilename;
    }
  }else

  if( c=='o'
   && (strncmp(azArg[0], "output", n)==0 || strncmp(azArg[0], "once", n)==0)
  ){
    const char *zFile = nArg>=2 ? azArg[1] : "stdout";
    if( nArg>2 ){
      fprintf(stderr, "Usage: .%s FILE\n", azArg[0]);
      rc = 1;
      goto meta_command_exit;
    }
    if( n>1 && strncmp(azArg[0], "once", n)==0 ){
      if( nArg<2 ){
        fprintf(stderr, "Usage: .once FILE\n");
        rc = 1;
        goto meta_command_exit;
      }
      p->outCount = 2;
    }else{
      p->outCount = 0;
    }
    output_reset(p);
    if( zFile[0]=='|' ){
#ifdef SQLITE_OMIT_POPEN
      fprintf(stderr,"Error: pipes are not supported in this OS\n");
      rc = 1;
      p->out = stdout;
#else
      p->out = popen(zFile + 1, "w");
      if( p->out==0 ){
        fprintf(stderr,"Error: cannot open pipe \"%s\"\n", zFile + 1);
        p->out = stdout;
        rc = 1;
      }else{
        sqlite3_snprintf(sizeof(p->outfile), p->outfile, "%s", zFile);
      }
#endif
    }else{
      p->out = output_file_open(zFile);
      if( p->out==0 ){
        if( strcmp(zFile,"off")!=0 ){
          fprintf(stderr,"Error: cannot write to \"%s\"\n", zFile);
        }
        p->out = stdout;
        rc = 1;
      } else {
        sqlite3_snprintf(sizeof(p->outfile), p->outfile, "%s", zFile);
      }
    }
  }else

  if( c=='p' && n>=3 && strncmp(azArg[0], "print", n)==0 ){
    int i;
    for(i=1; i<nArg; i++){
      if( i>1 ) fprintf(p->out, " ");
      fprintf(p->out, "%s", azArg[i]);
    }
    fprintf(p->out, "\n");
  }else

  if( c=='p' && strncmp(azArg[0], "prompt", n)==0 ){
    if( nArg >= 2) {
      strncpy(mainPrompt,azArg[1],(int)ArraySize(mainPrompt)-1);
    }
    if( nArg >= 3) {
      strncpy(continuePrompt,azArg[2],(int)ArraySize(continuePrompt)-1);
    }
  }else

  if( c=='q' && strncmp(azArg[0], "quit", n)==0 ){
    rc = 2;
  }else

  if( c=='r' && n>=3 && strncmp(azArg[0], "read", n)==0 ){
    FILE *alt;
    if( nArg!=2 ){
      fprintf(stderr, "Usage: .read FILE\n");
      rc = 1;
      goto meta_command_exit;
    }
    alt = fopen(azArg[1], "rb");
    if( alt==0 ){
      fprintf(stderr,"Error: cannot open \"%s\"\n", azArg[1]);
      rc = 1;
    }else{
      rc = process_input(p, alt);
      fclose(alt);
    }
  }else

  if( c=='r' && n>=3 && strncmp(azArg[0], "restore", n)==0 ){
    const char *zSrcFile;
    const char *zDb;
    sqlite3 *pSrc;
    sqlite3_backup *pBackup;
    int nTimeout = 0;

    if( nArg==2 ){
      zSrcFile = azArg[1];
      zDb = "main";
    }else if( nArg==3 ){
      zSrcFile = azArg[2];
      zDb = azArg[1];
    }else{
      fprintf(stderr, "Usage: .restore ?DB? FILE\n");
      rc = 1;
      goto meta_command_exit;
    }
    rc = sqlite3_open(zSrcFile, &pSrc);
    if( rc!=SQLITE_OK ){
      fprintf(stderr, "Error: cannot open \"%s\"\n", zSrcFile);
      sqlite3_close(pSrc);
      return 1;
    }
    open_db(p, 0);
    pBackup = sqlite3_backup_init(p->db, zDb, pSrc, "main");
    if( pBackup==0 ){
      fprintf(stderr, "Error: %s\n", sqlite3_errmsg(p->db));
      sqlite3_close(pSrc);
      return 1;
    }
    while( (rc = sqlite3_backup_step(pBackup,100))==SQLITE_OK
          || rc==SQLITE_BUSY  ){
      if( rc==SQLITE_BUSY ){
        if( nTimeout++ >= 3 ) break;
        sqlite3_sleep(100);
      }
    }
    sqlite3_backup_finish(pBackup);
    if( rc==SQLITE_DONE ){
      rc = 0;
    }else if( rc==SQLITE_BUSY || rc==SQLITE_LOCKED ){
      fprintf(stderr, "Error: source database is busy\n");
      rc = 1;
    }else{
      fprintf(stderr, "Error: %s\n", sqlite3_errmsg(p->db));
      rc = 1;
    }
    sqlite3_close(pSrc);
  }else


  if( c=='s' && strncmp(azArg[0], "scanstats", n)==0 ){
    if( nArg==2 ){
      p->scanstatsOn = booleanValue(azArg[1]);
#ifndef SQLITE_ENABLE_STMT_SCANSTATUS
      fprintf(stderr, "Warning: .scanstats not available in this build.\n");
#endif
    }else{
      fprintf(stderr, "Usage: .scanstats on|off\n");
      rc = 1;
    }
  }else

  if( c=='s' && strncmp(azArg[0], "schema", n)==0 ){
    ShellState data;
    char *zErrMsg = 0;
    open_db(p, 0);
    memcpy(&data, p, sizeof(data));
    data.showHeader = 0;
    data.mode = MODE_Semi;
    if( nArg==2 ){
      int i;
      for(i=0; azArg[1][i]; i++) azArg[1][i] = ToLower(azArg[1][i]);
      if( strcmp(azArg[1],"sqlite_master")==0 ){
        char *new_argv[2], *new_colv[2];
        new_argv[0] = "CREATE TABLE sqlite_master (\n"
                      "  type text,\n"
                      "  name text,\n"
                      "  tbl_name text,\n"
                      "  rootpage integer,\n"
                      "  sql text\n"
                      ")";
        new_argv[1] = 0;
        new_colv[0] = "sql";
        new_colv[1] = 0;
        callback(&data, 1, new_argv, new_colv);
        rc = SQLITE_OK;
      }else if( strcmp(azArg[1],"sqlite_temp_master")==0 ){
        char *new_argv[2], *new_colv[2];
        new_argv[0] = "CREATE TEMP TABLE sqlite_temp_master (\n"
                      "  type text,\n"
                      "  name text,\n"
                      "  tbl_name text,\n"
                      "  rootpage integer,\n"
                      "  sql text\n"
                      ")";
        new_argv[1] = 0;
        new_colv[0] = "sql";
        new_colv[1] = 0;
        callback(&data, 1, new_argv, new_colv);
        rc = SQLITE_OK;
      }else{
        zShellStatic = azArg[1];
        rc = sqlite3_exec(p->db,
          "SELECT sql FROM "
          "  (SELECT sql sql, type type, tbl_name tbl_name, name name, rowid x"
          "     FROM sqlite_master UNION ALL"
          "   SELECT sql, type, tbl_name, name, rowid FROM sqlite_temp_master) "
          "WHERE lower(tbl_name) LIKE shellstatic()"
          "  AND type!='meta' AND sql NOTNULL "
          "ORDER BY rowid",
          callback, &data, &zErrMsg);
        zShellStatic = 0;
      }
    }else if( nArg==1 ){
      rc = sqlite3_exec(p->db,
         "SELECT sql FROM "
         "  (SELECT sql sql, type type, tbl_name tbl_name, name name, rowid x"
         "     FROM sqlite_master UNION ALL"
         "   SELECT sql, type, tbl_name, name, rowid FROM sqlite_temp_master) "
         "WHERE type!='meta' AND sql NOTNULL AND name NOT LIKE 'sqlite_%' "
         "ORDER BY rowid",
         callback, &data, &zErrMsg
      );
    }else{
      fprintf(stderr, "Usage: .schema ?LIKE-PATTERN?\n");
      rc = 1;
      goto meta_command_exit;
    }
    if( zErrMsg ){
      fprintf(stderr,"Error: %s\n", zErrMsg);
      sqlite3_free(zErrMsg);
      rc = 1;
    }else if( rc != SQLITE_OK ){
      fprintf(stderr,"Error: querying schema information\n");
      rc = 1;
    }else{
      rc = 0;
    }
  }else


#if defined(SQLITE_DEBUG) && defined(SQLITE_ENABLE_SELECTTRACE)
  if( c=='s' && n==11 && strncmp(azArg[0], "selecttrace", n)==0 ){
    extern int sqlite3SelectTrace;
    sqlite3SelectTrace = integerValue(azArg[1]);
  }else
#endif


#ifdef SQLITE_DEBUG
  /* Undocumented commands for internal testing.  Subject to change
  ** without notice. */
  if( c=='s' && n>=10 && strncmp(azArg[0], "selftest-", 9)==0 ){
    if( strncmp(azArg[0]+9, "boolean", n-9)==0 ){
      int i, v;
      for(i=1; i<nArg; i++){
        v = booleanValue(azArg[i]);
        fprintf(p->out, "%s: %d 0x%x\n", azArg[i], v, v);
      }
    }
    if( strncmp(azArg[0]+9, "integer", n-9)==0 ){
      int i; sqlite3_int64 v;
      for(i=1; i<nArg; i++){
        char zBuf[200];
        v = integerValue(azArg[i]);
        sqlite3_snprintf(sizeof(zBuf),zBuf,"%s: %lld 0x%llx\n", azArg[i],v,v);
        fprintf(p->out, "%s", zBuf);
      }
    }
  }else
#endif

  if( c=='s' && strncmp(azArg[0], "separator", n)==0 ){
    if( nArg<2 || nArg>3 ){
      fprintf(stderr, "Usage: .separator COL ?ROW?\n");
      rc = 1;
    }
    if( nArg>=2 ){
      sqlite3_snprintf(sizeof(p->colSeparator), p->colSeparator,
                       "%.*s", (int)ArraySize(p->colSeparator)-1, azArg[1]);
    }
    if( nArg>=3 ){
      sqlite3_snprintf(sizeof(p->rowSeparator), p->rowSeparator,
                       "%.*s", (int)ArraySize(p->rowSeparator)-1, azArg[2]);
    }
  }else

  if( c=='s'
   && (strncmp(azArg[0], "shell", n)==0 || strncmp(azArg[0],"system",n)==0)
  ){
    char *zCmd;
    int i, x;
    if( nArg<2 ){
      fprintf(stderr, "Usage: .system COMMAND\n");
      rc = 1;
      goto meta_command_exit;
    }
    zCmd = sqlite3_mprintf(strchr(azArg[1],' ')==0?"%s":"\"%s\"", azArg[1]);
    for(i=2; i<nArg; i++){
      zCmd = sqlite3_mprintf(strchr(azArg[i],' ')==0?"%z %s":"%z \"%s\"",
                             zCmd, azArg[i]);
    }
    x = system(zCmd);
    sqlite3_free(zCmd);
    if( x ) fprintf(stderr, "System command returns %d\n", x);
  }else

  if( c=='s' && strncmp(azArg[0], "show", n)==0 ){
    int i;
    if( nArg!=1 ){
      fprintf(stderr, "Usage: .show\n");
      rc = 1;
      goto meta_command_exit;
    }
    fprintf(p->out,"%12.12s: %s\n","echo", p->echoOn ? "on" : "off");
    fprintf(p->out,"%12.12s: %s\n","eqp", p->autoEQP ? "on" : "off");
    fprintf(p->out,"%9.9s: %s\n","explain", p->normalMode.valid ? "on" :"off");
    fprintf(p->out,"%12.12s: %s\n","headers", p->showHeader ? "on" : "off");
    fprintf(p->out,"%12.12s: %s\n","mode", modeDescr[p->mode]);
    fprintf(p->out,"%12.12s: ", "nullvalue");
      output_c_string(p->out, p->nullValue);
      fprintf(p->out, "\n");
    fprintf(p->out,"%12.12s: %s\n","output",
            strlen30(p->outfile) ? p->outfile : "stdout");
    fprintf(p->out,"%12.12s: ", "colseparator");
      output_c_string(p->out, p->colSeparator);
      fprintf(p->out, "\n");
    fprintf(p->out,"%12.12s: ", "rowseparator");
      output_c_string(p->out, p->rowSeparator);
      fprintf(p->out, "\n");
    fprintf(p->out,"%12.12s: %s\n","stats", p->statsOn ? "on" : "off");
    fprintf(p->out,"%12.12s: ","width");
    for (i=0;i<(int)ArraySize(p->colWidth) && p->colWidth[i] != 0;i++) {
      fprintf(p->out,"%d ",p->colWidth[i]);
    }
    fprintf(p->out,"\n");
  }else

  if( c=='s' && strncmp(azArg[0], "stats", n)==0 ){
    if( nArg==2 ){
      p->statsOn = booleanValue(azArg[1]);
    }else{
      fprintf(stderr, "Usage: .stats on|off\n");
      rc = 1;
    }
  }else

  if( c=='t' && n>1 && strncmp(azArg[0], "tables", n)==0 ){
    sqlite3_stmt *pStmt;
    char **azResult;
    int nRow, nAlloc;
    char *zSql = 0;
    int ii;
    open_db(p, 0);
    rc = sqlite3_prepare_v2(p->db, "PRAGMA database_list", -1, &pStmt, 0);
    if( rc ) return rc;
    zSql = sqlite3_mprintf(
        "SELECT name FROM sqlite_master"
        " WHERE type IN ('table','view')"
        "   AND name NOT LIKE 'sqlite_%%'"
        "   AND name LIKE ?1");
    while( sqlite3_step(pStmt)==SQLITE_ROW ){
      const char *zDbName = (const char*)sqlite3_column_text(pStmt, 1);
      if( zDbName==0 || strcmp(zDbName,"main")==0 ) continue;
      if( strcmp(zDbName,"temp")==0 ){
        zSql = sqlite3_mprintf(
                 "%z UNION ALL "
                 "SELECT 'temp.' || name FROM sqlite_temp_master"
                 " WHERE type IN ('table','view')"
                 "   AND name NOT LIKE 'sqlite_%%'"
                 "   AND name LIKE ?1", zSql);
      }else{
        zSql = sqlite3_mprintf(
                 "%z UNION ALL "
                 "SELECT '%q.' || name FROM \"%w\".sqlite_master"
                 " WHERE type IN ('table','view')"
                 "   AND name NOT LIKE 'sqlite_%%'"
                 "   AND name LIKE ?1", zSql, zDbName, zDbName);
      }
    }
    sqlite3_finalize(pStmt);
    zSql = sqlite3_mprintf("%z ORDER BY 1", zSql);
    rc = sqlite3_prepare_v2(p->db, zSql, -1, &pStmt, 0);
    sqlite3_free(zSql);
    if( rc ) return rc;
    nRow = nAlloc = 0;
    azResult = 0;
    if( nArg>1 ){
      sqlite3_bind_text(pStmt, 1, azArg[1], -1, SQLITE_TRANSIENT);
    }else{
      sqlite3_bind_text(pStmt, 1, "%", -1, SQLITE_STATIC);
    }
    while( sqlite3_step(pStmt)==SQLITE_ROW ){
      if( nRow>=nAlloc ){
        char **azNew;
        int n2 = nAlloc*2 + 10;
        azNew = sqlite3_realloc64(azResult, sizeof(azResult[0])*n2);
        if( azNew==0 ){
          fprintf(stderr, "Error: out of memory\n");
          break;
        }
        nAlloc = n2;
        azResult = azNew;
      }
      azResult[nRow] = sqlite3_mprintf("%s", sqlite3_column_text(pStmt, 0));
      if( azResult[nRow] ) nRow++;
    }
    sqlite3_finalize(pStmt);        
    if( nRow>0 ){
      int len, maxlen = 0;
      int i, j;
      int nPrintCol, nPrintRow;
      for(i=0; i<nRow; i++){
        len = strlen30(azResult[i]);
        if( len>maxlen ) maxlen = len;
      }
      nPrintCol = 80/(maxlen+2);
      if( nPrintCol<1 ) nPrintCol = 1;
      nPrintRow = (nRow + nPrintCol - 1)/nPrintCol;
      for(i=0; i<nPrintRow; i++){
        for(j=i; j<nRow; j+=nPrintRow){
          char *zSp = j<nPrintRow ? "" : "  ";
          fprintf(p->out, "%s%-*s", zSp, maxlen, azResult[j] ? azResult[j]:"");
        }
        fprintf(p->out, "\n");
      }
    }
    for(ii=0; ii<nRow; ii++) sqlite3_free(azResult[ii]);
    sqlite3_free(azResult);
  }else

  if( c=='t' && n>=8 && strncmp(azArg[0], "testctrl", n)==0 && nArg>=2 ){
    static const struct {
       const char *zCtrlName;   /* Name of a test-control option */
       int ctrlCode;            /* Integer code for that option */
    } aCtrl[] = {
      { "prng_save",             SQLITE_TESTCTRL_PRNG_SAVE              },
      { "prng_restore",          SQLITE_TESTCTRL_PRNG_RESTORE           },
      { "prng_reset",            SQLITE_TESTCTRL_PRNG_RESET             },
      { "bitvec_test",           SQLITE_TESTCTRL_BITVEC_TEST            },
      { "fault_install",         SQLITE_TESTCTRL_FAULT_INSTALL          },
      { "benign_malloc_hooks",   SQLITE_TESTCTRL_BENIGN_MALLOC_HOOKS    },
      { "pending_byte",          SQLITE_TESTCTRL_PENDING_BYTE           },
      { "assert",                SQLITE_TESTCTRL_ASSERT                 },
      { "always",                SQLITE_TESTCTRL_ALWAYS                 },
      { "reserve",               SQLITE_TESTCTRL_RESERVE                },
      { "optimizations",         SQLITE_TESTCTRL_OPTIMIZATIONS          },
      { "iskeyword",             SQLITE_TESTCTRL_ISKEYWORD              },
      { "scratchmalloc",         SQLITE_TESTCTRL_SCRATCHMALLOC          },
      { "byteorder",             SQLITE_TESTCTRL_BYTEORDER              },
      { "never_corrupt",         SQLITE_TESTCTRL_NEVER_CORRUPT          },
      { "imposter",              SQLITE_TESTCTRL_IMPOSTER               },
    };
    int testctrl = -1;
    int rc2 = 0;
    int i, n2;
    open_db(p, 0);

    /* convert testctrl text option to value. allow any unique prefix
    ** of the option name, or a numerical value. */
    n2 = strlen30(azArg[1]);
    for(i=0; i<(int)(sizeof(aCtrl)/sizeof(aCtrl[0])); i++){
      if( strncmp(azArg[1], aCtrl[i].zCtrlName, n2)==0 ){
        if( testctrl<0 ){
          testctrl = aCtrl[i].ctrlCode;
        }else{
          fprintf(stderr, "ambiguous option name: \"%s\"\n", azArg[1]);
          testctrl = -1;
          break;
        }
      }
    }
    if( testctrl<0 ) testctrl = (int)integerValue(azArg[1]);
    if( (testctrl<SQLITE_TESTCTRL_FIRST) || (testctrl>SQLITE_TESTCTRL_LAST) ){
      fprintf(stderr,"Error: invalid testctrl option: %s\n", azArg[1]);
    }else{
      switch(testctrl){

        /* sqlite3_test_control(int, db, int) */
        case SQLITE_TESTCTRL_OPTIMIZATIONS:
        case SQLITE_TESTCTRL_RESERVE:             
          if( nArg==3 ){
            int opt = (int)strtol(azArg[2], 0, 0);        
            rc2 = sqlite3_test_control(testctrl, p->db, opt);
            fprintf(p->out, "%d (0x%08x)\n", rc2, rc2);
          } else {
            fprintf(stderr,"Error: testctrl %s takes a single int option\n",
                    azArg[1]);
          }
          break;

        /* sqlite3_test_control(int) */
        case SQLITE_TESTCTRL_PRNG_SAVE:
        case SQLITE_TESTCTRL_PRNG_RESTORE:
        case SQLITE_TESTCTRL_PRNG_RESET:
        case SQLITE_TESTCTRL_BYTEORDER:
          if( nArg==2 ){
            rc2 = sqlite3_test_control(testctrl);
            fprintf(p->out, "%d (0x%08x)\n", rc2, rc2);
          } else {
            fprintf(stderr,"Error: testctrl %s takes no options\n", azArg[1]);
          }
          break;

        /* sqlite3_test_control(int, uint) */
        case SQLITE_TESTCTRL_PENDING_BYTE:        
          if( nArg==3 ){
            unsigned int opt = (unsigned int)integerValue(azArg[2]);
            rc2 = sqlite3_test_control(testctrl, opt);
            fprintf(p->out, "%d (0x%08x)\n", rc2, rc2);
          } else {
            fprintf(stderr,"Error: testctrl %s takes a single unsigned"
                           " int option\n", azArg[1]);
          }
          break;
          
        /* sqlite3_test_control(int, int) */
        case SQLITE_TESTCTRL_ASSERT:              
        case SQLITE_TESTCTRL_ALWAYS:      
        case SQLITE_TESTCTRL_NEVER_CORRUPT:        
          if( nArg==3 ){
            int opt = booleanValue(azArg[2]);        
            rc2 = sqlite3_test_control(testctrl, opt);
            fprintf(p->out, "%d (0x%08x)\n", rc2, rc2);
          } else {
            fprintf(stderr,"Error: testctrl %s takes a single int option\n",
                            azArg[1]);
          }
          break;

        /* sqlite3_test_control(int, char *) */
#ifdef SQLITE_N_KEYWORD
        case SQLITE_TESTCTRL_ISKEYWORD:           
          if( nArg==3 ){
            const char *opt = azArg[2];        
            rc2 = sqlite3_test_control(testctrl, opt);
            fprintf(p->out, "%d (0x%08x)\n", rc2, rc2);
          } else {
            fprintf(stderr,"Error: testctrl %s takes a single char * option\n",
                            azArg[1]);
          }
          break;
#endif

        case SQLITE_TESTCTRL_IMPOSTER:
          if( nArg==5 ){
            rc2 = sqlite3_test_control(testctrl, p->db, 
                          azArg[2],
                          integerValue(azArg[3]),
                          integerValue(azArg[4]));
            fprintf(p->out, "%d (0x%08x)\n", rc2, rc2);
          }else{
            fprintf(stderr,"Usage: .testctrl imposter dbName onoff tnum\n");
          }
          break;

        case SQLITE_TESTCTRL_BITVEC_TEST:         
        case SQLITE_TESTCTRL_FAULT_INSTALL:       
        case SQLITE_TESTCTRL_BENIGN_MALLOC_HOOKS: 
        case SQLITE_TESTCTRL_SCRATCHMALLOC:       
        default:
          fprintf(stderr,"Error: CLI support for testctrl %s not implemented\n",
                  azArg[1]);
          break;
      }
    }
  }else

  if( c=='t' && n>4 && strncmp(azArg[0], "timeout", n)==0 ){
    open_db(p, 0);
    sqlite3_busy_timeout(p->db, nArg>=2 ? (int)integerValue(azArg[1]) : 0);
  }else
    
  if( c=='t' && n>=5 && strncmp(azArg[0], "timer", n)==0 ){
    if( nArg==2 ){
      enableTimer = booleanValue(azArg[1]);
      if( enableTimer && !HAS_TIMER ){
        fprintf(stderr, "Error: timer not available on this system.\n");
        enableTimer = 0;
      }
    }else{
      fprintf(stderr, "Usage: .timer on|off\n");
      rc = 1;
    }
  }else
  
  if( c=='t' && strncmp(azArg[0], "trace", n)==0 ){
    open_db(p, 0);
    if( nArg!=2 ){
      fprintf(stderr, "Usage: .trace FILE|off\n");
      rc = 1;
      goto meta_command_exit;
    }
    output_file_close(p->traceOut);
    p->traceOut = output_file_open(azArg[1]);
#if !defined(SQLITE_OMIT_TRACE) && !defined(SQLITE_OMIT_FLOATING_POINT)
    if( p->traceOut==0 ){
      sqlite3_trace(p->db, 0, 0);
    }else{
      sqlite3_trace(p->db, sql_trace_callback, p->traceOut);
    }
#endif
  }else

#if SQLITE_USER_AUTHENTICATION
  if( c=='u' && strncmp(azArg[0], "user", n)==0 ){
    if( nArg<2 ){
      fprintf(stderr, "Usage: .user SUBCOMMAND ...\n");
      rc = 1;
      goto meta_command_exit;
    }
    open_db(p, 0);
    if( strcmp(azArg[1],"login")==0 ){
      if( nArg!=4 ){
        fprintf(stderr, "Usage: .user login USER PASSWORD\n");
        rc = 1;
        goto meta_command_exit;
      }
      rc = sqlite3_user_authenticate(p->db, azArg[2], azArg[3],
                                    (int)strlen(azArg[3]));
      if( rc ){
        fprintf(stderr, "Authentication failed for user %s\n", azArg[2]);
        rc = 1;
      }
    }else if( strcmp(azArg[1],"add")==0 ){
      if( nArg!=5 ){
        fprintf(stderr, "Usage: .user add USER PASSWORD ISADMIN\n");
        rc = 1;
        goto meta_command_exit;
      }
      rc = sqlite3_user_add(p->db, azArg[2],
                            azArg[3], (int)strlen(azArg[3]),
                            booleanValue(azArg[4]));
      if( rc ){
        fprintf(stderr, "User-Add failed: %d\n", rc);
        rc = 1;
      }
    }else if( strcmp(azArg[1],"edit")==0 ){
      if( nArg!=5 ){
        fprintf(stderr, "Usage: .user edit USER PASSWORD ISADMIN\n");
        rc = 1;
        goto meta_command_exit;
      }
      rc = sqlite3_user_change(p->db, azArg[2],
                              azArg[3], (int)strlen(azArg[3]),
                              booleanValue(azArg[4]));
      if( rc ){
        fprintf(stderr, "User-Edit failed: %d\n", rc);
        rc = 1;
      }
    }else if( strcmp(azArg[1],"delete")==0 ){
      if( nArg!=3 ){
        fprintf(stderr, "Usage: .user delete USER\n");
        rc = 1;
        goto meta_command_exit;
      }
      rc = sqlite3_user_delete(p->db, azArg[2]);
      if( rc ){
        fprintf(stderr, "User-Delete failed: %d\n", rc);
        rc = 1;
      }
    }else{
      fprintf(stderr, "Usage: .user login|add|edit|delete ...\n");
      rc = 1;
      goto meta_command_exit;
    }    
  }else
#endif /* SQLITE_USER_AUTHENTICATION */

  if( c=='v' && strncmp(azArg[0], "version", n)==0 ){
    fprintf(p->out, "SQLite %s %s\n" /*extra-version-info*/,
        sqlite3_libversion(), sqlite3_sourceid());
  }else

  if( c=='v' && strncmp(azArg[0], "vfsname", n)==0 ){
    const char *zDbName = nArg==2 ? azArg[1] : "main";
    char *zVfsName = 0;
    if( p->db ){
      sqlite3_file_control(p->db, zDbName, SQLITE_FCNTL_VFSNAME, &zVfsName);
      if( zVfsName ){
        fprintf(p->out, "%s\n", zVfsName);
        sqlite3_free(zVfsName);
      }
    }
  }else

#if defined(SQLITE_DEBUG) && defined(SQLITE_ENABLE_WHERETRACE)
  if( c=='w' && strncmp(azArg[0], "wheretrace", n)==0 ){
    extern int sqlite3WhereTrace;
    sqlite3WhereTrace = nArg>=2 ? booleanValue(azArg[1]) : 0xff;
  }else
#endif

  if( c=='w' && strncmp(azArg[0], "width", n)==0 ){
    int j;
    assert( nArg<=ArraySize(azArg) );
    for(j=1; j<nArg && j<ArraySize(p->colWidth); j++){
      p->colWidth[j-1] = (int)integerValue(azArg[j]);
    }
  }else

  {
    fprintf(stderr, "Error: unknown command or invalid arguments: "
      " \"%s\". Enter \".help\" for help\n", azArg[0]);
    rc = 1;
  }

meta_command_exit:
  if( p->outCount ){
    p->outCount--;
    if( p->outCount==0 ) output_reset(p);
  }
  return rc;
}

/*
** Return TRUE if a semicolon occurs anywhere in the first N characters
** of string z[].
*/
static int line_contains_semicolon(const char *z, int N){
  int i;
  for(i=0; i<N; i++){  if( z[i]==';' ) return 1; }
  return 0;
}

/*
** Test to see if a line consists entirely of whitespace.
*/
static int _all_whitespace(const char *z){
  for(; *z; z++){
    if( IsSpace(z[0]) ) continue;
    if( *z=='/' && z[1]=='*' ){
      z += 2;
      while( *z && (*z!='*' || z[1]!='/') ){ z++; }
      if( *z==0 ) return 0;
      z++;
      continue;
    }
    if( *z=='-' && z[1]=='-' ){
      z += 2;
      while( *z && *z!='\n' ){ z++; }
      if( *z==0 ) return 1;
      continue;
    }
    return 0;
  }
  return 1;
}

/*
** Return TRUE if the line typed in is an SQL command terminator other
** than a semi-colon.  The SQL Server style "go" command is understood
** as is the Oracle "/".
*/
static int line_is_command_terminator(const char *zLine){
  while( IsSpace(zLine[0]) ){ zLine++; };
  if( zLine[0]=='/' && _all_whitespace(&zLine[1]) ){
    return 1;  /* Oracle */
  }
  if( ToLower(zLine[0])=='g' && ToLower(zLine[1])=='o'
         && _all_whitespace(&zLine[2]) ){
    return 1;  /* SQL Server */
  }
  return 0;
}

/*
** Return true if zSql is a complete SQL statement.  Return false if it
** ends in the middle of a string literal or C-style comment.
*/
static int line_is_complete(char *zSql, int nSql){
  int rc;
  if( zSql==0 ) return 1;
  zSql[nSql] = ';';
  zSql[nSql+1] = 0;
  rc = sqlite3_complete(zSql);
  zSql[nSql] = 0;
  return rc;
}

/*
** Read input from *in and process it.  If *in==0 then input
** is interactive - the user is typing it it.  Otherwise, input
** is coming from a file or device.  A prompt is issued and history
** is saved only if input is interactive.  An interrupt signal will
** cause this routine to exit immediately, unless input is interactive.
**
** Return the number of errors.
*/
static int process_input(ShellState *p, FILE *in){
  char *zLine = 0;          /* A single input line */
  char *zSql = 0;           /* Accumulated SQL text */
  int nLine;                /* Length of current line */
  int nSql = 0;             /* Bytes of zSql[] used */
  int nAlloc = 0;           /* Allocated zSql[] space */
  int nSqlPrior = 0;        /* Bytes of zSql[] used by prior line */
  char *zErrMsg;            /* Error message returned */
  int rc;                   /* Error code */
  int errCnt = 0;           /* Number of errors seen */
  int lineno = 0;           /* Current line number */
  int startline = 0;        /* Line number for start of current input */

  while( errCnt==0 || !bail_on_error || (in==0 && stdin_is_interactive) ){
    fflush(p->out);
    zLine = one_input_line(in, zLine, nSql>0);
    if( zLine==0 ){
      /* End of input */
      if( stdin_is_interactive ) printf("\n");
      break;
    }
    if( seenInterrupt ){
      if( in!=0 ) break;
      seenInterrupt = 0;
    }
    lineno++;
    if( nSql==0 && _all_whitespace(zLine) ){
      if( p->echoOn ) printf("%s\n", zLine);
      continue;
    }
    if( zLine && zLine[0]=='.' && nSql==0 ){
      if( p->echoOn ) printf("%s\n", zLine);
      rc = do_meta_command(zLine, p);
      if( rc==2 ){ /* exit requested */
        break;
      }else if( rc ){
        errCnt++;
      }
      continue;
    }
    if( line_is_command_terminator(zLine) && line_is_complete(zSql, nSql) ){
      memcpy(zLine,";",2);
    }
    nLine = strlen30(zLine);
    if( nSql+nLine+2>=nAlloc ){
      nAlloc = nSql+nLine+100;
      zSql = realloc(zSql, nAlloc);
      if( zSql==0 ){
        fprintf(stderr, "Error: out of memory\n");
        exit(1);
      }
    }
    nSqlPrior = nSql;
    if( nSql==0 ){
      int i;
      for(i=0; zLine[i] && IsSpace(zLine[i]); i++){}
      assert( nAlloc>0 && zSql!=0 );
      memcpy(zSql, zLine+i, nLine+1-i);
      startline = lineno;
      nSql = nLine-i;
    }else{
      zSql[nSql++] = '\n';
      memcpy(zSql+nSql, zLine, nLine+1);
      nSql += nLine;
    }
    if( nSql && line_contains_semicolon(&zSql[nSqlPrior], nSql-nSqlPrior)
                && sqlite3_complete(zSql) ){
      p->cnt = 0;
      open_db(p, 0);
      if( p->backslashOn ) resolve_backslashes(zSql);
      BEGIN_TIMER;
      rc = shell_exec(p->db, zSql, shell_callback, p, &zErrMsg);
      END_TIMER;
      if( rc || zErrMsg ){
        char zPrefix[100];
        if( in!=0 || !stdin_is_interactive ){
          sqlite3_snprintf(sizeof(zPrefix), zPrefix, 
                           "Error: near line %d:", startline);
        }else{
          sqlite3_snprintf(sizeof(zPrefix), zPrefix, "Error:");
        }
        if( zErrMsg!=0 ){
          fprintf(stderr, "%s %s\n", zPrefix, zErrMsg);
          sqlite3_free(zErrMsg);
          zErrMsg = 0;
        }else{
          fprintf(stderr, "%s %s\n", zPrefix, sqlite3_errmsg(p->db));
        }
        errCnt++;
      }
      nSql = 0;
      if( p->outCount ){
        output_reset(p);
        p->outCount = 0;
      }
    }else if( nSql && _all_whitespace(zSql) ){
      if( p->echoOn ) printf("%s\n", zSql);
      nSql = 0;
    }
  }
  if( nSql ){
    if( !_all_whitespace(zSql) ){
      fprintf(stderr, "Error: incomplete SQL: %s\n", zSql);
      errCnt++;
    }
    free(zSql);
  }
  free(zLine);
  return errCnt>0;
}

/*
** Return a pathname which is the user's home directory.  A
** 0 return indicates an error of some kind.
*/
static char *find_home_dir(void){
  static char *home_dir = NULL;
  if( home_dir ) return home_dir;

#if !defined(_WIN32) && !defined(WIN32) && !defined(_WIN32_WCE) \
     && !defined(__RTP__) && !defined(_WRS_KERNEL)
  {
    struct passwd *pwent;
    uid_t uid = getuid();
    if( (pwent=getpwuid(uid)) != NULL) {
      home_dir = pwent->pw_dir;
    }
  }
#endif

#if defined(_WIN32_WCE)
  /* Windows CE (arm-wince-mingw32ce-gcc) does not provide getenv()
   */
  home_dir = "/";
#else

#if defined(_WIN32) || defined(WIN32)
  if (!home_dir) {
    home_dir = getenv("USERPROFILE");
  }
#endif

  if (!home_dir) {
    home_dir = getenv("HOME");
  }

#if defined(_WIN32) || defined(WIN32)
  if (!home_dir) {
    char *zDrive, *zPath;
    int n;
    zDrive = getenv("HOMEDRIVE");
    zPath = getenv("HOMEPATH");
    if( zDrive && zPath ){
      n = strlen30(zDrive) + strlen30(zPath) + 1;
      home_dir = malloc( n );
      if( home_dir==0 ) return 0;
      sqlite3_snprintf(n, home_dir, "%s%s", zDrive, zPath);
      return home_dir;
    }
    home_dir = "c:\\";
  }
#endif

#endif /* !_WIN32_WCE */

  if( home_dir ){
    int n = strlen30(home_dir) + 1;
    char *z = malloc( n );
    if( z ) memcpy(z, home_dir, n);
    home_dir = z;
  }

  return home_dir;
}

/*
** Read input from the file given by sqliterc_override.  Or if that
** parameter is NULL, take input from ~/.sqliterc
**
** Returns the number of errors.
*/
static void process_sqliterc(
  ShellState *p,                  /* Configuration data */
  const char *sqliterc_override   /* Name of config file. NULL to use default */
){
  char *home_dir = NULL;
  const char *sqliterc = sqliterc_override;
  char *zBuf = 0;
  FILE *in = NULL;

  if (sqliterc == NULL) {
    home_dir = find_home_dir();
    if( home_dir==0 ){
      fprintf(stderr, "-- warning: cannot find home directory;"
                      " cannot read ~/.sqliterc\n");
      return;
    }
    sqlite3_initialize();
    zBuf = sqlite3_mprintf("%s/.sqliterc",home_dir);
    sqliterc = zBuf;
  }
  in = fopen(sqliterc,"rb");
  if( in ){
    if( stdin_is_interactive ){
      fprintf(stderr,"-- Loading resources from %s\n",sqliterc);
    }
    process_input(p,in);
    fclose(in);
  }
  sqlite3_free(zBuf);
}

/*
** Show available command line options
*/
static const char zOptions[] = 
  "   -ascii               set output mode to 'ascii'\n"
  "   -bail                stop after hitting an error\n"
  "   -batch               force batch I/O\n"
  "   -column              set output mode to 'column'\n"
  "   -cmd COMMAND         run \"COMMAND\" before reading stdin\n"
  "   -csv                 set output mode to 'csv'\n"
  "   -echo                print commands before execution\n"
  "   -init FILENAME       read/process named file\n"
  "   -[no]header          turn headers on or off\n"
#if defined(SQLITE_ENABLE_MEMSYS3) || defined(SQLITE_ENABLE_MEMSYS5)
  "   -heap SIZE           Size of heap for memsys3 or memsys5\n"
#endif
  "   -help                show this message\n"
  "   -html                set output mode to HTML\n"
  "   -interactive         force interactive I/O\n"
  "   -line                set output mode to 'line'\n"
  "   -list                set output mode to 'list'\n"
  "   -lookaside SIZE N    use N entries of SZ bytes for lookaside memory\n"
  "   -mmap N              default mmap size set to N\n"
#ifdef SQLITE_ENABLE_MULTIPLEX
  "   -multiplex           enable the multiplexor VFS\n"
#endif
  "   -newline SEP         set output row separator. Default: '\\n'\n"
  "   -nullvalue TEXT      set text string for NULL values. Default ''\n"
  "   -pagecache SIZE N    use N slots of SZ bytes each for page cache memory\n"
  "   -scratch SIZE N      use N slots of SZ bytes each for scratch memory\n"
  "   -separator SEP       set output column separator. Default: '|'\n"
  "   -stats               print memory stats before each finalize\n"
  "   -version             show SQLite version\n"
  "   -vfs NAME            use NAME as the default VFS\n"
#ifdef SQLITE_ENABLE_VFSTRACE
  "   -vfstrace            enable tracing of all VFS calls\n"
#endif
;
static void usage(int showDetail){
  fprintf(stderr,
      "Usage: %s [OPTIONS] FILENAME [SQL]\n"  
      "FILENAME is the name of an SQLite database. A new database is created\n"
      "if the file does not previously exist.\n", Argv0);
  if( showDetail ){
    fprintf(stderr, "OPTIONS include:\n%s", zOptions);
  }else{
    fprintf(stderr, "Use the -help option for additional information\n");
  }
  exit(1);
}

/*
** Initialize the state information in data
*/
static void main_init(ShellState *data) {
  memset(data, 0, sizeof(*data));
  data->mode = MODE_List;
  memcpy(data->colSeparator,SEP_Column, 2);
  memcpy(data->rowSeparator,SEP_Row, 2);
  data->showHeader = 0;
  data->shellFlgs = SHFLG_Lookaside;
  sqlite3_config(SQLITE_CONFIG_URI, 1);
  sqlite3_config(SQLITE_CONFIG_LOG, shellLog, data);
  sqlite3_config(SQLITE_CONFIG_MULTITHREAD);
  sqlite3_snprintf(sizeof(mainPrompt), mainPrompt,"sqlite> ");
  sqlite3_snprintf(sizeof(continuePrompt), continuePrompt,"   ...> ");
}

/*
** Output text to the console in a font that attracts extra attention.
*/
#ifdef _WIN32
static void printBold(const char *zText){
  HANDLE out = GetStdHandle(STD_OUTPUT_HANDLE);
  CONSOLE_SCREEN_BUFFER_INFO defaultScreenInfo;
  GetConsoleScreenBufferInfo(out, &defaultScreenInfo);
  SetConsoleTextAttribute(out,
         FOREGROUND_RED|FOREGROUND_INTENSITY
  );
  printf("%s", zText);
  SetConsoleTextAttribute(out, defaultScreenInfo.wAttributes);
}
#else
static void printBold(const char *zText){
  printf("\033[1m%s\033[0m", zText);
}
#endif

/*
** Get the argument to an --option.  Throw an error and die if no argument
** is available.
*/
static char *cmdline_option_value(int argc, char **argv, int i){
  if( i==argc ){
    fprintf(stderr, "%s: Error: missing argument to %s\n",
            argv[0], argv[argc-1]);
    exit(1);
  }
  return argv[i];
}

int SQLITE_CDECL main(int argc, char **argv){
  char *zErrMsg = 0;
  ShellState data;
  const char *zInitFile = 0;
  int i;
  int rc = 0;
  int warnInmemoryDb = 0;
  int readStdin = 1;
  int nCmd = 0;
  char **azCmd = 0;

#if USE_SYSTEM_SQLITE+0!=1
  if( strcmp(sqlite3_sourceid(),SQLITE_SOURCE_ID)!=0 ){
    fprintf(stderr, "SQLite header and source version mismatch\n%s\n%s\n",
            sqlite3_sourceid(), SQLITE_SOURCE_ID);
    exit(1);
  }
#endif
  setBinaryMode(stdin);
  setvbuf(stderr, 0, _IONBF, 0); /* Make sure stderr is unbuffered */
  Argv0 = argv[0];
  main_init(&data);
  stdin_is_interactive = isatty(0);

  /* Make sure we have a valid signal handler early, before anything
  ** else is done.
  */
#ifdef SIGINT
  signal(SIGINT, interrupt_handler);
#endif

#ifdef SQLITE_SHELL_DBNAME_PROC
  {
    /* If the SQLITE_SHELL_DBNAME_PROC macro is defined, then it is the name
    ** of a C-function that will provide the name of the database file.  Use
    ** this compile-time option to embed this shell program in larger
    ** applications. */
    extern void SQLITE_SHELL_DBNAME_PROC(const char**);
    SQLITE_SHELL_DBNAME_PROC(&data.zDbFilename);
    warnInmemoryDb = 0;
  }
#endif

  /* Do an initial pass through the command-line argument to locate
  ** the name of the database file, the name of the initialization file,
  ** the size of the alternative malloc heap,
  ** and the first command to execute.
  */
  for(i=1; i<argc; i++){
    char *z;
    z = argv[i];
    if( z[0]!='-' ){
      if( data.zDbFilename==0 ){
        data.zDbFilename = z;
      }else{
        /* Excesss arguments are interpreted as SQL (or dot-commands) and
        ** mean that nothing is read from stdin */
        readStdin = 0;
        nCmd++;
        azCmd = realloc(azCmd, sizeof(azCmd[0])*nCmd);
        if( azCmd==0 ){
          fprintf(stderr, "out of memory\n");
          exit(1);
        }
        azCmd[nCmd-1] = z;
      }
    }
    if( z[1]=='-' ) z++;
    if( strcmp(z,"-separator")==0
     || strcmp(z,"-nullvalue")==0
     || strcmp(z,"-newline")==0
     || strcmp(z,"-cmd")==0
    ){
      (void)cmdline_option_value(argc, argv, ++i);
    }else if( strcmp(z,"-init")==0 ){
      zInitFile = cmdline_option_value(argc, argv, ++i);
    }else if( strcmp(z,"-batch")==0 ){
      /* Need to check for batch mode here to so we can avoid printing
      ** informational messages (like from process_sqliterc) before 
      ** we do the actual processing of arguments later in a second pass.
      */
      stdin_is_interactive = 0;
    }else if( strcmp(z,"-heap")==0 ){
#if defined(SQLITE_ENABLE_MEMSYS3) || defined(SQLITE_ENABLE_MEMSYS5)
      const char *zSize;
      sqlite3_int64 szHeap;

      zSize = cmdline_option_value(argc, argv, ++i);
      szHeap = integerValue(zSize);
      if( szHeap>0x7fff0000 ) szHeap = 0x7fff0000;
      sqlite3_config(SQLITE_CONFIG_HEAP, malloc((int)szHeap), (int)szHeap, 64);
#endif
    }else if( strcmp(z,"-scratch")==0 ){
      int n, sz;
      sz = (int)integerValue(cmdline_option_value(argc,argv,++i));
      if( sz>400000 ) sz = 400000;
      if( sz<2500 ) sz = 2500;
      n = (int)integerValue(cmdline_option_value(argc,argv,++i));
      if( n>10 ) n = 10;
      if( n<1 ) n = 1;
      sqlite3_config(SQLITE_CONFIG_SCRATCH, malloc(n*sz+1), sz, n);
      data.shellFlgs |= SHFLG_Scratch;
    }else if( strcmp(z,"-pagecache")==0 ){
      int n, sz;
      sz = (int)integerValue(cmdline_option_value(argc,argv,++i));
      if( sz>70000 ) sz = 70000;
      if( sz<800 ) sz = 800;
      n = (int)integerValue(cmdline_option_value(argc,argv,++i));
      if( n<10 ) n = 10;
      sqlite3_config(SQLITE_CONFIG_PAGECACHE, malloc(n*sz+1), sz, n);
      data.shellFlgs |= SHFLG_Pagecache;
    }else if( strcmp(z,"-lookaside")==0 ){
      int n, sz;
      sz = (int)integerValue(cmdline_option_value(argc,argv,++i));
      if( sz<0 ) sz = 0;
      n = (int)integerValue(cmdline_option_value(argc,argv,++i));
      if( n<0 ) n = 0;
      sqlite3_config(SQLITE_CONFIG_LOOKASIDE, sz, n);
      if( sz*n==0 ) data.shellFlgs &= ~SHFLG_Lookaside;
#ifdef SQLITE_ENABLE_VFSTRACE
    }else if( strcmp(z,"-vfstrace")==0 ){
      extern int vfstrace_register(
         const char *zTraceName,
         const char *zOldVfsName,
         int (*xOut)(const char*,void*),
         void *pOutArg,
         int makeDefault
      );
      vfstrace_register("trace",0,(int(*)(const char*,void*))fputs,stderr,1);
#endif
#ifdef SQLITE_ENABLE_MULTIPLEX
    }else if( strcmp(z,"-multiplex")==0 ){
      extern int sqlite3_multiple_initialize(const char*,int);
      sqlite3_multiplex_initialize(0, 1);
#endif
    }else if( strcmp(z,"-mmap")==0 ){
      sqlite3_int64 sz = integerValue(cmdline_option_value(argc,argv,++i));
      sqlite3_config(SQLITE_CONFIG_MMAP_SIZE, sz, sz);
    }else if( strcmp(z,"-vfs")==0 ){
      sqlite3_vfs *pVfs = sqlite3_vfs_find(cmdline_option_value(argc,argv,++i));
      if( pVfs ){
        sqlite3_vfs_register(pVfs, 1);
      }else{
        fprintf(stderr, "no such VFS: \"%s\"\n", argv[i]);
        exit(1);
      }
    }
  }
  if( data.zDbFilename==0 ){
#ifndef SQLITE_OMIT_MEMORYDB
    data.zDbFilename = ":memory:";
    warnInmemoryDb = argc==1;
#else
    fprintf(stderr,"%s: Error: no database filename specified\n", Argv0);
    return 1;
#endif
  }
  data.out = stdout;

  /* Go ahead and open the database file if it already exists.  If the
  ** file does not exist, delay opening it.  This prevents empty database
  ** files from being created if a user mistypes the database name argument
  ** to the sqlite command-line tool.
  */
  if( access(data.zDbFilename, 0)==0 ){
    open_db(&data, 0);
  }

  /* Process the initialization file if there is one.  If no -init option
  ** is given on the command line, look for a file named ~/.sqliterc and
  ** try to process it.
  */
  process_sqliterc(&data,zInitFile);

  /* Make a second pass through the command-line argument and set
  ** options.  This second pass is delayed until after the initialization
  ** file is processed so that the command-line arguments will override
  ** settings in the initialization file.
  */
  for(i=1; i<argc; i++){
    char *z = argv[i];
    if( z[0]!='-' ) continue;
    if( z[1]=='-' ){ z++; }
    if( strcmp(z,"-init")==0 ){
      i++;
    }else if( strcmp(z,"-html")==0 ){
      data.mode = MODE_Html;
    }else if( strcmp(z,"-list")==0 ){
      data.mode = MODE_List;
    }else if( strcmp(z,"-line")==0 ){
      data.mode = MODE_Line;
    }else if( strcmp(z,"-column")==0 ){
      data.mode = MODE_Column;
    }else if( strcmp(z,"-csv")==0 ){
      data.mode = MODE_Csv;
      memcpy(data.colSeparator,",",2);
    }else if( strcmp(z,"-ascii")==0 ){
      data.mode = MODE_Ascii;
      sqlite3_snprintf(sizeof(data.colSeparator), data.colSeparator,
                       SEP_Unit);
      sqlite3_snprintf(sizeof(data.rowSeparator), data.rowSeparator,
                       SEP_Record);
    }else if( strcmp(z,"-separator")==0 ){
      sqlite3_snprintf(sizeof(data.colSeparator), data.colSeparator,
                       "%s",cmdline_option_value(argc,argv,++i));
    }else if( strcmp(z,"-newline")==0 ){
      sqlite3_snprintf(sizeof(data.rowSeparator), data.rowSeparator,
                       "%s",cmdline_option_value(argc,argv,++i));
    }else if( strcmp(z,"-nullvalue")==0 ){
      sqlite3_snprintf(sizeof(data.nullValue), data.nullValue,
                       "%s",cmdline_option_value(argc,argv,++i));
    }else if( strcmp(z,"-header")==0 ){
      data.showHeader = 1;
    }else if( strcmp(z,"-noheader")==0 ){
      data.showHeader = 0;
    }else if( strcmp(z,"-echo")==0 ){
      data.echoOn = 1;
    }else if( strcmp(z,"-eqp")==0 ){
      data.autoEQP = 1;
    }else if( strcmp(z,"-stats")==0 ){
      data.statsOn = 1;
    }else if( strcmp(z,"-scanstats")==0 ){
      data.scanstatsOn = 1;
    }else if( strcmp(z,"-backslash")==0 ){
      /* Undocumented command-line option: -backslash
      ** Causes C-style backslash escapes to be evaluated in SQL statements
      ** prior to sending the SQL into SQLite.  Useful for injecting
      ** crazy bytes in the middle of SQL statements for testing and debugging.
      */
      data.backslashOn = 1;
    }else if( strcmp(z,"-bail")==0 ){
      bail_on_error = 1;
    }else if( strcmp(z,"-version")==0 ){
      printf("%s %s\n", sqlite3_libversion(), sqlite3_sourceid());
      return 0;
    }else if( strcmp(z,"-interactive")==0 ){
      stdin_is_interactive = 1;
    }else if( strcmp(z,"-batch")==0 ){
      stdin_is_interactive = 0;
    }else if( strcmp(z,"-heap")==0 ){
      i++;
    }else if( strcmp(z,"-scratch")==0 ){
      i+=2;
    }else if( strcmp(z,"-pagecache")==0 ){
      i+=2;
    }else if( strcmp(z,"-lookaside")==0 ){
      i+=2;
    }else if( strcmp(z,"-mmap")==0 ){
      i++;
    }else if( strcmp(z,"-vfs")==0 ){
      i++;
#ifdef SQLITE_ENABLE_VFSTRACE
    }else if( strcmp(z,"-vfstrace")==0 ){
      i++;
#endif
#ifdef SQLITE_ENABLE_MULTIPLEX
    }else if( strcmp(z,"-multiplex")==0 ){
      i++;
#endif
    }else if( strcmp(z,"-help")==0 ){
      usage(1);
    }else if( strcmp(z,"-cmd")==0 ){
      /* Run commands that follow -cmd first and separately from commands
      ** that simply appear on the command-line.  This seems goofy.  It would
      ** be better if all commands ran in the order that they appear.  But
      ** we retain the goofy behavior for historical compatibility. */
      if( i==argc-1 ) break;
      z = cmdline_option_value(argc,argv,++i);
      if( z[0]=='.' ){
        rc = do_meta_command(z, &data);
        if( rc && bail_on_error ) return rc==2 ? 0 : rc;
      }else{
        open_db(&data, 0);
        rc = shell_exec(data.db, z, shell_callback, &data, &zErrMsg);
        if( zErrMsg!=0 ){
          fprintf(stderr,"Error: %s\n", zErrMsg);
          if( bail_on_error ) return rc!=0 ? rc : 1;
        }else if( rc!=0 ){
          fprintf(stderr,"Error: unable to process SQL \"%s\"\n", z);
          if( bail_on_error ) return rc;
        }
      }
    }else{
      fprintf(stderr,"%s: Error: unknown option: %s\n", Argv0, z);
      fprintf(stderr,"Use -help for a list of options.\n");
      return 1;
    }
  }

  if( !readStdin ){
    /* Run all arguments that do not begin with '-' as if they were separate
    ** command-line inputs, except for the argToSkip argument which contains
    ** the database filename.
    */
    for(i=0; i<nCmd; i++){
      if( azCmd[i][0]=='.' ){
        rc = do_meta_command(azCmd[i], &data);
        if( rc ) return rc==2 ? 0 : rc;
      }else{
        open_db(&data, 0);
        rc = shell_exec(data.db, azCmd[i], shell_callback, &data, &zErrMsg);
        if( zErrMsg!=0 ){
          fprintf(stderr,"Error: %s\n", zErrMsg);
          return rc!=0 ? rc : 1;
        }else if( rc!=0 ){
          fprintf(stderr,"Error: unable to process SQL: %s\n", azCmd[i]);
          return rc;
        }
      }
    }
    free(azCmd);
  }else{
    /* Run commands received from standard input
    */
    if( stdin_is_interactive ){
      char *zHome;
      char *zHistory = 0;
      int nHistory;
      printf(
        "SQLite version %s %.19s\n" /*extra-version-info*/
        "Enter \".help\" for usage hints.\n",
        sqlite3_libversion(), sqlite3_sourceid()
      );
      if( warnInmemoryDb ){
        printf("Connected to a ");
        printBold("transient in-memory database");
        printf(".\nUse \".open FILENAME\" to reopen on a "
               "persistent database.\n");
      }
      zHome = find_home_dir();
      if( zHome ){
        nHistory = strlen30(zHome) + 20;
        if( (zHistory = malloc(nHistory))!=0 ){
          sqlite3_snprintf(nHistory, zHistory,"%s/.sqlite_history", zHome);
        }
      }
      if( zHistory ) shell_read_history(zHistory);
      rc = process_input(&data, 0);
      if( zHistory ){
        shell_stifle_history(100);
        shell_write_history(zHistory);
        free(zHistory);
      }
    }else{
      rc = process_input(&data, stdin);
    }
  }
  set_table_name(&data, 0);
  if( data.db ){
    sqlite3_close(data.db);
  }
  sqlite3_free(data.zFreeOnClose); 
  return rc;
}
