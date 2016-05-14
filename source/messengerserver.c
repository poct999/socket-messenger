#include "messengerserver.h"

//API FOR USERS:

element* find_element_by_login(element** head, char* login) 
{
	element* temp = (*head);
	user* u;
	while (temp != NULL){
		u = (user*) temp->data;
		if (!strcmp(u->login,login)) 
			return temp;

		temp = temp->prev;
	}
	return NULL;
}

user* add_user(element** head, char* login, int socket)
{
	if (find_element_by_login(head, login) != NULL) return NULL;

	user* u = (user*) malloc(sizeof(user));
	if(u == NULL){
        perror("memory error");
        exit(1);
    }
 	
	u->login = (char*) malloc(strlen(login)*sizeof(char)+1);
	if(u->login == NULL){
        perror("memory error");
        exit(1);
    }

	strcpy(u->login, login);
	u->socket = socket;

	push_element(head, (void*) u);
	return u;
}

int find_socket_by_login(element** head, char* login) 
{
	element* temp = find_element_by_login(head, login);
	if (temp == NULL) 
		return -1;
	
	user* u = (user*) temp->data;
	
	return u->socket;
}

int remove_user_by_login(element** head, char* login)
{
	element* temp = find_element_by_login(head, login);
	if (temp == NULL) return -1;

	user* u = (user*)pop_element(head, temp);
	if (u->login != NULL) 
		free(u->login);
	free(u);
	
	return 1;
}

void print_user_list(element** head)
{
	element* temp = (*head);
	user* u;
	int count = 0;
	
	while (temp != NULL){
		u = (user*) temp->data;
		
		printf("[%d] Login: %s | Socket: %d\n",count++,u->login, u->socket);
		
		temp = temp->prev;
	}
	
	printf("-------------\n");
}



//API FOR GROUPS: 

element* find_element_by_group_name(element** head, char* group_name) 
{
	element* temp = (*head);
	group* g;
	while (temp != NULL){
		g = (group*) temp->data;
		
		if (!strcmp(g->group_name,group_name)) 
			return temp;
		
		temp = temp->prev;
	}

	return NULL;
}

group* add_group(element** head, char* group_name, user* creator)
{
	if (find_element_by_group_name(head,group_name) != NULL) return NULL;

	group* g = (group*) malloc(sizeof(group));
	if(g == NULL){
        perror("memory error");
        exit(1);
    }
	
	g->group_name = (char*) malloc(strlen(group_name)*sizeof(char)+1);
	if(g->group_name == NULL){
        perror("memory error");
        exit(1);
    }
	
	strcpy(g->group_name, group_name);
	g->creator = creator;
	push_element(&g->user_list_head, (void*) creator);

	push_element(head, (void*) g);
	return g;
}

int find_user_in_group(element** head, char* group_name, user* u){
	element* e = find_element_by_group_name(head, group_name);
	if (e == NULL) return -1;
	
	group* g = (group*) e->data;
	element* temp = g->user_list_head;
	while (temp != NULL){
		user* us = (user*) temp->data;
		
		if (us == u) 
			return 1;
		
		temp = temp->prev;
	}
	
	return -1;
}

int add_user_to_group_by_group_name(element** head, char* group_name, user* u) 
{
	if (find_user_in_group(head, group_name, u) > 0) 
		return -1;
	
	element* e = find_element_by_group_name(head, group_name);
	group* g = (group*) e->data;
	push_element(&g->user_list_head, (void*) u);
}

int remove_user_from_group(element** head, group* g, user* u)
{
	element* temp = g->user_list_head;
	while (temp != NULL){
		user* u1 = (user*) temp->data;
	
		if (u1 == u) {
			void* p = pop_element(&g->user_list_head, temp);
			return (p == NULL) ? -1 : 1; 
		}
	
		temp = temp->prev;
	}
	
	return -1;
}
void *remove_user(element** client_head, element** group_head, user* u)
{
	printf("Removed user: %s\n", u->login);
	
	element* temp = (*group_head);
	while (temp != NULL){
		group* g = (group*) temp->data;
		remove_user_from_group(group_head, g, u);
		element *temp2 = temp->prev;
		if (g->user_list_head == NULL) {remove_group(group_head, g);}
		
		temp = temp2;
	}

	close(u->socket);
	remove_user_by_login(client_head, u->login);
}

int remove_group(element** head, group* g)
{
	element* temp = find_element_by_group_name(head, g->group_name);
	if (temp == NULL) return -1;

	pop_element(head, temp);

	if (g->group_name != NULL) free(g->group_name);
	clear_list(&g->user_list_head);
	free(g);

	return 1;
}

int print_group_list(element** head)
{
	element* temp = (*head);
	group* g;
	int count = 0;

	while (temp != NULL){
		g = (group*) temp->data;
	
		printf("[%d] Group: %s | Users: ",count++,g->group_name);
	
		element* temp2 = g->user_list_head;
		while(temp2!=NULL){
			user* u = (user*) temp2->data;
			printf("%s,", u->login);
			temp2 = temp2->prev;
		}
	
		printf("\n");
		temp = temp->prev;
	}
	
	printf("-------------\n");
}

//SERVER WORK
int serve_client(element**client_head, element** group_head, user* u){
	
	int	count;

	unsigned char q;
	if ((count = read(u->socket, &q, sizeof(q))) <= 0){
		remove_user(client_head, group_head, u);
		return -1;
	}
	
	if (q == TYPE_QUERY_MESSAGE){
		message_to m; 
		if ((count = server_read_message(u->socket, &m)) <= 0){
			remove_user(client_head, group_head, u);
			return -1;
		}

		int res_sock;
		message_from mf;
		strcpy(mf.sender.login, u->login);
		strcpy(mf.message, m.message);

		if (m.recipient.destination_type == TYPE_SOURCE_GROUP){
			if (find_user_in_group(group_head, m.recipient.name, u) < 0) return 1;
			element* gr = find_element_by_group_name(group_head, m.recipient.name);
			group* g = (group*) gr->data;
			element* temp = g->user_list_head;

			mf.sender.source_type = TYPE_SOURCE_GROUP;
			strcpy(mf.sender.group_name, m.recipient.name);
			
			while (temp!=NULL){
				user* us = (user*)temp->data;
				if (u != us){
					res_sock = us->socket;
					if ((server_send_message(res_sock, &mf)) <= 0){
						perror("cant send message in serve_client");
					}
				}
				temp = temp->prev;
			}
		}else if(m.recipient.destination_type == TYPE_SOURCE_USER){
			mf.sender.source_type = TYPE_SOURCE_USER;
			if ((res_sock = find_socket_by_login(client_head, m.recipient.name)) < 0) return 1;
			if ((count = server_send_message(res_sock, &mf)) <= 0){
				perror("cant send message in serve_client");
				return 1;
			}
		}
	}else if(q == TYPE_QUERY_GO_TO_GROUP){
		char buf[LOGIN_LENGTH];
		if ((count = read_string(u->socket, buf, LOGIN_LENGTH)) <= 0){
			remove_user(client_head, group_head, u);
			return -1;
		}
		element* el;
		if ( (el = find_element_by_group_name(group_head, buf)) != NULL){
			add_user_to_group_by_group_name(group_head, buf,u);
		}else{
			add_group(group_head, buf, u);
		}
		print_group_list(group_head);
	}
	
}

int write_answer(int socket, unsigned char answer){
	if (write(socket, &answer, sizeof(answer)) <= 0){
		return -1;
	}
	return 1;
}

int server_login(element**client_head, int socket){
	char buf[LOGIN_LENGTH];
	if (read(socket, buf, LOGIN_LENGTH) <= 0){
		return -1;
	}

	if (find_element_by_login(client_head, buf) != NULL){
		write_answer(socket, TYPE_ANSWER_FAIL);
		return -1;
	}
	
	user *u;
	if ((u = add_user(client_head, buf, socket)) == NULL) return -1;
	write_answer(socket, TYPE_ANSWER_SUCCESS);
	
	printf("we added user: %s\n", u->login);
	print_user_list(client_head);

	return 1;
}
