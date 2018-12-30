/*************************************************************************\
*                  Copyright (C) Michael Kerrisk, 2018.                   *
*                                                                         *
* This program is free software. You may use, modify, and redistribute it *
* under the terms of the GNU Lesser General Public License as published   *
* by the Free Software Foundation, either version 3 or (at your option)   *
* any later version. This program is distributed without any warranty.    *
* See the files COPYING.lgpl-v3 and COPYING.gpl-v3 for details.           *
\*************************************************************************/

/* rdwrn.c

   Implementations of readn() and writen().
*/
#include <unistd.h>
#include <errno.h>

#include "rdwrn.h"                      /* Declares readn() and writen() */

/* Search the first n characters of buf for a network newline (\r\n).
 * Return one plus the index of the '\n' of the first network newline,
 * or -1 if no network newline is found.
 * Definitely do not use strchr or other string functions to search here. (Why not?)
 */
int find_network_newline(const char *buf, int n) {
    for (int i = 1; i < n; i++) {
        if (buf[i - 1] == '\r' && buf[i] == '\n') {
            return i + 1;
        }
    }
    return -1;
}


/* Read 'n' bytes from 'fd' into 'buf', restarting after partial
   reads or interruptions by a signal handlers
   Original.
*/
ssize_t readn(int fd, void *buffer, size_t n) {
    ssize_t num_read;                    /* # of bytes fetched by last read() */
    size_t tot_read;                     /* Total # of bytes read so far */
    char *buf;

    buf = buffer;                       /* No pointer arithmetic on "void *" */
    for (tot_read = 0; tot_read < n; ) {
        num_read = read(fd, buf, n - tot_read);

        if (num_read == 0)               /* EOF */
            return tot_read;             /* May be 0 if this is first read() */
        if (num_read == -1) {
            if (errno == EINTR)
                continue;               /* Interrupted --> restart read() */
            else
                return -1;              /* Some other error */
        }
        tot_read += num_read;
        buf += num_read;
    }
    return tot_read;                     /* Must be 'n' bytes if we get here */
}


/* Write 'n' bytes to 'fd' from 'buf', restarting after partial
   write or interruptions by a signal handlers */
ssize_t writen(int fd, const void *buffer, size_t n) {
    ssize_t num_written;                 /* # of bytes written by last write() */
    size_t tot_written;                  /* Total # of bytes written so far */
    const char *buf;

    buf = buffer;                       /* No pointer arithmetic on "void *" */
    for (tot_written = 0; tot_written < n; ) {
        num_written = write(fd, buf, n - tot_written);

        /* The "write() returns 0" case should never happen, but the
           following ensures that we don't loop forever if it does */

        if (num_written <= 0) {
            if (num_written == -1 && errno == EINTR)
                continue;               /* Interrupted --> restart write() */
            else
                return -1;              /* Some other error */
        }
        tot_written += num_written;
        buf += num_written;
    }
    return tot_written;                  /* Must be 'n' bytes if we get here */
}
