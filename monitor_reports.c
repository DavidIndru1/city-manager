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

//o variabila globala pe care o modific cand primesc SIGINT
int keep_running = 1;

//rutina de tratare a semnalelor folosind sigaction
void handle_signal(int sig) {
    if (sig == SIGUSR1) {
        //user defined 1
        //programul raspunde la SIGUSR1 scriind un mesaj
        const char *msg = "Un nou raport a fost adaugat in sistem!\n";
        write(STDOUT_FILENO, msg, strlen(msg));
        //trimit acest mesaj direct pe ecranul terminalului
    }
    else if (sig == SIGINT) {
        //programul se incheie la SIGINT, iar scrie un mesaj pe ecranul terminalului
        //desigur, folosind write
        const char *msg = "\nSIGINT. Se inchide...\n";
        write(STDOUT_FILENO, msg, strlen(msg));

        //modific variabila ca sa opresc bucla while din main
        keep_running = 0;
    }
}

int main(){
    //cele 4 linii de sigaction facute cu AI
    struct sigaction sa;//declar functia
    sa.sa_handler = handle_signal;//setez functia de tratare
    sigemptyset(&sa.sa_mask);//curat masca de semnale
    sa.sa_flags = 0;//setez flagurile pe 0

    //aplic formularul pt cele 2 semnale cerute
    if (sigaction (SIGUSR1, &sa, NULL) == -1) {
        printf("eroare la sigaction USR1\n");
        exit(-1);
    }
    if (sigaction (SIGINT, &sa, NULL) == -1) {
        printf("eroare la sigaction INT\n");
        exit(-1);
    }

    //creearea fisierului .monitor_pid
    const char *pid_file = ".monitor_pid";
    int fd = open(pid_file, O_CREAT | O_TRUNC | O_WRONLY, 0644);
    if (fd == -1) {
        printf("errore la open %s\n", pid_file);
        exit(-1);
    }

    //scrierea PID-ului
    pid_t pid = getpid();
    char pid_str[32];
    sprintf(pid_str, "%d\n", pid);
    write(fd, pid_str, strlen(pid_str));
    close(fd);

    printf("Monitor pid: %d\nVedem semnalele\n", pid);

    //bucla de asteptare
    while (keep_running == 1) {
        pause();//suspenda executia lui pana la primirea unui semnal
    }

    //stergerea fisierului pana la final
    unlink(pid_file);
    printf("Monitor inchis");

    return 0;
}
