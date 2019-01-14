#ifndef UPS_BOARD_H
#define UPS_BOARD_H

#include "card.h"

#define BOARD_SIZE 4
#define CARD_COUNT (BOARD_SIZE * BOARD_SIZE)
#define PAIR_COUNT (CARD_COUNT / 2)

int** generate_board();

void fill_board(int **board);

int is_board_empty(int **board);

int get_card(int X, int Y, int **board);

void return_card(int X, int Y, int value, int **board);

void remove_card(int X, int Y, int **board);

int check_move(Card *first, Card *second);

int pair_check(int first, int second);

void draw_board(int **board);

void free_board(int **board);


#endif
