#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/select.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>
#include <pthread.h>

#include "board.h"
#include "card.h"
#include "game.h"
#include "player.h"
#include "server.h"
#include "utils.h"

#define MAX_GAMES 255
#define MAX_PLAYERS (PLAYERS_COUNT * MAX_GAMES)
#define BUFFER 255
#define MESSAGE_LENGTH 3
#define DEBUG 1


pthread_mutex_t mutex_count;
pthread_mutex_t mutex_serve;

void get_next_round(Game *game, const int *player_turn)
{
    int turn = *player_turn;
    int last_turn = turn;

    do
    {
        turn = (turn + 1) % PLAYER_COUNT;

        if (turn == last_turn)
        {
            game->running = 0;
            break;
        }
    }
    while (game->players[turn]->connected != 1);
}

void find_winner(Game *game)
{
    int best_score = 0 , winner = 0;
    int i;

    for (i = 0; i < PLAYER_COUNT; i++)
    {
        if (game->players[i]->score > best_score)
        {
            best_score = game->players[i]->score;
            winner = i;
        }
    }

    for (i = 0; i < PLAYER_COUNT; i++)
    {
        if (i != winner)
        {
            write_client_message(game->players[i]->client_socket, "LSE");
        }
        else
        {
            write_client_message(game->players[i]->client_socket, "WIN");
        }
    }
}

void *run_game(void *thread_data)
{
    int game_over = 0, player_turn = 0, turn_count = 0;
    int valid = 0 , pair = 0;
    int client_socket;
    Card *first_card = NULL, *second_card = NULL;

    Game *game = (Game *) thread_data;
    int **board = generate_board();
    game->board = board;
    game->running = 1;

    printf("Game stars!\n");

    write_clients_message(game->client_socket, "SRT");

    if (DEBUG)
    {
        printf("[DEBUG] Sent start message to clients in game %d.\n", game->id);
    }

    draw_board(board);

    while (!game_over)
    {
        do
        {
            valid = 0;
            client_socket = game->client_socket[player_turn];

            print_player(game->players[player_turn]);

            first_card = get_player_move(client_socket, board);

            if (!is_player_in_game(game->players[player_turn]))
            {
                get_next_round(game, &player_turn);
                continue;
            }

            send_update(game->client_socket, first_card);

            while (!valid)
            {
                second_card = get_player_move(client_socket, board);

                if ((valid = check_move(first_card, second_card)) == 0)
                {
                    write_client_message(client_socket, "INV");
                }

                if (!is_player_in_game(game->players[player_turn]))
                {
                    get_next_round(game, &player_turn);
                    continue;
                }
            }

            send_update(game->client_socket, second_card);
            pair = pair_check(first_card->value, second_card->value);

            if (pair == 1)
            {
                remove_card(first_card->X, first_card->Y, board);
                remove_card(second_card->X, second_card->Y, board);
                game->cards = game->cards - 2;
                game->players[player_turn]->score++;
            }

            draw_board(board);
            send_pair(game->client_socket, first_card, second_card, pair);

        } while (pair);

        turn_count++;

        get_next_round(game, &player_turn);

        if (game->running == 0 || game->cards == 0)
        {
            game_over = 1;
            find_winner(game);
        }
    }

    printf("Game over.\n");

    pthread_exit(NULL);
}

void *serve_player(void *thread_data)
{
    char *name;
    int client_socket = *((int *) thread_data);

    pthread_mutex_lock(&mutex_serve);

    name = get_player_name(client_socket);
    choose_game_type(name, client_socket, 1);

    free(name);

    check_games_to_start();

    pthread_mutex_unlock(&mutex_serve);

    pthread_exit(NULL);
}

void error(const char *message)
{
	perror(message);
	pthread_exit(NULL);
}

char* receive_message(int client_socket)
{
    char *message;
    int n;

    message = calloc(BUFFER, 1);

    n = (int) read(client_socket, message, BUFFER);

    if (n < 0)
    {
        error("Client disconnected\n");
        return NULL;
    }

    message = remove_newline(message);

    if (DEBUG)
    {
        printf("[DEBUG] received message: %s from id %d.\n", message, client_socket);
    }

    return message;
}

int receive_int(int client_socket)
{
    char message[10];
    int value;

    int n = (int) read(client_socket, message, 10);

    if (n <= 0)
    {
        set_player_disconnected(&players, client_socket);
        return -1;
    }

    value = message[0] - '0';

    if (value > BOARD_SIZE)
    {
        return -1;
    }

    if (DEBUG)
    {
        printf("[DEBUG] Received int: %d from id %d..\n", value, client_socket);
    }

    return value;
}

void write_client_message(int client_socket, char *message)
{
    int n;
    message = add_newline(message);

    n = (int) send(client_socket, message, strlen(message), MSG_NOSIGNAL);
    if (n <= 0)
    {
        set_player_disconnected(&players, client_socket);
        return;
    }

    if (DEBUG)
    {
        printf("[DEBUG] Message %s sent to id %d.\n", remove_newline(message), client_socket);
    }

    free(message);
}

void write_clients_message(int *client_socket, char *message)
{
    int i, client;

    for (i = 0; i< PLAYER_COUNT; i++)
    {
        client = client_socket[i];

        if (client != -1)
        {
            write_client_message(client, message);
        }
    }
}

void write_client_int(int client_socket, int value)
{
    int n;
    char *message;

    message = convert_int_to_char(value);
    message = add_newline(message);

    n = (int) send(client_socket, message, strlen(message), MSG_NOSIGNAL);
    if (n < 0)
    {
        set_player_disconnected(&players, client_socket);
        return;
    }

    if (DEBUG)
    {
        printf("[DEBUG] Sent int: %s to %d.\n", remove_newline(message), client_socket);
    }
}

void write_clients_int(int *client_socket, int value)
{
    int i, client;

    for (i = 0; i < PLAYER_COUNT; i++)
    {
        client = client_socket[i];

        if (client != -1)
        {
            write_client_int(client, value);
        }
    }
}

Card* get_player_move(int client_socket, int **board)
{
    int valid = 0;
    Card *card = (Card *) calloc(sizeof(card), 1);

    if (DEBUG)
    {
        printf("[DEBUG] Getting player %d move.\n", client_socket);
    }

    write_client_message(client_socket, "TRN");

    while (!valid)
    {
        card->X = receive_int(client_socket);
        card->Y = receive_int(client_socket);

        if (card->X < 0 || card->Y < 0)
        {
            printf("Client %d not responding.\n", client_socket);
            return card;
        }

        if (card->X < BOARD_SIZE && card->Y < BOARD_SIZE && board[card->X][card->Y] != -1)
        {
            valid = 1;
        }
        else
        {
            valid = 0;
            write_client_message(client_socket, "INV");
        }
    }


    card->value = get_card(card->X, card->Y, board);

    return card;
}

/* void send_player_count(int client_socket)
{
    write_client_message(client_socket, "CNT");
    write_client_int(client_socket, player_count);

    if (DEBUG)
    {
        printf("[DEBUG] Players count sent.\n");
    }
}
*/

void send_update(int *client_socket, Card *card)
{
    if (DEBUG)
    {
        printf("[DEBUG] Sending update.\n");
    }

    write_clients_message(client_socket, "UPD");

    write_clients_int(client_socket, card->X);
    write_clients_int(client_socket, card->Y);
    write_clients_int(client_socket, card->value);

    if (DEBUG)
    {
        printf("[DEBUG] Update send.\n");
    }
}

void send_pair(int *client_socket, Card *first, Card *second, int is_pair)
{
    if (DEBUG)
    {
        printf("[DEBUG] Sending pair.\n");
    }

    write_clients_message(client_socket, "DUO");

    write_clients_int(client_socket, is_pair);

    write_clients_int(client_socket, first->X);
    write_clients_int(client_socket, first->Y);

    write_clients_int(client_socket, second->X);
    write_clients_int(client_socket, second->Y);

    if (DEBUG)
    {
        printf("[DEBUG] Pair send.\n");
    }
}

int setup_listener(int port)
{
    int sockfd;
    struct sockaddr_in server_address;

    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd < 0)
    {
        error("Error opening listener socket.");
    }
    if (DEBUG)
    {
        printf("[DEBUG] Socket created.\n");
    }

    memset(&server_address, 0, sizeof(struct sockaddr_in));

    server_address.sin_family = AF_INET;
    server_address.sin_port = htons((uint16_t) port);
    server_address.sin_addr.s_addr = INADDR_ANY;

    if (bind(sockfd, (struct sockaddr *) &server_address, sizeof(server_address)) < 0)
    {
        error("Binding ERROR\n");
    }

    if (DEBUG)
    {
        printf("[DEBUG] Binding OK.\n");
        printf("[DEBUG] Listener set.\n");
    }

    return sockfd;
}

char* get_player_name(int client_socket)
{
    char *name;

    write_client_message(client_socket, "NME");

    name = calloc(BUFFER, 1);
    strcpy(name, receive_message(client_socket));

    while (is_name_unique(&players, name) != 1)
    {
        write_client_message(client_socket, "NKO");
        memset(name, '\0', strlen(name));
        strcpy(name, receive_message(client_socket));
    }

    write_client_message(client_socket, "NOK");

    return name;
}

void choose_game_type(char *name, int client_socket, int first_game)
{
    char *buffer;
    int game_id;
    Game *game;
    Player *player;

    buffer = calloc(1, BUFFER);
    strcpy(buffer, receive_message(client_socket));

    player = add_player(&players, name, client_socket);

    if (strcmp(buffer, "NEW") == 0)
    {
        game = add_game(&games);
        player_join_game(game, player);
    }
    else if (strcmp(buffer, "OLD") == 0)
    {
        if ((game_id = find_empty_game(&games)) != -1)
        {
            player_join_game(&games[game_id], player);
        }
    }
    else if (strcmp(buffer, "EXT") == 0)
    {
        close(client_socket);
    }
    else
    {
        printf("Illegal option of game\n");
        return;
    }

    write_client_message(client_socket, "HLD");
    print_all_games(&games);
}

void check_games_to_start()
{
    int game_id;
    pthread_t thread;

    game_id = find_ready_game(&games);
    if (game_id != -1)
    {
        pthread_create(&thread, NULL, run_game, (void *) &games[game_id]);
    }
}

int main(int argc, char *argv[])
{
    char buffer[1024];
    int port;
    int server_socket;
    int client_socket;
    int return_value;
	int fd;
	int max_fd;
    socklen_t length_address;

	pthread_t server_t;

	struct sockaddr_in peer_addr;

    fd_set fds, client_sockets;

	if (argc < 2)
	{
	        printf("Not enough parameters\n");
	        return 0;
	}

    /* set up port */
    port = (int) strtol(argv[1], NULL, 0);

    server_socket = setup_listener(port);

    return_value = listen(server_socket, 10);

    if (return_value == 0)
    {
        printf("Listening OK\n");
    }
    else
    {
        printf("Listening error\n");
        return -1;
    }

    FD_ZERO(&client_sockets);
    FD_SET(server_socket, &client_sockets);
    FD_SET(0, &client_sockets);

	max_fd = server_socket;
    pthread_mutex_init(&mutex_serve, NULL);

    while (1)
    {
	    memset(&buffer, 0, sizeof(buffer));

        fds = client_sockets;

        return_value = select(FD_SETSIZE, &fds, (fd_set *) 0, (fd_set *) 0, (struct timeval *) 0);

        if (return_value < 0)
	    {
		    error("Select - ERR\n");
	    }

        if (FD_ISSET(0, &fds))
        {
            read(0, buffer, sizeof(buffer));
            if (strcmp(buffer, "quit") == 0)
            {
                exit(0);
            }
        }

        for (fd = 3; fd < FD_SETSIZE; fd++)
        {
            if (FD_ISSET(fd, &fds))
            {
                if (fd == server_socket)
                {
                    length_address = sizeof(peer_addr);
                    client_socket = accept(server_socket, (struct sockaddr *) &peer_addr, &length_address);
                    FD_SET(client_socket, &client_sockets);
                    max_fd++;
                    printf("New client connected to fd %d.\n", client_socket);

                    pthread_create(&server_t, NULL, serve_player, (void *) &client_socket);
                }
            }
        }
    }
}
