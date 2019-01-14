#ifndef UPS_SERVER_H
#define UPS_SERVER_H

Game *games;
Player *players;

int player_count = 0;
int games_count = 0;

void write_client_message(int client_socket, char *message);

void write_clients_message(int *client_socket, char *message);

void write_client_int(int client_socket, int value);

void write_clients_int(int *client_socket, int value);

char* receive_message(int client_socket);

Card* get_player_move(int client_socket, int **board);

char* get_player_name(int client_socket);

void send_update(int *client_socket, Card *card);

void send_pair(int *client_socket, Card *first, Card *second, int is_pair);

void choose_game_type(char *name, int client_socket, int first_game);

void check_games_to_start();

#endif
