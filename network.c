#include <sys/socket.h>
#include <stdlib.h>
#include <assert.h>
#include <string.h>
#include <netinet/in.h>
#include <stdio.h>
#include <errno.h>
#include <fcntl.h>

#include "command.h"
#include "network.h"

struct network *nNewCommandInterface(int port)
{
	struct network *net = malloc(sizeof(*net));
	struct sockaddr_in addr;

	if ((net->socket = socket(AF_INET, SOCK_DGRAM, 0)) == -1) {
		return NULL;
	}
	memset(&addr, 0, sizeof(addr));
	addr.sin_family = AF_INET;
	addr.sin_addr.s_addr = htonl(INADDR_ANY);
	addr.sin_port = htons(port);

	if (fcntl(net->socket, F_SETFL, O_NONBLOCK) == -1) {
		return NULL;
	}
	if (bind(net->socket, (struct sockaddr *)&addr, sizeof(addr)) == -1) {
		return NULL;
	}
	return net;
}

int nPollCommand(struct network *net, struct command *cmd)
{
	char buf[1024] = {0};

	ssize_t nread = recvfrom(net->socket, buf, sizeof(buf), 0, NULL, NULL);

	if (nread == 0) { // Peer shutdown
		return -1;
	} else if (nread == -1 && (errno == EAGAIN || errno == EWOULDBLOCK)) {
		return -1;
	}
	return cmdFromString(cmd, buf, nread);
}

void nPollEvents(struct network *net)
{

}
