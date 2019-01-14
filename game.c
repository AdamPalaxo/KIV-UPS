#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <string.h>

#include "board.h"
#include "constant.h"
#include "game.h"


Game* add_game(Game **head)
{
	Game *new, *current;

	new = (Game *) malloc(sizeof(Game));
	if (NULL != new)
	{
		new->id = find_unique_id(head);
		new->cards = CARD_COUNT;
		new->client_socket = (int *) calloc(PLAYER_COUNT, sizeof(int));
		new->running = 0;
		new->player_count = 0;
		new->next = NULL;
	}
	else
	{
		fprintf(stderr, "Out of memory!");
		exit(1);
	}

	if (NULL == *head)
	{
		new->next = *head;
		*head = new;
	}
	else
	{
		current = *head;

		while (NULL != current->next)
		{
			current = current->next;
		}	

		new->next = current->next;
		current->next = new;	
	}

    return new;
}

int find_unique_id(Game **head)
{
    Game *current = *head;

	int i = 0;

    while (NULL != current)
    {
    	i = current->id;
        current = current->next;
    }

    return ++i;
}

int find_ready_game(Game **head)
{
    int i = 0;
	Game *current = NULL;

	current = *head;

	while (NULL != current)
	{
		if (current->player_count == PLAYER_COUNT && current->running == 0)
		{
			return i;
		}
		current = current->next;
	}

	return -1;
}

int find_empty_game(Game **head)
{
	Game *current = *head;
	int i = 0;

	while (NULL != current)
	{
		if (current->player_count < PLAYER_COUNT && current->running == 0)
		{
			return i;
		}
		current = current->next;
	}

	return -1;
}

void print_all_games(Game **head)
{
	int i;
	Player *player;
	Game *current = *head;

	if (NULL == current)
	{
		printf("Game's list is empty.\n");
		return;
	}

	while (NULL != current)
	{
		printf("Game %d\nPlayers\n-------\n", current->id);
		for (i = 0; i < PLAYER_COUNT; i++)
		{
			player = current->players[i];
			if (NULL != player)
			{
				printf("Player [%d] - \"%s\"\n", i, player->name);
			}
			else
			{
				printf("Player [%d] - NULL\n", i);
			}
		}
		printf("\n");

		current = current->next;
	}

}

int player_join_game(Game *game, Player *player)
{
	int i;

	for (i = 0; i < PLAYER_COUNT; i++)
	{
		if (NULL == game->players[i])
		{
			player->game = game;

			game->players[i] = player;
			game->client_socket[i] = player->client_socket;
			game->player_count++;
			return 1;
		}
	}

	return -1;
}

void player_disconnect_game(Game *game, Player *player)
{
	int i;

	for (i = 0; i < PLAYER_COUNT; i++)
	{
		if (player->client_socket == game->client_socket[i])
		{
			game->client_socket[i] = -1;
		}
	}

	if (DEBUG)
	{
		printf("[DEBUG] Player %d disconnected from game %d.\n", player->client_socket, game->id);
	}
}

void free_games(Game **head)
{
	Game *current = *head;

	while (NULL != current)
	{
		free(current);
		current = current->next;
	}

	free(head);
}

