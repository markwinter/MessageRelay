#ifndef MESSAGE_RELAY_H_
#define MESSAGE_RELAY_H_

void print_title();
void print_offlineonly();
void print_bottom_bar();
void print_help();
void save_data();
void load_data();
void refresh_screen();
void print_connection();
void loop();
void evaluate_input();
void shutdown();
int init_tox();
uint8_t* hex_string_to_bin(char* string);

void on_connection_change(Tox *m, int32_t friendnumber, uint8_t status, void *userdata);
void on_message(Tox *m, int32_t friendnumber, const uint8_t *string, uint16_t length, void *userdata);
void on_friend_request(Tox *m, const uint8_t *public_key, const uint8_t *data, uint16_t length, void *userdata);

#endif
