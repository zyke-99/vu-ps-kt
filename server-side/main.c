#include<stdio.h>
#include<string.h>
#include<sys/socket.h>
#include<arpa/inet.h>
#include<unistd.h>
#include <stdlib.h>


//Pagalbine struktura saugot client'o informacija - port, address, domain(AF_INET and etc.). galima ir be jos, taciau ji padetu, jei reiktu, tarkim, chat room
//vardus laikyti and so on
struct client_info {

	struct sockaddr_in6 client_addr;

};

/*
IPv4 struktura kliento informacijai saugoti

struct client_info {


	struct sockaddr_in client_addr;

};

*/





int main(int argc , char *argv[])
{

	//integer'iai laikyti pagrindinio socketo deskriptoriu bei pagalbinius deskriptorius
	int socket_desc , new_socket , c;
	//struct sockaddr_in server , client; //IPv4, jei norim IPv6, reik sockaddr_in6
	struct sockaddr_in6 server, client;
	//musu pagalbines strukturos deklaravimas kaip array (suteikiam didziausia galimo FD rinkinio nariu skaiciu)
	struct client_info cl_info[FD_SETSIZE];
	//Musu file descriptor rinkiniai;
	fd_set current_sockets, read_sockets;
	//pagalbinis integer'is suzinoti, kas ivyko, kai bandem priimti is kliento duomenis
	int recv_val;


	//Sukuriamas socket'as
	/*
	socket_desc = socket(AF_INET , SOCK_STREAM , 0);  //AF_INET nurodys, kad naudosim IPv4, SOCK_STREAM - TCP, SOCK_DGRAM - UDP
	if (socket_desc == -1)
	{
		printf("Could not create socket");
	}
	*/
	socket_desc = socket(AF_INET6 , SOCK_STREAM , 0);
	if(socket_desc<0) {

		printf("Could not create socket\n");
	
	}
	printf("Socket created successfully\n");

	//"Nunulinamas" turimu socket'u rinkinys, FD_SET - pridedam savo serverio (socket_desc) socket'a i rinkini, niekad jo neisimam, nes bus negerai:
//jei tiksliau, tai nustosim acceptint connectionus
    	FD_ZERO(&current_sockets);
	FD_SET(socket_desc, &current_sockets);


	//Sockadd_in struktura
	/*
	server.sin_family = AF_INET; //IPv4
	server.sin_addr.s_addr = inet_addr("127.0.0.1"); //IPv4 local address -- INADDR_ANY jei norim nurodyti visus available interfeisus
	server.sin_port = htons( 9000 ); //Porto skaicius

	//inet_addr neveiks su ipv6, reiks     inet_pton(AF_*, string-ipv6addr, &addr.sin6_addr);
	*/

	server.sin6_family = AF_INET6;
	inet_pton(AF_INET6, "::1", &server.sin6_addr);
	server.sin6_port = htons( 9000 );


	//Surisa socket'a su paduota musu serverio adreso struktura (Siuo atveju device adresu ir pasirinktu 9000 port'u)
	if( bind(socket_desc,(struct sockaddr *)&server , sizeof(server)) < 0)
	{
		puts("Failed to bind the socket to the given address and port");
		return 1;
	}
	puts("Socket bound succsesfuly to the given address and port\n");

	//klausom atkeliaujancios info i sita addr ir port(socketa)
	if((listen(socket_desc , 3))<0) {
	
		puts("Failed to set socket to listening mode");
		return 1;

	}

	//Priimam atkeliaujancia uzklausa prisijungti
	puts("Waiting for client to connect\n");


	char buffer[1024];
	char clientname[512];
    	while(1) {
		
		//kadangi select funkcija destruktyvi (modifikuoja fd_set turini), issaugom visu prisijungusiu socket'u deskriptorius naudodami gilias kopijas
		read_sockets = current_sockets;
		if(select(FD_SETSIZE, &read_sockets, NULL, NULL, NULL)<0) {

			puts("Error occured while checking sockets");

		}
		//Iteruojam per visus galimus socketus ir ne tik
		for(int i=0; i<FD_SETSIZE; i++) { 

			//tikrinam, ar socket'as (arba ne socketas, tokiu atveju nepateks cionais) yra readable
			if(FD_ISSET(i, &read_sockets)) {
				
				//Tikrinam, ar tai pagr. serverio socket'as, aka listeneris. jei taip ir jis yra readable, tai veikiausiai laukia connection request'as
				if(i == socket_desc) {

					memset(buffer,'\0',strlen(buffer));
					new_socket = accept(socket_desc, (struct sockaddr *)&client, (socklen_t*)&c);
					if(new_socket<0) {
				
						puts("Failed to accept connection request from client");

					} else {
						//pridedam i turimu socketu rinkini bei issaugom informacija(address, port, family apie client'a)
						FD_SET(new_socket, &current_sockets);
						cl_info[new_socket].client_addr.sin6_addr = client.sin6_addr;
						cl_info[new_socket].client_addr.sin6_port = client.sin6_port;
						cl_info[new_socket].client_addr.sin6_family = client.sin6_family;
						sprintf(buffer, "Connection request accepted from: %s:%d",
							inet_ntop(cl_info[new_socket].client_addr.sin6_family,&cl_info[new_socket].client_addr.sin6_addr,
                    					clientname,sizeof(clientname)), htons(cl_info[new_socket].client_addr.sin6_port));
						/*

						sprintf(buffer, "Connection request accepted from: %s:%d",
							inet_ntop(cl_info[new_socket].client_addr.sin_family,&cl_info[new_socket].client_addr.sin_addr,
                    					clientname,sizeof(clientname)), htons(cl_info[new_socket].client_addr.sin_port));

						*/
						puts(buffer);

					}
					
					
					
				} else {

					memset(buffer,'\0',strlen(buffer));
					recv_val=recv(i, buffer, sizeof(buffer), 0);
					//Tikrinam, ka gavom(baitu skaicius). Jei 0, tai reiskias client'as nutrauke komunikacija.
					if(recv_val == 0) {

						sprintf(buffer, "Client disconnected: %s:%d",
							inet_ntop(cl_info[i].client_addr.sin6_family,&cl_info[i].client_addr.sin6_addr,
                    					clientname,sizeof(clientname)), htons(cl_info[i].client_addr.sin6_port));
						/*

						sprintf(buffer, "Connection request accepted from: %s:%d",
							inet_ntop(cl_info[new_socket].client_addr.sin_family,&cl_info[new_socket].client_addr.sin_addr,
                    					clientname,sizeof(clientname)), htons(cl_info[new_socket].client_addr.sin_port));

						*/
					puts(buffer);
						//Graziai istrinam atsijungusio kliento sukurta socket'a is musu PAGRINDINIO rinkinio ir uzdarom socket'a.
						FD_CLR(i, &current_sockets);
						close(i);
					//jei is recv gavom maziau nei 0, reiskias error bandant gaut is kliento zinute
					} else if(recv_val < 0) {
						
						puts("Error while attempting to receive message from client");
					//jei viskas gerai, tai kliento zinute paverciam i uppercase zinute ir nusiunciam jam atgal, netikrinam, nes neidomu
					} else {
					
						for(int j=0; j<sizeof(buffer); j++) {

							buffer[j] = toupper(buffer[j]);

						}
						send(i, buffer, sizeof(buffer), 0);

					}
					
					
				}
		
			}


		}


	
    }

    close(socket_desc);

    printf("Work done!");
	return 0;
}
