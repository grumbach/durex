#ifndef ERRORS_H
# define ERRORS_H

# include <stdio.h>
# include <stdlib.h>
# include <errno.h>

# ifdef DEBUG
#  define silent_log(...)	printf(__VA_ARGS__)
# else
#  define silent_log(...)
# endif

/*
** ------- errors.c
*/

int		error(const char *s);
int		silent_error(const char *s);
void		silent_fatal(const char *s);

#endif
