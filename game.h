#ifndef UPS_GAME_H
#define UPS_GAME_H

#define PLAYER_COUNT 2

#include "player.h"

typedef struct Game{
	int id;
	int player_count;
	int running;
	int cards;
	int *client_socket;
	int **board;
	Player *players[6];
	struct Game *next;
} Game;

Game* add_game(Game **head);

int find_unique_id(Game **head);

int find_ready_game(Game **head);

int find_empty_game(Game **head);

void print_all_games(Game **head);

int player_join_game(Game *game, Player *player);

void player_disconnect_game(Game *game, Player *player);

void free_games(Game **head);

#endif
