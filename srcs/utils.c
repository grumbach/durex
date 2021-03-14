#include <string.h>
#include <stdlib.h>
#include <errno.h>
#include <limits.h>
#include <stdio.h>
#include <stdint.h>

int	dump_to_file(const char *filepath, const char *str)
{
	FILE	*dst = fopen(filepath, "w");
	if (dst == NULL) return -1;

	if (fputs(str, dst) == EOF)
		return -1;

	fclose(dst);
	return 0;
}

char	*strnchr(char *s, char c, int n)
{
	while (*s)
	{
		if (*s == c)
		{
			n--;
			if (n == 0)
				return s;
		}
		s++;
	}
	return NULL;
}

/*
** read_num
**   - fancy atoi
**   - only accepts positive number
**   - returns -1 on error
*/
long int	read_num(const char *s)
{
	// to distinguish success/failure after call
	errno = 0;

	char		*endptr;
	long int	val = strtol(s, &endptr, 10);

	// check for various possible errors
	if ((errno == ERANGE && (val == LONG_MAX || val == LONG_MIN))
	|| (errno != 0 && val == 0))
		return -1;

	// no digits found
	if (endptr == s)
		return -1;

	// negative int
	if (val < 0)
		return -1;

	return val;
}

uint64_t        checksum(const char *buf, size_t buflen)
{
	uint64_t	sum = 0;

	while (buflen--)
		sum += buf[buflen];
	return sum;
}
