#ifndef LIST_H
#define LIST_H

#include <stdlib.h>

typedef struct element
{
	void *data;
	struct element *prev;
} element;

int get_list_count(element** head); //Количество элементов в списке

element* push_element(element **head, void* data); //Создает новый элемент, задает ему data и возвращает указатель на него.

int clear_list(element** head); //Чистит список

void* pop_element(element** head, element* pop_element);  //Удаляет pop_element из списка, возвращает data.

#endif
