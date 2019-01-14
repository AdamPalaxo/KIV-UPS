#ifndef UPS_PLAYER_H
#define UPS_PLAYER_H

typedef struct Player{
	char name[50];
	int score;
    int connected;
	int client_socket;
	struct Game *game;
	struct Player *next;
} Player;

Player* add_player(Player **head, char *name, int client_socket);

int is_name_unique(Player **head, char *name);

void set_player_disconnected(Player **head, int client_socket);

int is_player_in_game(Player *player);

void print_player(Player *player);

void print_all_players(Player **head);

void free_players(Player **head);

#endif
