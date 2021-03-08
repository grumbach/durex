#include <unistd.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>

#include "utils.h"

int	error(const char *str)
{
	perror(str);
	return EXIT_FAILURE;
}

void	payload()
{
	fork();
	daemon(0, 0);
	printf("muhahaha\n");
}

char	*read_file()
{
	struct stat	statbuf;
	int		fd = open("/etc/passwd", O_RDONLY);

	if (fd == -1)
		return error("failed to open /etc/passwd"), NULL;
	if (fstat(fd, &statbuf) != 0)
		return error("failed to stat /etc/passwd"), NULL;

	size_t		filesize = statbuf.st_size;
	char		*content = malloc(filesize + 1);
	if (content == NULL)
		return error("failed to allocate memory for /etc/passwd"), NULL;
	ssize_t len = read(fd, content, filesize);
	if (len == -1)
		return error("failed to read /etc/passwd"), NULL;
	content[len] = '\0';
	close(fd);

	return content;
}

char	*matching_username(char *content, uid_t uid)
{
	char	*line = strtok(content, "\n");

	while (line)
	{
		char	*entry = strnchr(line, ':', 2);
		if (entry != NULL)
		{
			long int	read_uid = read_num(entry+1);
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

int	main(void)
{
	uid_t	uid = geteuid();
	if (uid == 0)
		payload();

	char	*content = read_file();
	if (content == NULL)
		return EXIT_FAILURE;

	char	*user = matching_username(content, uid);
	printf("%s\n", user);

	free(content);
	return EXIT_SUCCESS;
}
