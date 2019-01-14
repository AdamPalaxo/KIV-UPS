#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#include "board.h"
#include "card.h"

#define DEBUG 1

int** generate_board()
{
    int i, **board;

    board = (int **) calloc(BOARD_SIZE, sizeof(int *));

    for (i = 0; i < BOARD_SIZE; i++)
    {
        board[i] = calloc(BOARD_SIZE, sizeof(int));
    }

    fill_board(board);

    return board;
}

void fill_board(int **board)
{
    int i, j, random;
    int cards[PAIR_COUNT] = {0};
    time_t t;

    srand((unsigned) time(&t));

    for (i = 0; i < PAIR_COUNT; i++)
    {
        cards[i] = 2;
    }

    for (i = 0; i < BOARD_SIZE; i++)
    {
        for (j = 0; j < BOARD_SIZE; j++)
        {
            random = rand() % PAIR_COUNT;

            while (cards[random] == 0)
            {
                random = rand() % PAIR_COUNT;
            }

            board[i][j] = random;
            cards[random] = cards[random] - 1;
        }
    }
}

int is_board_empty(int **board)
{
    int i, j;

    for (i = 0; i < BOARD_SIZE; i++)
    {
        for (j = 0; j < BOARD_SIZE; j++)
        {
            if (board[i][j] != 0)
            {
                return 0;
            }
        }
    }

    return 1;
}

int get_card(int X, int Y, int **board)
{
    if (X < BOARD_SIZE && Y < BOARD_SIZE)
    {
        return board[X][Y];
    }

    return -1;
}

int pair_check(int first, int second)
{
    if (first == second)
    {
        return 1;
    }

    return 0;
}

void return_card(int X, int Y, int value, int **board)
{
    if (X < BOARD_SIZE && Y < BOARD_SIZE)
    {
        board[X][Y] = value;
    }
}

void remove_card(int X, int Y, int **board)
{
    if (X < BOARD_SIZE && Y < BOARD_SIZE)
    {
        board[X][Y] = -1;
    }
}

int check_move(Card *first, Card *second)
{
    if (first->X < 0 || first->Y < 0 || second->X < 0 || second->Y < 0)
    {
        return -1;
    }


    if (first->X == second->X && first->Y == second->Y)
    {
        return 0;
    }

    return 1;
}

void draw_board(int **board)
{
    int i, j;

    for (i = 0; i < BOARD_SIZE; i++)
    {
        for (j = 0; j < BOARD_SIZE; j++)
        {
            if (DEBUG)
            {
                printf(" %d", board[i][j]);
                continue;
            }

            if (board[i][j] != -1)
            {
                printf(" -");
            }
            else
            {
                printf(" X");
            }
        }

        printf("\n");
    }

}

void free_board(int **board)
{
    int i;

    for (i = 0; i < BOARD_SIZE; i++)
    {
        free(board[i]);
    }

    free(board);
}

