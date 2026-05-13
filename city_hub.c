#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>
#include <errno.h>
#include <sys/wait.h>
#include <signal.h>

void start_monitor(){
    int pfd[2];
    int hub_mon = fork();
    if(hub_mon < 0){
        printf("eroare fork() la hub mon!\n");
        exit(-1);
    }
    else if(hub_mon == 0){
        pipe(pfd);
        int monitor = fork();
        if(monitor > 0){
            printf("eroare la fork() la monitor\n");
            exit(-1);
        }
        else if(monitor == 0){
            struct stat vf;
        }
    }
}

int main(int argc, char *argv[]){

}
