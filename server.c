#include <sys/types.h>
#include <sys/socket.h>
#include <stdio.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <signal.h>

#include "messenger.h"
#include "list.h"
#include "messengerserver.h"

static element* clients_list_head = NULL; //Указатель на список пользователей
static element* groups_list_head = NULL; //Указатель на список групп

void ctrlc(int sig) {
	element* temp;
	element* temp2;

	temp = clients_list_head;
	while (temp != NULL){
		user* u = (user*) temp->data;
		temp2 = temp->prev;
		close(u->socket);
		remove_user_by_login(&clients_list_head, u->login);		
		temp = temp2;
	}

	temp = groups_list_head;
	while (temp != NULL){
		group *g = (group*) temp->data;
		temp2 = temp->prev;
		remove_group(&groups_list_head, g);
		temp = temp2;
	}

	exit(0);
}
 	
int main() 
{
	struct sigaction act;
	act.sa_handler = ctrlc;sigemptyset(&act.sa_mask);
	act.sa_flags = 0;sigaction(SIGINT, &act, 0);

	int server_sockfd, client_sockfd;
	int server_len;
	struct sockaddr_in server_address;
	struct sockaddr_in client_address;

	server_sockfd = socket(AF_INET, SOCK_STREAM, 0);
	if(server_sockfd < 0)
    {
        perror("socket");
        exit(1);
    }

	server_address.sin_family = AF_INET;
	server_address.sin_addr.s_addr = htonl(INADDR_ANY);
	server_address.sin_port = htons(9734);
	
	server_len = sizeof(server_address);
	if(bind(server_sockfd, (struct sockaddr *)&server_address, server_len) < 0)
	{
        perror("bind");
        exit(1);
    }

	printf("Ok, go!\n");

	listen(server_sockfd, 5);
	fd_set readfds, testfds;
	FD_ZERO(&readfds);
 	FD_SET(server_sockfd, &readfds);

	while (1){
		testfds = readfds;
		
		if ((select(FD_SETSIZE, &testfds, (fd_set *)0, (fd_set *)0, (struct timeval *)0)) < 1) {
			perror("Select error");
			exit(1);
		}
	
		if (FD_ISSET(server_sockfd, &testfds)){
			client_sockfd = accept(server_sockfd, NULL, NULL);
	 		if(client_sockfd < 0)
	        {
	            perror("accept");
	            exit(1);
	        }
	        printf("ACCEPTED!\n");
	        
	        if (server_login(&clients_list_head, client_sockfd) > 0) FD_SET(client_sockfd, &readfds);
    			else close(client_sockfd);
	    }
		else{
			element* temp = clients_list_head;
			user* u;
			while (temp != NULL){
				u = (user*) temp->data;
				int sock = u->socket;
				if (FD_ISSET(sock, &testfds)){
					if (serve_client(&clients_list_head,&groups_list_head,u) < 0) FD_CLR(sock, &readfds);
					break;
				}
				temp = temp->prev;
			}
		}
					
	}
}
