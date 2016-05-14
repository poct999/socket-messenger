#include <stdio.h>
#include <string.h>

#include "messenger.h"

int read_string(int fd, char* string, int max_len)
{
	int count = 0; char c;
	
	while (count < max_len){
		if (read(fd, &c, 1) > 0){
			count++;
			*(string++) = c;
			if (c == '\0') break;
		}else{
			return -1;
		}
	}
	
	return count;
}


int client_read_message(int sockfd, message_from* m)
{
	int count, res = 0;
	if ((count = read(sockfd, &m->sender.source_type, sizeof(m->sender.source_type))) <= 0)
	{
		return -1;
	} 
	res += count;
	
	if (m->sender.source_type == TYPE_SOURCE_GROUP){
		
		if ((count = read_string(sockfd, m->sender.group_name, LOGIN_LENGTH)) <= 0)
		{
			return -1;
		}
		res += count;
	}
	
	if ((count = read_string(sockfd, m->sender.login, LOGIN_LENGTH)) <= 0)
	{
		return -1;
	} 
	res += count;
	
	if ((count = read_string(sockfd, m->message, MESSAGE_TEXT_LENGTH)) <= 0)
	{
		return -1;
	}
	res += count;		

	return res;
}

int server_send_message(int sockfd, message_from* m)
{
	int count, res = 0;
	
	if ((count = write(sockfd, &m->sender.source_type, sizeof(m->sender.source_type))) <= 0)
	{
		return -1;
	} 
	res += count;
	
	if (m->sender.source_type == TYPE_SOURCE_GROUP){
		if ((count = write(sockfd, m->sender.group_name, strlen(m->sender.group_name)+1)) <= 0)
		{
			return -1;
		} 
		res += count;
	}
	
	if ((count = write(sockfd, m->sender.login, strlen(m->sender.login)+1)) <= 0)
	{
		return -1;
	} 
	res += count;
	
	if ((count = write(sockfd, m->message, strlen(m->message)+1)) <= 0)
	{
		return -1;
	}
	res += count;		

	return res;
}
int client_send_message(int sockfd, message_to* m)
{
	unsigned char k;
	k = TYPE_QUERY_MESSAGE;
	if (write(sockfd, &k, sizeof(k)) <= 0) 
	{
		return -1;
	}
	
	int count, res = 0;
	
	if ((count = write(sockfd, &m->recipient.destination_type, sizeof(m->recipient.destination_type))) <= 0)
	{
		return -1;
	} 
	res += count;
	
	if ((count = write(sockfd, m->recipient.name, strlen(m->recipient.name)+1)) <= 0)
	{
		return -1;
	} 
	res += count;
	
	if ((count = write(sockfd, m->message, strlen(m->message)+1)) <= 0)
	{
		return -1;
	}
	res += count;		

	return res;
}


int server_read_message(int sockfd, message_to* m)
{
	int count, res = 0;
	
	if ((count = read(sockfd, &m->recipient.destination_type, sizeof(m->recipient.destination_type))) <= 0)
	{
		return -1;
	} 
	res += count;
	
	if ((count = read_string(sockfd, m->recipient.name, LOGIN_LENGTH)) <= 0)
	{
		return -1;
	} 
	res += count;
	
	if ((count = read_string(sockfd, m->message, MESSAGE_TEXT_LENGTH)) <= 0)
	{
		return -1;
	}
	res += count;		

	return res;
}

