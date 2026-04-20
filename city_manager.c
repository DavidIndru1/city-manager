#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>
#include <errno.h>

typedef struct report
{
    int id;
    char inspector_name[32];
    float latitude;
    float longitude;
    char issue_category[16];
    int severity;
    time_t timestamp;
    char description[136];
}report;

void init_district_files(const char *district_name) {
    printf("%s\n", district_name);
    char path[256];

    //creez directory, verific daca a fost creat cu succes
    if (mkdir(district_name, 0750) != 0) {
        if (errno != EEXIST) { //ma opresc numai daca eroarea nu este EEXIST (manpage mkdir)
            printf("eroare mkdir\n");
            exit(-1);
        }
    }
    chmod(district_name, 0750);//ca sa fortez permisiunile directorului

    sprintf(path, "%s/reports.dat", district_name);
    int fd = 0;
    fd = open(path, O_CREAT | O_APPEND | O_RDWR, 0664);//creez reports.dat
    close(fd);

    sprintf(path, "%s/district.cfg", district_name);
    fd = open(path, O_CREAT | O_RDWR, 0640);//creez district.cfg
    close(fd);

    sprintf(path, "%s/logged_district", district_name);
    fd = open(path, O_CREAT, 0644);//creez logged_district
    close(fd);
}

int main(int argc, char *argv[])
{
    if(argc < 7)
    {
        printf("prea putine argumente\n");
        exit(-1);
    }
    if (strcmp(argv[1], "--role") != 0) {
        printf("argumentul de pe pozitia 1 nu este --role !!!\n");
        exit(-1);
    }
    char *current_role = argv[2];
    if (strcmp(current_role, "manager") != 0 && strcmp(current_role, "inspector") != 0) {
        printf("argumentul de pozitia 2 nu este manager sau inspector !!!\n");
        exit(-1);
    }
    if (strcmp(argv[3], "--user") != 0) {
        printf("argumentul de pe pozitia 3 nu este --user !!!\n");
        exit(-1);
    }
    char *current_user = argv[4];//retine numele user-ului
    char *command = argv[5]; // va fi --add, --list etc
    char *target_district = argv[6]; //va fi de ex: "downtown"

    init_district_files(target_district);

    printf("Rol: %s | User: %s\n", current_role, current_user);
    printf("Command: %s | Target: %s\n", command, target_district);

    if (strcmp(command, "--add") == 0) {
        printf("Comanda add.\n");
        //functia add
    }
    else if (strcmp(command, "--list") == 0) {
        printf("Comanda list.\n");
        //functia list
    }
    else if (strcmp(command, "--view") == 0) {
        if (argc < 8) {
            printf("eroare! lipseste report_id pentru view !\n");
            exit(-1);
        }
        int report_id = atoi(argv[7]);//convertim din text in numar
        printf("Comanda view cu ID-ul %d.\n",report_id);
        //functia view
    }
    else if (strcmp(command, "--remove_report") == 0) {
        if (argc < 8) {
            printf("eroare! lipseste report_id pentru stergere !\n");
            exit(-1);
        }
        if (strcmp(current_role,"manager")!=0) {
            printf("eroare de permisiune! numai managerii pot sterge !\n");
            exit(-1);
        }
        int report_id = atoi(argv[7]);//convertim din text in numar
        printf("Comanda remove_report cu ID-ul %d.\n",report_id);
        //functia remove_report
    }
    else if (strcmp(command, "--update_threshold") == 0) {
        if (argc < 8) {
            printf("eroare! lipseste value pentru update treshold !\n");
            exit(-1);
        }
        if (strcmp(current_role,"manager")!=0) {
            printf("eroare de permisiune! numai managerii pot actualiza !\n");
            exit(-1);
        }
        int value = atoi(argv[7]);
        printf("Actualizez treshold la valoarea %d.\n",value);
    }
    else if (strcmp(command, "--filter") == 0) {
        if (argc < 8) {
            printf("eroare! lipseste condition pentru filter !\n");
            exit(-1);
        }
        char *condition = argv[7];
        printf("Comanda filter.\n");
        for (int i = 7 ; i < argc ; i++) {
            char *current_condition_filter = argv[i];
            char field[32], op[4], value[32];

            // Daca functia generata de AI returneaza 1, totul e ok
            if (parse_condition(current_condition_filter, field, op, value)) {
                printf("Conditie gasita - Camp: %s | Operator: %s | Valoare: %s\n", field, op, value);
            }
            // Daca returneaza 0, utilizatorul a tastat ceva gresit -> OPRIM TOT
            else {
                printf("Eroare: Sintaxa invalida pentru conditia '%s'!\n", current_condition_filter);
                printf("Formatul corect este camp:operator:valoare (ex: severity:>=:2)\n");
                exit(-1);
            }
        }
    }
    else {
        printf("Comanda nacunoscuta (%s) !\n",command);
        exit(-1);
    }
}
