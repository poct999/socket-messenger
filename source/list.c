#include "list.h"
#include <stdlib.h>
#include <stdio.h>

int clear_list(element** head)
{
    while(pop_element(head, NULL) != NULL);
    return 1;
}

element* push_element(element **head, void* data)
{
	element *new_elem = (element*) malloc(sizeof(element));
	if(new_elem == NULL){
        perror("memory error");
        exit(1);
    }
	new_elem->data = data;
	new_elem->prev = (*head);
	(*head) = new_elem;

	return new_elem;
}

int get_list_count(element** head)
{
    element* temp = (*head);
    int count = 0;
    while (temp != NULL){
        count++;
        temp = temp->prev;
    }   
    return count;
}

void* pop_element(element **head, element* pop_element)
{
	if (pop_element == NULL) pop_element = (*head);
	
    if ((*head) == NULL) {
        return NULL;
    }
    
    element* temp;
    if (pop_element == (*head)){
        (*head) = (*head)->prev;        
    }else{
    	element* prev = pop_element->prev;
        temp = (*head); 
        while (temp->prev != pop_element){
        	temp = temp->prev;
        	if(temp->prev == NULL){
        		perror("List(pop): element not found! \n");
            	return NULL;
        	}
        }
        temp->prev = prev;
    }
    void* data = pop_element->data;
    free(pop_element);

    return data;
}
