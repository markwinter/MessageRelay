#ifndef MESSAGE_LIST_H_
#define MESSAGE_LIST_H_

typedef struct item {
	char* message;
	struct item* next;
} Item;

typedef struct list {
	int message_count;
	Item *head, *end;
} MessageList;

MessageList* create_message_list();
void destroy_message_list(MessageList* m);
void add_message(MessageList* m, char *message);
void shift_list_head(MessageList* m, int amount);

#endif
