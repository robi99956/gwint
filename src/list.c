#include <stdlib.h>
#include <string.h>

#include "list.h"

list_handle_t list_create( uint16_t data_size )
{
	list_handle_t list = (list_handle_t)malloc( sizeof(list_t) );
	if( list == NULL ) return NULL;

	list->begin = list->end = list->actual = NULL;
	list->data_size = data_size;
	list->size = 0;

	return list;
}

list_element_t * list_create_elem( uint16_t data_size, void * data )
{
	list_element_t * new = (list_element_t*)malloc( sizeof(list_element_t) ); // nowy element
	if( new == NULL ) return NULL;

	new->data = malloc(data_size);
	if( new->data == NULL )
	{
		free(new);
		return NULL;
	}

	memcpy(new->data, data, data_size); // dane nowego elementu

	return new;
}

list_status_t list_delete_elem( list_element_t * elem )
{
	if( elem == NULL ) return LIST_MEM_ERR;

	if( elem->data != NULL ) free( elem->data );
	free( elem );

	return LIST_OK;
}

list_status_t list_push_front( list_handle_t list, void * data )
{
	list_element_t * new = list_create_elem(list->data_size, data);
	if( new == NULL ) return LIST_MEM_ERR;

	new->prev = NULL;
	new->next = (struct _list_elem*)list->begin;

	if( list->begin == NULL ) // dodanie elementu do listy
	{
		list->end = new;
	}
	else
		list->begin->prev = (struct _list_elem*)new;

	list->begin = new;
	list->size++;

	return LIST_OK;
}

list_status_t list_push_back( list_handle_t list, void * data )
{
	list_element_t * new = list_create_elem(list->data_size, data);
	if( new == NULL ) return LIST_MEM_ERR;

	new->next = NULL;
	new->prev = (struct _list_elem*)list->end;

	if( list->begin == NULL ) // dodanie elementu do listy
	{
		list->begin = new;
	}
	else
		list->end->next = (struct _list_elem*)new;

	list->end = new;
	list->size++;

	return LIST_OK;
}

list_element_t * list_pop_front( list_handle_t list )
{
	list_element_t * ret = list->begin;

	if( ret == NULL ) return NULL;

	if( ret == list->actual ) list->actual = NULL;

	list_element_t * new_begin = (list_element_t *)list->begin->next;

	if( new_begin != NULL ) new_begin->prev = NULL;
	else
	{
		list->end = NULL; // lista nie ma ju� element�w
	}

	list->begin = new_begin;

	ret->next = NULL;
	list->size--;

	return ret;
}

list_element_t * list_pop_back( list_handle_t list )
{
	list_element_t * ret = list->end;

	if( ret == NULL ) return NULL;

	if( ret == list->actual ) list->actual = NULL;

	list_element_t * new_end = (list_element_t *)list->end->prev;

	if( new_end != NULL ) new_end->next = NULL;
	else
	{
		list->begin = NULL; // lista nie ma ju� element�w
	}

	list->end = new_end;

	ret->prev = NULL;
	list->size--;

	return ret;
}

list_element_t * list_begin( list_handle_t list )
{
	return list->begin;
}

list_element_t * list_end( list_handle_t list )
{
	return list->end;
}

list_element_t * list_actual( list_handle_t list )
{
	return list->actual;
}

uint32_t list_size( list_handle_t list )
{
	return list->size;
}

void list_to_begin( list_handle_t list )
{
	list->actual = list->begin;
}

void list_to_end( list_handle_t list )
{
	list->actual = list->end;
}

void list_to_next( list_handle_t list )
{
	if( list->actual )
	{
		list->actual = (list_element_t *)list->actual->next;
	}
}

void list_to_prev( list_handle_t list )
{
	if( list->actual )
	{
		list->actual = (list_element_t *)list->actual->prev;
	}
}

void list_clear( list_handle_t list )
{
	list_element_t * begin;

	while( (begin = list_pop_front(list)) )
	{
		list_delete_elem( begin );
	}
}

list_status_t list_delete_actual( list_handle_t list, next_actual_t next_actual )
{
	if( list->actual == NULL ) return LIST_ACTUAL_NULL;

	list_element_t * next = (list_element_t *)list->actual->next, * prev = (list_element_t *)list->actual->prev;

	if( prev != NULL )
	{
		prev->next = (list_element_t *)next;
	}
	else
	{
		list->begin = (list_element_t *)next;
	}

	if( next != NULL )
	{
		next->prev = prev;
	}
	else
	{
		list->end = prev;
	}

	list_delete_elem( list->actual );
	
	switch (next_actual)
	{
	case SET_TO_NEXT:
		list->actual = next;
		break;
	case SET_TO_NULL:
		list->actual = NULL;
		break;
	case SET_TO_BEGIN:
		list->actual = list->begin;
		break;
	case SET_TO_END:
		list->actual = list->end;
		break;
	case SET_TO_PREV:
		list->actual = prev;
		break;
	}

	return LIST_OK;
}

list_status_t list_goto( list_handle_t list, list_element_t * _elem )
{
	list_element_t * elem;
	LIST_FOR_EACH(elem, list, 
	{
		if( elem == _elem ) return LIST_OK;
	})

	return LIST_ITEM_NOT_FOUND;
}