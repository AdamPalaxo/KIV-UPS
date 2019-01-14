#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "constant.h"
#include "game.h"
#include "player.h"

Player* add_player(Player **head, char *name, int client_socket)
{
	Player *new, *current;

	if (is_name_unique(head, name) == 0)
	{
		printf("Name %s already exists.\n", name);
		return NULL;
	}

	new = (Player *) malloc(sizeof(Player));
	if (NULL != new)
	{
		memset(new->name, '\0', sizeof(new->name));
		strcpy(new->name, name);
        new->score = 0;
        new->client_socket = client_socket;
        new->connected = 1;
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

int is_name_unique(Player **head, char *name)
{
	Player *current = *head;

	while (NULL != current)
	{
		if (strcmp(current->name, name) == 0)
		{
			return 0;
		}

		current = current->next;
	}
		
	return 1;
}

void set_player_disconnected(Player **head, int client_socket)
{
    Player *current = *head;

    while (NULL != current)
    {
        if (current->client_socket == client_socket)
        {
        	if (current->connected == 1)
			{
				current->connected = 0;
				player_disconnect_game(current->game, current);
			}
            break;
        }

        current = current->next;
    }
}

int is_player_in_game(Player *player)
{
	return player->connected;
}

void delete_player()
{

}

void print_player(Player *player)
{
	printf("Printing player %s.\n", player->name);
	printf("Is connected: %d.\n", player->connected);
	printf("Fd: %d.\n", player->client_socket);
	printf("Game id: %d.\n", player->game->id);
	printf("Score: %d.\n", player->score);
}

void print_all_players(Player **head)
{
	Player *current = *head;

	if (NULL == current)
	{
		printf("Player's list is empty.\n");
		return;
	}

	while (NULL != current)
	{
		printf("%s\n", current->name);
		current = current->next;
	}

}

void free_players(Player **head)
{
	Player *current = *head;

	while (NULL != current)
	{
		free(current);
		current = current->next;
	}

	free(head);
}
