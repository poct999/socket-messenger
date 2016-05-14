#ifndef MESSENGER_SERVER_H
#define MESSENGER_SERVER_H

#include <sys/types.h>
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>

#include "messenger.h"
#include "list.h"

//API FOR USERS:

typedef struct user
{
	char* login;
	int socket;
}user;

element* find_element_by_login(element** head, char* login); //находит структуру в списке, соответстсвующую login

user* add_user(element** head, char* login, int socket);

int find_socket_by_login(element** head, char* login); //возвращает sockfd, соответствующий login

int remove_user_by_login(element** head, char* login); 

void print_user_list(element** head); //debug function



//API FOR GROUPS: 

typedef struct group
{
	char* group_name;
	element* user_list_head;
	user* creator;
}group;

element* find_element_by_group_name(element** head, char* group_name); //находит структуру в списке, соответстсвующую group_name

group* add_group(element** head, char* group_name, user* creator);

int find_user_in_group(element** head, char* group_name, user* u);

int add_user_to_group_by_group_name(element** head, char* group_name, user* u); //добавляет пользователя в группу

int remove_user_from_group(element** head, group* g, user* u);

int remove_group(element** head, group* g);

int print_group_list(element** head); //debug function


//SERVER WORK
int write_answer(int socket, unsigned char answer); //Записать ответ из TYPE_ANSWER_*

int serve_client(element**client_head, element** group_head, user *u); //Основная функция обслуживания клиента

int server_login(element**client_head, int socket); //Считать и создать юзера


#endif
