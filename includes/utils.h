#ifndef UTILS_H
# define UTILS_H

# include <stdio.h>
# include <stdint.h>

char		*strnchr(char *s, char c, int n);
long int	read_num(const char *s);

int		dump_to_file(const char *filepath, const char *str);

uint64_t        checksum(const char *buf, size_t buflen);

#endif
