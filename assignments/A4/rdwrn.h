/* rdwrn.h

   Header file for rdwrn.c.
*/
#ifndef RDWRN_H
#define RDWRN_H

#include <sys/types.h>

int find_network_newline(const char *buf, int n);

ssize_t readn(int fd, void *buf, size_t len);

ssize_t writen(int fd, const void *buf, size_t len);

#endif
