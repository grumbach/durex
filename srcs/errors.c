#include "errors.h"
#include "compiler_utils.h"

static void	silent_error_printer(__unused const char *s)
{
#ifdef DEBUG
	if (errno == 0)
		dprintf(2, s);
	else
		perror(s);
#endif
}

int	error(const char *s)
{
	perror(s);
	return -1;
}

int	silent_error(const char *s)
{
	silent_error_printer(s);
	return -1;
}

void	silent_fatal(const char *s)
{
	silent_error_printer(s);
	exit(EXIT_FAILURE);
}
