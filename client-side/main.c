#include<stdio.h>
#include<string.h>	//strlen
#include<sys/socket.h>
#include<arpa/inet.h>	//inet_addr

int main(int argc , char *argv[])
{
	int socket_desc;
	//struct sockaddr_in server;
	struct sockaddr_in6 server;
	int incomingMessage;
	char buffer[1024] = {0};

	//Create socket
	//socket_desc = socket(AF_INET , SOCK_STREAM , 0);
	socket_desc = socket(AF_INET6 , SOCK_STREAM , 0);
	if (socket_desc == -1)
	{
		printf("Could not create socket");
	}

	/*
	server.sin_addr.s_addr = inet_addr("127.0.0.1");
	server.sin_family = AF_INET;
	server.sin_port = htons( 9000 );
	*/

	server.sin6_family = AF_INET6;
	inet_pton(AF_INET6, "::1", &server.sin6_addr);
	server.sin6_port = htons( 9000 );

	//Connect to remote server
	if (connect(socket_desc , (struct sockaddr *)&server , sizeof(server)) < 0)
	{
		puts("Could not connect");
		return 1;
	}

	puts("Connected\n");

	while(1) {
	//bzero(buffer, sizeof buffer);
	memset(buffer,'\0',strlen(buffer));
	printf("\n");
        fgets(buffer, 1024, stdin);
        if(strlen(buffer)==0) break;
        send(socket_desc, buffer, strlen(buffer), 0);
        incomingMessage = recv(socket_desc, &buffer, sizeof(buffer), 0);
        printf("Message received: %s", buffer);

	}

	close(socket_desc);

	return 0;
}
