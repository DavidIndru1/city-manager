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
#include <signal.h> // pentru semnale, restul sunt pasted din city manager

int main(){
    int pid = getpid();
    char path[256] = "";
    strcpy(path,".monitor_pid");
    int fd = open(path, O_WRONLY | O_CREAT | O_APPEND,644);
    if(fd == -1){
        printf("eroare deschidere fisier\n");
        exit(-1);
    }
    fprintf(fd, "%d", pid);
    close(fd);
}
