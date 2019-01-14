#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "utils.h"

char *add_newline(char *message)
{
    int position;
    char *new = (char *) calloc(strlen(message) + 2, 1);
    strcpy(new, message);

    position = (int) strcspn(new, "\n");

    if (position == (int) strlen(new))
    {
        strcat(new, "\n");
        return new;
    }

    free(new);
    return message;
}

char *remove_newline(char *message)
{
    char *new = message;
    new[strcspn(new, "\n")] = 0;

    return new;
}

char *convert_int_to_char(int value)
{
    char string[5];
    char *message;

    sprintf(string, "%d", value);
    message = string;

    return message;
}
