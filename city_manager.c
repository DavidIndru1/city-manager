#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <fcntl.h>
#include <sys/stat.h>

typedef struct report
{
    int id;
    char name[30];
    float latitude;
    float longitude;
    char issue[30];
    int severity;
    time_t timestamp;
    char description[30];
}

void add(char *district)
{

}

int main(int argc, char *argv[])
{
    if(argc < 2)
    {
        printf("prea putine argumente\n");
        exit(-1);
    }

}
