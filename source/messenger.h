#ifndef MESSENGER_H
#define MESSENGER_H

//LENGTHS
#define LOGIN_LENGTH 30
#define MESSAGE_TEXT_LENGTH 300

//QUERY TYPE
#define TYPE_QUERY_MESSAGE 1
#define TYPE_QUERY_LOGIN 2
#define TYPE_QUERY_GO_TO_GROUP 3
#define TYPE_QUERY_CONFIRMATION 4

//ANSWER TYPE
#define TYPE_ANSWER_SUCCESS 1
#define TYPE_ANSWER_FAIL 2

//SOURCE TYPES
#define TYPE_SOURCE_USER 1
#define TYPE_SOURCE_GROUP 2

typedef struct message_from   //Сообщение от сервера клиенту
{
	struct sender{
		unsigned char source_type;
		char group_name[LOGIN_LENGTH];
		char login[LOGIN_LENGTH];
	} sender;
	char message[MESSAGE_TEXT_LENGTH];
}message_from;

typedef struct message_to    //Сообщение клиента серверу
{
	struct recipient{
		unsigned char destination_type;
		char name[LOGIN_LENGTH];
	} recipient;
	char message[MESSAGE_TEXT_LENGTH];
}message_to;

int read_string(int fd, char* string, int max_len); 

int server_read_message(int sockfd, message_to* m);
int server_send_message(int sockfd, message_from* m);
int client_send_message(int sockfd, message_to* m);
int client_read_message(int sockfd, message_from* m);

#endif
