#include "errors.h"
#include "compiler_utils.h"

int	error(const char *s)
{
	perror(s);
	return -1;
}

int	silent_error(__unused const char *s)
{
#ifdef DEBUG
	perror(s);
#endif
	return -1;
}

void	silent_fatal(__unused const char *s)
{
#ifdef DEBUG
	perror(s);
#endif
	exit(EXIT_FAILURE);
}
