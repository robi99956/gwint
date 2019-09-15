#ifndef LISTA_LISTA_H
#define LISTA_LISTA_H

#include <stdint.h>

typedef struct _list_elem
{
	void * data;

	struct _list_elem *next, *prev;
} list_element_t;

typedef struct _list
{
	list_element_t * begin;
	list_element_t * end;
	list_element_t * actual;
	uint16_t data_size;
	uint32_t size;
} list_t, *list_handle_t;

typedef enum _next{SET_TO_BEGIN, SET_TO_END, SET_TO_NEXT, SET_TO_PREV, SET_TO_NULL} next_actual_t;
typedef enum _list_status{LIST_OK=0, LIST_MEM_ERR, LIST_ACTUAL_NULL, LIST_ITEM_NOT_FOUND, LIST_OTH_ERR} list_status_t;

list_handle_t list_create( uint16_t data_size );
list_status_t list_delete_elem( list_element_t * elem );
list_status_t list_push_front( list_handle_t list, void * data );
list_status_t list_push_back( list_handle_t list, void * data );
list_element_t * list_pop_front( list_handle_t list );
list_element_t * list_pop_back( list_handle_t list );

list_element_t * list_begin( list_handle_t list );
list_element_t * list_end( list_handle_t list );
list_element_t * list_actual( list_handle_t list );

void list_to_begin( list_handle_t list );
void list_to_end( list_handle_t list );
void list_to_next( list_handle_t list );
void list_to_prev( list_handle_t list );
list_status_t list_goto( list_handle_t list, list_element_t * _elem );

uint32_t list_size( list_handle_t list );

void list_clear( list_handle_t list );
list_status_t list_delete_actual( list_handle_t list, next_actual_t next_actual );

#define LIST_FOR_EACH(elem, list, action) {list_to_begin(list); \
	list_element_t * __element__; \
	while( (__element__ = list_actual(list))) \
	{ \
		elem = __element__->data; \
		action; \
		list_to_next(list); \
	} \
}
#endif // !LISTA_LISTA_H
