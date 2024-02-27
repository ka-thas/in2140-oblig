#include <stdio.h>
#include <stdlib.h>
#include <string.h>

char *getCharArray()
{
    char *str = (char *)malloc(6 * sizeof(char));
    strcpy(str, "Hello");
    return str;
}

int main()
{
    char *result = getCharArray();
    printf("%s\n", result);
    free(result);
    return 0;
}