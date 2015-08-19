#include "messagerelay.h"
#include "message_list.h"

#define MAX_INPUT 100

int ROW, COL;
int keep_running = 1;
Tox *tox;
char user_input[MAX_INPUT];
MessageList* message_list;
int offlineonly = 0;
unsigned char* relay_key;

void on_friend_request(Tox *m, const uint8_t *public_key, const uint8_t *data, uint16_t length, void *userdata) {
	/* Auto accept all incoming friend requests */
	tox_add_friend_norequest(m, public_key);
	add_message(message_list, "Received friend request");	
}

void on_connection_change(Tox *m, int32_t friendnumber, const uint8_t status, void *userdata) {
	int friend_number = tox_get_friend_number(m, relay_key);
	
	/* If user is coming online, send stored messages */
	if (status == 1 && friendnumber == friend_number) {
		add_message(message_list, "Relay has come online - sending any stored messages");

		FILE* file = fopen("saved_messages", "r");

		if (file) {
			int c;
			char line[1368];
			int counter = 0;

			while ((c = fgetc(file)) != EOF) {
				if ((char) c == '\n') {
					char message[counter];
					memcpy(message, line, counter);	
					tox_send_message(m, friendnumber, (const uint8_t*) message, counter);
					counter = 0;
				}

				line[counter] = c;

				counter++;	
			}
			
			/* Clear the file contents */
			freopen("saved_messages", "w", file);

			fclose(file);
		}
	}
}

void on_message(Tox *m, int32_t friendnumber, const uint8_t *string, uint16_t length, void *userdata) {
	int relay_number = tox_get_friend_number(m, relay_key);

	/* Get the name of the sender */
	char tmp[TOX_MAX_NAME_LENGTH];	
	int name_len = tox_get_name(m, friendnumber, (uint8_t*) tmp);
	tmp[name_len] = '\0';
	// TODO: Check for name_len being -1
	char name[name_len+1]; 	
	memcpy(name, tmp, name_len+1);

	const char* seperator = " | ";

	int message_len = name_len+1 + strlen(seperator) + length;

	// TODO: Check if message_len > 1368 and split into separate messages if it is

	/* Combine all into one message */
	char message[message_len];
	snprintf(message, message_len, "%s%s%s", name, seperator, (const char*) string);	

	/* If friend is online forward message */
	if (tox_get_friend_connection_status(tox, relay_number) == 1 && offlineonly != 1) {
		tox_send_message(m, relay_number, (const uint8_t*) message, message_len);
	} 

	/* Store the message */	
	else {
		FILE* file = fopen("saved_messages", "a");

		if (file) {
			fwrite(message, sizeof(char), message_len, file);
			fwrite("\n", sizeof(char), 1, file);
			fclose(file);
		}
	}
}

void print_title() {
	attron(A_BOLD | A_UNDERLINE);
	mvprintw(0, 0, "%s", "Tox Message Relay");
	attroff(A_BOLD | A_UNDERLINE);
}

void print_bottom_bar() {
	move(ROW-2, 0);
	hline(ACS_HLINE, COL);
}

void print_help() {
	add_message(message_list, "Commands");
	add_message(message_list, "/help - prints this message");
	add_message(message_list, "/id - print the tox id of this relay");
	add_message(message_list, "/name - sets your name");	
	add_message(message_list, "/addfriend <tox id> - adds tox id as a friend");
	add_message(message_list, "/addrelay <tox id> - adds tox id as message destination (and as a friend)");
	add_message(message_list, "/offlineonly <1|0> - 1 will cause the relay to only store messages. Default 0");
	add_message(message_list, "/clear - clears the screen");
	add_message(message_list, "/quit - exits the client");
}

void save_data() {
	FILE* file = fopen("tox_save", "w");

	if (file) {
		size_t size = tox_size(tox);
		uint8_t data[size];
		tox_save(tox, data);

		fwrite(data, sizeof(uint8_t), size, file);
		fclose(file);
	}
}

void load_data() {
	FILE* file = fopen("tox_save", "r");

	if (file) {
		fseek(file, 0, SEEK_END);
		size_t size = ftell(file);
		rewind(file);

		uint8_t data[size];

		fread(data, sizeof(uint8_t), size, file);
		
		tox_load(tox, data, size);

		fclose(file);
	} 
	
	else 
		save_data();
	
}

void load_relay() {
	FILE* file = fopen("relay", "r");

	if (file) {
		char id[76];
		while (fscanf(file, "%s", id) != EOF);
		fclose(file);		

		/* Get just the public key part of the id */
		char key[64];
		strncpy(key, id, 64);

		relay_key = hex_string_to_bin(key);
	}
}

uint8_t* hex_string_to_bin(char* string) {
	size_t i, len = strlen(string) / 2;
	uint8_t* ret = malloc(len);
	char* pos = string;

	for (i = 0; i < len; ++i, pos += 2)
		sscanf(pos, "%2hhx", &ret[i]);

	return ret;
}

/* Allocates new tox structure and bootstraps 
 * Returns 0 on success
 * Returns -1 on failure
 */
int init_tox() {
	Tox_Options options;
	options.ipv6enabled = 1;
	options.udp_disabled = 0;
	options.proxy_type = TOX_PROXY_NONE;

	tox = tox_new(&options);

	/* Failed creating new tox */
	if (!tox)
		return -1;

	load_data();

	tox_callback_connection_status(tox, on_connection_change, NULL);
	tox_callback_friend_message(tox, on_message, NULL);
	tox_callback_friend_request(tox, on_friend_request, NULL);

	tox_set_name(tox, (uint8_t*) "Message Relay", strlen("Message Relay"));

	const char* address = "178.62.125.224";
	uint16_t port = 33445;
	unsigned char* binary_string = hex_string_to_bin("04119E835DF3E78BACF0F84235B300546AF8B936F035185E2A8E9E0A67C8924F");
	int res = tox_bootstrap_from_address(tox, address, port, binary_string);
	free(binary_string);

	/* Failed bootstrapping */
	if (res != 1)
		return -1;

	return 0;
}

void shutdown() {
	save_data();
	tox_kill(tox);
	endwin();
}

void evaluate_input() {
	/* If first char is a '/' then user is entering a command */
	if ((char) user_input[0] == '/') {
		int counter = 1;
		int command = 0;

		/* Find the length of the command by searching for the space */
		while (user_input[counter] != 0) {
			if (user_input[counter] == 32)
				break;

			command += user_input[counter];
			counter++;
		}

		if (command == 425) { // help
			print_help();
		}

		else if (command == 519) { // clear
			destroy_message_list(message_list);
			message_list = create_message_list();
		}

		else if (command == 205) { // id
			char* id = (char*) malloc(TOX_FRIEND_ADDRESS_SIZE * 2 * sizeof(char) + 1);
			char address[TOX_FRIEND_ADDRESS_SIZE];
			tox_get_address(tox, (uint8_t*) address);

			for (size_t i = 0; i < TOX_FRIEND_ADDRESS_SIZE; ++i) {
				char a[3];
				snprintf(a, sizeof(a), "%02X", address[i] & 0xff);
				strcat(id, a);
			}
			
			add_message(message_list, id);	
	
			// message_list is in charge of freeing id so we dont have to here
		}	

		else if (command == 417) { // name 
			int index = 0;
			int length = 0;

			while (user_input[index] != 0) {
				length++;
				index++;
			}

			/* Check for valid length of name */
			if (length < 1 || length > TOX_MAX_NAME_LENGTH) {
				add_message(message_list, "Invalid name given");
				return;
			}


			char name[length];
			strncpy(name, &user_input[counter + 1], length);
			tox_set_name(tox, (const uint8_t*) name, length);

			add_message(message_list, "Successfully set new name");
		}

		else if (command == 929 || command == 838) { // addfriend or addrelay
			/* Extract Tox ID from the command parameters */
			int total = 0;
			int index = counter + 1;

			while (user_input[index] != 0) {
				if (user_input[index] == 32)
					break;

				total++;
				index++;
			}

			if (total != 76) {
				add_message(message_list, "Invalid Tox ID length");
			} 
			
			else {
				char id[76];
				strncpy(id, &user_input[counter + 1], 76);

				if (command == 838) {
					/* Append Tox ID to end of csv relay file */
					FILE* file = fopen("relay", "w");

					if (file) {
						fwrite(id, sizeof(char), 76, file);
						fclose(file);
					}	

					add_message(message_list, "Added relay");	
				}

				unsigned char* address = hex_string_to_bin(id);

				int res = tox_add_friend(tox, address, (uint8_t*) "Message Relay", sizeof("Message relay"));

				if (res >= 0)
					add_message(message_list, "Friend added");
				else
					add_message(message_list, "Error adding friend");
			}
		}

		else if (command == 451) { // quit
			keep_running = 0;	
		} 

		else if (command == 1189) { // offlineonly 
			char toggle[1];
			strncpy(toggle, &user_input[counter + 1], 1);

			if (toggle[0] == '1')
				offlineonly = 1;
			else 
				offlineonly = 0;	
		}

		else 
			add_message(message_list, "Type /help for a list of commands");
	} 

	else
		add_message(message_list, "Type /help for a list of commands");

	/* Just for formatting purposes. Add a line between messages/commands */
	add_message(message_list, "");
}

void refresh_screen() {
	erase();

	print_title();
	print_connection();
	print_bottom_bar();
	print_offlineonly();

	/* Print the message list to screen */
	Item* curr = message_list->head;
	int counter = 0;
	int offset = 2;

	/* If too many messages to display, then remove the top messages until it can fit */
	if (message_list->message_count > ROW - 5)
		shift_list_head(message_list, (message_list->message_count - ROW - 5));

	while (curr != 0) {
		mvprintw(offset + counter, 0, "%s", curr->message);
		curr = curr->next;
		counter++;
	}

	mvprintw(ROW-1, 0, "%s", user_input);
}

void print_offlineonly() {
	char* message;

	if (offlineonly)
		message = "[ Offline Only ]";
	else
		message = "[ Online/Offline On ]";

	attron(A_BOLD);
	mvprintw(1, COL-strlen(message), "%s", message);
	attroff(A_BOLD);
}

void print_connection() {
	char* message;
	int type;

	if (tox_isconnected(tox)) {
		type = 1;
		message = "[ Connected ]";
	} else { 
		type = 2;
		message = "[ Disconnected ]";
	}

	attron(COLOR_PAIR(type) | A_BOLD);
	mvprintw(0, COL-strlen(message), "%s", message);
	attroff(COLOR_PAIR(type) | A_BOLD);
}

void loop() {
	int counter = 0;

	while(keep_running) {
		tox_do(tox);

		refresh_screen();

		/* Process user input */
		int c = getch();
		if (c == ERR) // nodelay() in main() sets getch() to ERR if no user input was available
			continue;

		else if (c == 3) // Ctrl-C
			keep_running = 0;	

		else if (c == 8 || c == 127) {	// Backspace
			if (counter > 0) {
				counter--;
				user_input[counter] = 0;	
			}
		}

		else if (isalnum(c) || c == 32 || ispunct(c)) { // alphanumeric, space, punctuation
			// Make sure we only accept MAX_INPUT many chars
			if (counter < MAX_INPUT) {	
				user_input[counter] = c;
				counter++;
			}
		}
	
		else if (c == 13 || c == 10) { // enter
			user_input[counter] = 0;
			evaluate_input();
			memset(&user_input[0], 0, sizeof(user_input));	
			counter = 0;
		}	
	}
}


int main() {
	initscr();
	noecho();
	raw();
	timeout(50);	
	getmaxyx(stdscr, ROW, COL);
	
	start_color();
	init_pair(1, COLOR_GREEN, COLOR_BLACK);
	init_pair(2, COLOR_RED, COLOR_BLACK);

	int res = init_tox();

	/* Failed to init tox in some way */
	if (res == -1)
		return EXIT_FAILURE;

	message_list = create_message_list();	

	if (!message_list)
		return EXIT_FAILURE;

	add_message(message_list, "Type /help for a list of commands");
	add_message(message_list, "");

	load_relay();

	loop();	

	destroy_message_list(message_list);

	shutdown();

	return EXIT_SUCCESS;
}
