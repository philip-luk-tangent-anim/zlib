/* minigzip.c -- simulate gzip using the zlib compression library
 * Copyright (C) 1995-1998 Jean-loup Gailly.
 * For conditions of distribution and use, see copyright notice in zlib.h 
 */

/*
 * minigzip is a minimal implementation of the gzip utility. This is
 * only an example of using zlib and isn't meant to replace the
 * full-featured gzip. No attempt is made to deal with file systems
 * limiting names to 14 or 8+3 characters, etc... Error checking is
 * very limited. So use minigzip only for testing; use gzip for the
 * real thing. On MSDOS, use only on file names without extension
 * or in pipe mode.
 */

/* @(#) $Id$ */

#include <stdio.h>
#include "zlib.h"

#ifdef STDC
#  include <string.h>
#  include <stdlib.h>
#else
   extern void exit  OF((int));
#endif


#if defined(MSDOS) || defined(OS2) || defined(WIN32)
#  include <fcntl.h>
#  include <io.h>
#  define SET_BINARY_MODE(file) setmode(fileno(file), O_BINARY)
#else
#  define SET_BINARY_MODE(file)
#endif

#ifdef VMS
#  define unlink delete
#  define GZ_SUFFIX "-gz"
#endif
#ifdef RISCOS
#  define unlink remove
#  define GZ_SUFFIX "-gz"
#  define fileno(file) file->__file
#endif

#ifndef GZ_SUFFIX
#  define GZ_SUFFIX ".gz"
#endif
#define SUFFIX_LEN (sizeof(GZ_SUFFIX)-1)

extern int unlink OF((const char *));

#define BUFLEN 4096
#define MAX_NAME_LEN 1024

#ifdef MAXSEG_64K
#  define local static
   /* Needed for systems with limitation on stack size. */
#else
#  define local
#endif

char *prog;

void error           OF((const char *msg));
void gz_compress     OF((FILE   *in, gzFile out));
void gz_uncompress   OF((gzFile in, FILE   *out));
void file_compress   OF((char  *file, char *mode));
void file_uncompress OF((char  *file));
int  main            OF((int argc, char *argv[]));

/* ===========================================================================
 * Display error message and exit
 */
void error(msg)
    const char *msg;
{
    fprintf(stderr, "%s: %s\n", prog, msg);
    exit(1);
}

/* ===========================================================================
 * Compress input to output then close both files.
 */
void gz_compress(in, out)
    FILE   *in;
    gzFile out;
{
    local char buf[BUFLEN];
    int len;
    int err;

    for (;;) {
        len = fread(buf, 1, sizeof(buf), in);
        if (ferror(in)) {
            perror("fread");
            exit(1);
        }
        if (len == 0) break;

        if (gzwrite(out, buf, (unsigned)len) != len) error(gzerror(out, &err));
    }
    fclose(in);
    if (gzclose(out) != Z_OK) error("failed gzclose");
}

/* ===========================================================================
 * Uncompress input to output then close both files.
 */
void gz_uncompress(in, out)
    gzFile in;
    FILE   *out;
{
    local char buf[BUFLEN];
    int len;
    int err;

    for (;;) {
        len = gzread(in, buf, sizeof(buf));
        if (len < 0) error (gzerror(in, &err));
        if (len == 0) break;

        if ((int)fwrite(buf, 1, (unsigned)len, out) != len) {
	    error("failed fwrite");
	}
    }
    if (fclose(out)) error("failed fclose");

    if (gzclose(in) != Z_OK) error("failed gzclose");
}


/* ===========================================================================
 * Compress the given file: create a corresponding .gz file and remove the
 * original.
 */
void file_compress(file, mode)
    char  *file;
    char  *mode;
{
    local char outfile[MAX_NAME_LEN];
    FILE  *in;
    gzFile out;

    strcpy(outfile, file);
    strcat(outfile, GZ_SUFFIX);

    in = fopen(file, "rb");
    if (in == NULL) {
        perror(file);
        exit(1);
    }
    out = gzopen(outfile, mode);
    if (out == NULL) {
        fprintf(stderr, "%s: can't gzopen %s\n", prog, outfile);
        exit(1);
    }
    gz_compress(in, out);

    unlink(file);
}


/* ===========================================================================
 * Uncompress the given file and remove the original.
 */
void file_uncompress(file)
    char  *file;
{
    local char buf[MAX_NAME_LEN];
    char *infile, *outfile;
    FILE  *out;
    gzFile in;
    int len = strlen(file);

    strcpy(buf, file);

    if (len > SUFFIX_LEN && strcmp(file+len-SUFFIX_LEN, GZ_SUFFIX) == 0) {
        infile = file;
        outfile = buf;
        outfile[len-3] = '\0';
    } else {
        outfile = file;
        infile = buf;
        strcat(infile, GZ_SUFFIX);
    }
    in = gzopen(infile, "rb");
    if (in == NULL) {
        fprintf(stderr, "%s: can't gzopen %s\n", prog, infile);
        exit(1);
    }
    out = fopen(outfile, "wb");
    if (out == NULL) {
        perror(file);
        exit(1);
    }

    gz_uncompress(in, out);

    unlink(infile);
}


/* ===========================================================================
 * Usage:  minigzip [-d] [-f] [-h] [-1 to -9] [files...]
 *   -d : decompress
 *   -f : compress with Z_FILTERED
 *   -h : compress with Z_HUFFMAN_ONLY
 *   -1 to -9 : compression level
 */

int main(argc, argv)
    int argc;
    char *argv[];
{
    int uncompr = 0;
    gzFile file;
    char outmode[20];

    strcpy(outmode, "wb6 ");

    prog = argv[0];
    argc--, argv++;

    while (argc > 0) {
      if (strcmp(*argv, "-d") == 0)
	uncompr = 1;
      else if (strcmp(*argv, "-f") == 0)
	outmode[3] = 'f';
      else if (strcmp(*argv, "-h") == 0)
	outmode[3] = 'h';
      else if ((*argv)[0] == '-' && (*argv)[1] >= '1' && (*argv)[1] <= '9' &&
	       (*argv)[2] == 0)
	outmode[2] = (*argv)[1];
      else
	break;
      argc--, argv++;
    }
    if (argc == 0) {
        SET_BINARY_MODE(stdin);
        SET_BINARY_MODE(stdout);
        if (uncompr) {
            file = gzdopen(fileno(stdin), "rb");
            if (file == NULL) error("can't gzdopen stdin");
            gz_uncompress(file, stdout);
        } else {
            file = gzdopen(fileno(stdout), outmode);
            if (file == NULL) error("can't gzdopen stdout");
            gz_compress(stdin, file);
        }
    } else {
        do {
            if (uncompr) {
                file_uncompress(*argv);
            } else {
                file_compress(*argv, outmode);
            }
        } while (argv++, --argc);
    }
    exit(0);
    return 0; /* to avoid warning */
}
