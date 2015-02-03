#include "message_list.h"
#include <stdlib.h>

MessageList* create_message_list() {
	MessageList* m = (MessageList*) malloc(sizeof(MessageList));
	
	if (!m)
		return (MessageList*) 0;

	m->head = 0;
	m->end = 0;

	return m;
}

void destroy_message_list(MessageList* m) {
	Item* curr_node = m->head;

	while (curr_node != 0) {
		Item* tmp = curr_node;
		curr_node = curr_node->next;
		free(tmp);
	}

	free(m);
}

void add_message(MessageList* m, char* message) {
	Item* item = (Item*) malloc(sizeof(Item));
	item->message = message;
	item->next = 0;

	if (m->head == 0)
		m->head = item;
	else 
		m->end->next = item;

	m->end = item;
}
