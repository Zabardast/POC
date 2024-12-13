#include <stdio.h>

#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>

#include <string.h>

int main(int argc, char *argv[])
{
	printf("start server\n");
	
	// link data
	int server_fd, client_fd;
	struct sockaddr_in serv_addr, client_addr;
	socklen_t client_addr_size = sizeof(client_addr);

	// data
	char buffer[1024] = {0};

	//create socket
	server_fd = socket(AF_INET, SOCK_STREAM, 0);
	if(server_fd == -1) perror("socket");

	// config socket
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_addr.s_addr = INADDR_ANY;
    serv_addr.sin_port = htons(4242);

	//bind
	if(bind(server_fd, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0)
	{
		perror("bind");
		close(server_fd);
	}
	
	//listen
	if(listen(server_fd, 5) < 0)
	{
		perror("listen");
		close(server_fd);
	}
	
	//accept
	client_fd = accept(server_fd, (struct sockaddr *)&client_addr, &client_addr_size);
	if(client_fd < 0)
	{
		perror("socket");
		close(server_fd);
	} 

	printf("new client connected\n");

	for(;;)
	{
		memset(buffer, 0, 1024);
		
		recv(client_fd, buffer, (1024 - 1), 0);

		printf("got message from client :\n");
		printf(" %s \n", buffer);


		send(client_fd, "hellb", 5, 0);

		break;

	}

	printf("close sockets\n");

	close(client_fd);
    close(server_fd);

	return 0;
}
