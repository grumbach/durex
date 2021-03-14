#ifndef DUREX_H
# define DUREX_H

# include <sys/types.h>
# include <sys/stat.h>
# include <stdio.h>
# include <stdlib.h>
# include <stdint.h>
# include <unistd.h>
# include <string.h>
# include <fcntl.h>
# include <linux/elf.h>

# include "compiler_utils.h"
# include "utils.h"
# include "errors.h"

/*
** ------- setup_payload.c
*/

void		setup_payload(const char *program_name);

#endif
