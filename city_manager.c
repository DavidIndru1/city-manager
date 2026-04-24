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
    int id;//4 bytes
    char inspector_name[32];//32 bytes
    float latitude;//4 bytes
    float longitude;//4 bytes
    char issue_category[16];//16 bytes
    int severity;//4 bytes
    time_t timestamp;//8 bytes
    char description[136];//136 bytes
}report;//total 208 bytes

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
    int fd;
    fd = open(path, O_CREAT | O_APPEND | O_RDWR, 0664);//creez reports.dat
    if (fd != -1) {
        fchmod(fd, 0664);
        close(fd);
    }
    else {
        printf("Eroare la crearea reports.dat\n");
        exit(-1);
    }

    sprintf(path, "%s/district.cfg", district_name);
    fd = open(path, O_CREAT | O_RDWR, 0640);//creez district.cfg
    if (fd != -1) {
        fchmod(fd, 0640);
        close(fd);
    }
    else {
        printf("Eroare la crearea district.cfg\n");
        exit(-1);
    }

    sprintf(path, "%s/logged_district", district_name);
    fd = open(path, O_CREAT | O_RDWR | O_APPEND, 0644);//creez logged_district
    if (fd != -1) {
        fchmod(fd, 0644);
        close(fd);
    }
    else {
        printf("Eroare la crearea logged_district\n");
        exit(-1);
    }
}

void add(char *district_name, char *user_name) {
    report r = {0}; //initializez toata structura cu 0

    strncpy(r.inspector_name, user_name, sizeof(r.inspector_name)-1);//copiez numele
    //pt siguranta ca asa am loc si de \0, folosesc strncpy
    r.timestamp = time(NULL);//citim timpul actual => o sa fie un nr dubios

    char path[256];
    sprintf(path, "%s/reports.dat", district_name);

    struct stat st = {0};//statisticile fisierul
    if (stat(path, &st) == 0) {//citim detaliile fisierului.
        r.id = (st.st_size / sizeof(report)) + 1;//daca fisierul are 208 bytes, inseamna ca are un singur raport in el
        //+1 si obtinem ID-ul 2 pentru noul nostru raport
    }
    else {
        r.id = 1;
        //daca stat da eroare, fisierul nu are nimic in el, fortez primul ID sa fie 1.
    }

    printf("X: ");
    scanf("%f", &r.latitude);

    printf("Y: ");
    scanf("%f", &r.longitude);

    printf("Category (road/lighting/flooding/other): ");
    scanf("%15s", r.issue_category);//citesc maxim 15 caractere

    printf("Severity level (1/2/3): ");
    scanf("%d", &r.severity);

    int c;
    while ((c=getchar()) != '\n' && c != EOF);//ca sa manance \n de la scanf de la severity

    printf("Description: ");
    fgets(r.description, sizeof(r.description), stdin);//fgets ca sa citesc cu spatii
    r.description[strcspn(r.description,"\n")] = '\0';// Taiem \n de la finalul lui fgets

    int fd = open(path, O_WRONLY | O_APPEND);//deschid fisierul
    if (fd != -1) {
        write(fd, &r, sizeof(report));
        close(fd);

        chmod(path, 0644);//setez permisiunea fisierului pe 664
        printf("Raportul cu ID %d a fost adaugat cu succes in districtul %s!\n", r.id, district_name);
    }
    else {
        printf("Eroare la deschiderea reports.dat pentru scriere (functia add)!\n");
        exit(-1);
    }
}

void print_permissions(mode_t mode) {
    char perms[10] = "---------"; // incep cu totul gol

    if (mode & S_IRUSR) perms[0] = 'r'; // manager: citire (sau owner)
    if (mode & S_IWUSR) perms[1] = 'w'; // manager: scriere
    if (mode & S_IXUSR) perms[2] = 'x'; // manager: executie

    if (mode & S_IRGRP) perms[3] = 'r'; // inspector: citire (sau grup)
    if (mode & S_IWGRP) perms[4] = 'w'; // inspector: scriere
    if (mode & S_IXGRP) perms[5] = 'x'; // inspector: executie

    if (mode & S_IROTH) perms[6] = 'r'; // altii: citire (others)
    if (mode & S_IWOTH) perms[7] = 'w'; // altii: scriere
    if (mode & S_IXOTH) perms[8] = 'x'; // altii: executie

    printf("%s\n", perms);
}

void list(char *district_name) {
    char path[256];
    sprintf(path, "%s/reports.dat", district_name);

    struct stat st = {0};
    if (stat(path, &st) == -1) {
        printf("eroare functia list, nu s-a putut citi informatii despre %s\n",path);
        exit(-1);
    }

    printf("=== Informatii Fisier ===\n");
    printf("Fisier: %s\n", path);
    printf("Dimensiune: %ld bytes\n", st.st_size);
    printf("Permisiuni : ");
    print_permissions(st.st_mode);

    // ctime returneaza data cu un '\n' la final
    printf("\nUltima mod : %s", ctime(&st.st_mtime));
    printf("=========================\n\n");

    int fd = open(path, O_RDONLY);
    if (fd == -1) {
        printf("Eroare la deschiderea reports.dat pentru citire!\n");
        exit(-1);
    }

    report r = {0};
    int count = 0; // Contor pentru a vedea daca fisierul e gol

    while (read(fd, &r, sizeof(report)) == sizeof(report)) {
        count++;
        printf("--- Raport ID: %d ---\n", r.id);
        printf("User name : %s\n", r.inspector_name);
        printf("Categorie : %s (Severitate: %d)\n", r.issue_category, r.severity);
        printf("GPS       : [%.4f, %.4f]\n", r.latitude, r.longitude);

        char time_str[64];
        strcpy(time_str, ctime(&r.timestamp));
        time_str[strcspn(time_str, "\n")] = '\0';

        printf("Data      : %s\n", time_str);
        printf("Descriere : %s\n", r.description);
        printf("\n");
    }

    if (count == 0) {
        printf("nu exista rapoarte in districtul %s.\n",district_name);
    }
    close(fd);
}

void view(char *district_name, int report_id) {
    char path[256];
    sprintf(path, "%s/reports.dat", district_name);

    int fd = open(path, O_RDONLY);
    if (fd == -1) {
        printf("eroare la view! nu s-a putut deschide fisierul!!!\n");
        exit(-1);
    }

    report r = {0};
    int found = 0;//o variabila pentru cand am gasit id-ul

    //citesc structura cu structura (208 bytes)
    while (read(fd, &r, sizeof(report)) == sizeof(report)) {
        if (r.id == report_id) {
            found = 1;//l-am gasit!

            printf("=== Detalii Raport ID: %d ===\n",r.id);
            printf("Inspector : %s\n",r.inspector_name);
            printf("Categorie : %s (Severitate: %d)\n", r.issue_category,r.severity);
            printf("GPS       : [%.4f, %.4f]\n", r.latitude, r.longitude);

            //am sa tai '\n' adaugat de ctime
            char time_str[64];
            strcpy(time_str, ctime(&r.timestamp));
            time_str[strcspn(time_str, "\n")] = '\0';

            printf("Data      : %s\n", time_str);
            printf("Descriere : %s\n", r.description);
            printf("=============================\n");

            break;//opresc bucla fortat
        }
    }
    if (found == 0) {
        printf("eroare! raportul cu id-ul %d nu exista in districtul %s.\n",report_id,district_name);
    }
    close(fd);
}

void remove_report(char *district_name, int report_id) {
    char path[256];
    sprintf(path, "%s/reports.dat", district_name);

    int fd = open(path, O_RDWR);
    if (fd == -1) {
        printf("eroare la deschiderea reports.dat pentru stergere!!\n");
        exit(-1);
    }

    struct stat st = {0};
    fstat(fd, &st);
    off_t file_size = st.st_size; //salvez marimea in bytes

    report r = {0};
    int found = 0;
    off_t target_offset = 0;//aici salvez pozitia unde e raportul gasit

    while (read(fd, &r, sizeof(report)) == sizeof(report)) {
        if (r.id == report_id) {
            found = 1;
            //daca l-am gasit, salvez pozitia unde a inceput in fisier
            //lseek cu SEEK_CUR ne da pozitia curenta
            //scad marimea unui raport ca sa aflu pozitia de inceput a lui
            target_offset = lseek(fd, 0, SEEK_CUR) - sizeof(report);
            break;
        }
    }
    if (found == 0) {
        printf("eroare, nu am gasit raportul cu ID-ul %d in districtul %s.\n",report_id,district_name);
        close(fd);
        return;
    }
    //o sa am 2 cursoare, read_offset de unde citesc, write_offset de unde
    off_t write_offset = target_offset;//incep sa suprascriu de la raportul sters
    off_t read_offset = target_offset + sizeof(report);//incep sa citesc de la urmatorul raport

    //cat timp mai am rapoarte citesc pana la finalul fisierului
    while (read_offset < file_size) {
        //mut cursorul la raportul urmator si il citesc in memorie
        lseek(fd, read_offset, SEEK_SET);
        read(fd, &r, sizeof(report));

        //mut cursorul inapoi cu o pozitie si il scriu peste cel vechi
        lseek(fd, write_offset, SEEK_SET);
        write(fd, &r, sizeof(report));

        //avansez ambele cursoare pentru urmatorul pas al buclei
        read_offset += sizeof(report);
        write_offset += sizeof(report);
    }
    ftruncate(fd, file_size - sizeof(report));
    //fisierul are un raport duplicat la final, il tai stergand fix 208 bytes din marimea totala
    printf("Fisierul cu ID-ul %d a fost sters.\n",report_id);
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
        add(target_district, current_user);
    }
    else if (strcmp(command, "--list") == 0) {
        printf("Comanda list.\n");
        list(target_district);
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
        remove_report(target_district, report_id);
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
        //functia update treshold
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
            //functia filter
            }
        }
    else {
        printf("Comanda nacunoscuta (%s) !\n",command);
        exit(-1);
    }
}
