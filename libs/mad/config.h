#define HAVE_ASSERT_H 1
#define HAVE_ERRNO_H 1
#define HAVE_LIMITS_H 1
#define HAVE_STDLIB_H 1
#define HAVE_STRING_H 1
#define HAVE_SYS_TYPES_H 1
#define HAVE_FCNTL_H 1


/* Define if your MIPS CPU supports a 2-operand MADD16 instruction. */
/* #undef HAVE_MADD16_ASM */
/* Define if your MIPS CPU supports a 2-operand MADD instruction. */
/* #undef HAVE_MADD_ASM */


#if defined(unix) || defined(__unix__) || defined(__APPLE__)
#define HAVE_FORK 1
#define HAVE_PIPE 1
#define HAVE_SYS_WAIT_H 1
#define HAVE_UNISTD_H 1
#define HAVE_WAITPID 1
#define HAVE_FCNTL 1
#endif	/* UNIX */


/* Define to optimize for accuracy over speed. */
/* #undef OPT_ACCURACY */
/* Define to optimize for speed over accuracy. */
#define OPT_SPEED 1

/* Define to enable a fast subband synthesis approximation optimization. */
/* #undef OPT_SSO */

/* Define to influence a strict interpretation of the ISO/IEC standards, even
   if this is in opposition with best accepted practices. */
/* #undef OPT_STRICT */

#define SIZEOF_INT 4
#define SIZEOF_LONG 8
#define SIZEOF_LONG_LONG 8

/* Define to 1 if you have the ANSI C header files. */
/* #undef STDC_HEADERS */

/* Version number of package */
#define VERSION "0.15.1b"

#if defined(__mips) || defined(__sparc__) || defined(__sparc)
#define WORDS_BIGENDIAN
#endif

/* Define to `__inline__' or `__inline' if that's what the C compiler
   calls it, or to nothing if 'inline' is not supported under any name.  */
#ifndef __cplusplus
#if defined(__SUNPRO_C)
#define inline inline
#else
#define inline __inline
#endif
#endif
