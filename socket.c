#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <netdb.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <unistd.h>

typedef struct {
	int socket;
	struct sockaddr_storage addr;
} sockinfo;

void socket_close(int s)
{
	close(s);
}

sockinfo *socket_accept(int s)
{

	sockinfo *info;
	socklen_t addr_size;

	info = malloc(sizeof(sockinfo));

	info->socket = accept(s, (struct sockaddr *)&info->addr, &addr_size);

	return info;
}

int socket_listen_new(int port)
{
	struct addrinfo hints, *info, *p;
	char portstr[32];
	int r, sock;

	memset(&hints, 0, sizeof(hints));
	hints.ai_family		= AF_UNSPEC;
	hints.ai_socktype	= SOCK_STREAM;
	hints.ai_flags		= AI_PASSIVE;

	sprintf(portstr, "%d", port);

	r = getaddrinfo(NULL, portstr, &hints, &info);
	if(r)
	{
		fprintf(stderr, "error: getaddrinfo: %s\n", gai_strerror(r));
		exit(r);
	}

	for(p = info; p; p = p->ai_next)
	{
		int yes;

		sock = socket(p->ai_family, p->ai_socktype, p->ai_protocol);
		if(sock == -1)
		{
			fprintf(stderr, "warning: Unable to create socket.\n");
			continue;
		}

		r = setsockopt(sock, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(int));
		if(r == -1)
		{
			fprintf(stderr, "warning: Unable to set SO_REUSEADDR on socket.\n");
			continue;
		}

		r = bind(sock, p->ai_addr, p->ai_addrlen);
		if(r == -1)
		{
			fprintf(stderr, "warning: Unable to bind on socket.\n");
			continue;
		}

		break;
	}

	if(!p)
	{
		fprintf(stderr, "Fatal: Unable to create socket.\n");
		exit(1);
	}

	freeaddrinfo(info);

	r = listen(sock, 10);
	if(r == -1)
	{
		fprintf(stderr, "Fatal: Unable to listen on socket.\n");
		exit(1);
	}

	printf("Listening on port %s.\n", portstr);

	return sock;
}