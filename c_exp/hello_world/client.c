#include <stdio.h>

#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>

#include <string.h>

int main(int argc, char *argv[])
{
	printf("start client\n");

	//link vars
	int client_fd;
	struct sockaddr_in serv_addr, client_addr;
	socklen_t client_size = sizeof(client_addr);

	//data
	char buffer[1024] = {0};

	//create socket
	client_fd = socket(AF_INET, SOCK_STREAM, 0);

	//conf
    client_addr.sin_family = AF_INET;
    client_addr.sin_addr.s_addr = INADDR_ANY;
    client_addr.sin_port = htons(4242);

	//connect
	connect(client_fd, (struct sockaddr *)&client_addr, client_size);

	//send ???
	send(client_fd, "hella", 5, 0);

	//rcv
	memset(buffer, 0, 1024);

	recv(client_fd, buffer, (1024-1), 0);

	printf("message back from server %s\n", buffer);

	close(client_fd);

	return 0;
}