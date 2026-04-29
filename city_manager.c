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

    //symlink
    char link_name[256];
    sprintf(link_name, "active_reports-%s", district_name);

    //symlink(target, linkpath)
    //daca link ul exista deja, symlink va returna -1 (cu eroarea EEXIST), dar nu-i problema, doar il ignoram
    if (symlink(path, link_name) != 0) {
        if (errno != EEXIST) {
            printf("Nu s-a putut crea link simbolic %s\n",link_name);
        }
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

void log_action(const char *district_name, const char *role, const char *user, const char *action) {
    char path[256];
    sprintf(path, "%s/logged_district", district_name);

    struct stat st = {0};
    if (stat(path, &st) != -1) {
        //dacaa userul e inspector si bitul de scriere pe grup (S_IWGRP) lipseste (fisierul e 644)
        if (strcmp(role, "inspector") == 0 && !(st.st_mode & S_IWGRP)) {
            //nu printez nimic ca sa nu stric flow ul, dar dau return ca sa NU scrie in fisier
            return;
        }
    }
    int fd = open(path, O_WRONLY | O_APPEND);
    if (fd != -1) {
        time_t now = time(NULL);
        char *timestamp = ctime(&now);
        timestamp[strcspn(timestamp, "\n")] = '\0';//scot \n de la ctime

        //un buffer ca sa construiesc toata propozitia
        char log_buffer[512];
        sprintf(log_buffer, "[%s] User: %s (%s) - Action: %s\n", timestamp, user, role, action);

        //folosesc write, numar cate caractere are propozitia
        write(fd, log_buffer, strlen(log_buffer));
        close(fd);
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

        chmod(path, 0664);//setez permisiunea fisierului pe 664
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
        printf("eroare la deschiderea reports.dat pentru stergerea ID-ului!!\n");
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

        //scad id-ul ca sa umplu gaura lasata de raportul sters
        r.id = r.id - 1;

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

void update_treshold(char *district_name, int value) {
    char path[256];
    sprintf(path, "%s/district.cfg", district_name);

    //apelez stat ca sa extrag informatiile fisierului inainte de a scrie
    struct stat st = {0};
    if (stat(path, &st) == -1) {
        printf("eroare stat update_treshold!!\n");
        exit(-1);
    }

    //st_mode & 0777 extrage nr octal al permisiunilor curente
    if ((st.st_mode & 0777) != 0640) {
        printf("Eroare de securitate\n");
        printf("Permisiunile fisierului %s au fost compromise\n",path);
        printf("M-am asteptat la 640 | Am gasit %o\n",st.st_mode & 0777);
        printf("S-a refuzat operatiunea update_treshold\n");
        exit(-1);
    }

    int fd = open(path, O_WRONLY | O_TRUNC);
    if (fd == -1) {
        printf("eroare open update_treshold\n");
        exit(-1);
    }

    if (write(fd, &value, sizeof(value)) != sizeof(value)) {
        printf("eroare write update_treshold\n");
        exit(-1);
    }

    fchmod(fd, 0640);
    close(fd);
    printf("pragul pentru districtul %s a fost actulizat cu succes la valoarea: %d\n",district_name,value);
}

int parse_condition(const char *input, char *field, char *op, char *value) {
    //caut primul caracter ':' in sirul de caractere 'input'
    //strchr returneaza un pointer catre locul unde a gasit ':'
    const char *colon1 = strchr(input, ':');

    //daca nu exista niciun ':', conditia e gresita si ne oprim
    if (colon1 == NULL) {
        printf("introdus date gresit, precum nu s-a pus primul ':' la filter (parse_condition)\n");
        return 0;
    }

    //caut al 2-lea ':' incepand de la pozitia de dupa primul ':' (colon1 + 1)
    const char *colon2 = strchr(colon1 + 1, ':');
    if (colon2 == NULL) {
        printf("nu s-a gasit al doilea ':' la filter!\n");
        return 0;
    }

    //am sa extrag campul field (tot ce este inainte de primul ':')
    //adresa lui colon1 - input va da lungimea primului cuvant
    int field_len = colon1 - input;
    strncpy(field, input, field_len);
    //copiez primele 'field_len' litere in variabila 'field'
    field[field_len] = '\0';//pun la final '\0' pentru a-l face un string valid in C

    //extrag operatorul op , intre primul si al 2-ela ':'
    int op_len = colon2 - (colon1 + 1);
    strncpy(op, colon1 + 1, op_len);//copiez literele operatorului
    op[op_len] = '\0';//inchid stringul

    //extrag value, de la al 2-ela ':' pana la final
    strcpy(value, colon2 + 1);

    //am reusit sa sparg in 3 bucati, return 1
    return 1;
}

int match_condition(report *r, const char *field, const char *op, const char *value) {
    //verific campul severity (tip numeric: int)
    if (strcmp(field,"severity")==0) {
        //valoarea a fost citita ca text, trebuie s-o transform in nr intreg
        int num_value = atoi(value);

        //verific daca operatorul este cerut si fac comparatia matematica
        if (strcmp(op, "==")==0)return r->severity == num_value;
        if (strcmp(op, "!=")==0)return r->severity != num_value;
        if (strcmp(op, ">")==0)return r->severity > num_value;
        if (strcmp(op, "<")==0)return r->severity < num_value;
        if (strcmp(op, "<=")==0)return r->severity <= num_value;
        if (strcmp(op, ">=")==0)return r->severity >= num_value;
    }
    //verific campul category (tip text: string)
    else if (strcmp(field,"category")==0) {
        //daca strcmp returneaza 0, inseamna ca textele sunt identice
        if (strcmp(op, "==") == 0)return strcmp(r->issue_category, value) == 0;
        if (strcmp(op, "!=") == 0)return strcmp(r->issue_category, value) != 0;
        //operatiile <,> etc nu au sens logic pentru categoriile in acest proiect, asa ca nu le prindem
    }
    //verific campul insepctor (tip text: string)
    else if (strcmp(field,"inspector")==0) {
        if (strcmp(op, "==") == 0)return strcmp(r->inspector_name, value) == 0;
        if (strcmp(op, "!=") == 0)return strcmp(r->inspector_name, value) != 0;
    }
    //verific campul timestamp (tip numeric: time_t)
    else if (strcmp(field,"timestamp")==0) {
        //transform textul in nr de tip long si il convertesc fortat in time_t
        time_t time_value = (time_t)atol(value);

        if (strcmp(op, "==") == 0)return r->timestamp == time_value;
        if (strcmp(op, "!=") == 0)return r->timestamp != time_value;
        if (strcmp(op, ">") == 0)return r->timestamp > time_value;
        if (strcmp(op, "<") == 0)return r->timestamp < time_value;
        if (strcmp(op, "<=") == 0)return r->timestamp <= time_value;
        if (strcmp(op, ">=") == 0)return r->timestamp >= time_value;
    }
    //daca am ajuns aici inseamna ca utilizatorul a introdus un camp necunoscut
    //sau un operator invalid pentru acel camp. returnez 0
    return 0;
}

void filter(char *disctrict_name, int condition_count, char *conditions[]) {
    char path[256];
    sprintf(path, "%s/reports.dat", disctrict_name);

    //deschid fisierul doar pentru citire
    int fd = open(path, O_RDONLY);
    if (fd == -1) {
        printf("eroare la deschiderea fisierului pentru filter!!!\n");
        exit(-1);
    }

    report r = {0};
    int found_any = 0; //sa vad daca am printat macar un raport
    while (read(fd, &r, sizeof(report)) == sizeof(report)) {
        int passes_all = 1; //presupun initial ca raportul trece testul

        //trec raportul prin toate conditiile posibile
        for (int i = 0; i < condition_count; i++) {
            char field[32] = {0}, op[4] = {0}, value[128] = {0};
            //daca parse_condition returneaza 1, a reusit sa sparga textul in 3 bucati
            if (parse_condition(conditions[i], field, op, value)) {
                //acum verific daca raportul curent se potriveste cu bucatile
                if (match_condition(&r, field, op, value)==0) {
                    passes_all = 0;//a picat conditia
                    break;//opresc verificarea
                }
            }
            else {
                printf("eroare, conditia %s este invalida!!!\n",conditions[i]);
                close(fd);
                exit(-1);
            }
        }
        //dupa ce am verificat toate conditiile, 'passes all' a ramas 1, voi printa
        if (passes_all==1) {
            found_any = 1;

            printf("--- Raport ID: %d ---\n", r.id);
            printf("Inspector : %s\n", r.inspector_name);
            printf("Categorie : %s (Severitate: %d)\n", r.issue_category, r.severity);
            printf("GPS       : [%.4f, %.4f]\n", r.latitude, r.longitude);

            char time_str[64];
            strcpy(time_str, ctime(&r.timestamp));
            time_str[strcspn(time_str, "\n")] = '\0';

            printf("Data      : %s\n", time_str);
            printf("Descriere : %s\n", r.description);
            printf("---------------------\n");
        }
    }

    //daca am ajuns la final si nu a trecut nimic de filtre
    if (found_any==0) {
        printf("niciun raport nu corespunde filtrelor in disctrictul %s.\n",disctrict_name);
    }
    close(fd);
}

void remove_district(char *district_name){
    char path[256];
    sprintf(path, "%s/reports.dat", disctrict_name);

    int fd = open(path, O_RDWR);
    if(fd == -1) {
        printf("eroare la deschiderea reports.dat pentru stergerea districtului !!\n");
        exit(-1);
    }
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
        log_action(target_district, current_role, current_user, "add");
    }
    else if (strcmp(command, "--list") == 0) {
        printf("Comanda list.\n");
        list(target_district);
        log_action(target_district, current_role, current_user, "list");
    }
    else if (strcmp(command, "--view") == 0) {
        if (argc < 8) {
            printf("eroare! lipseste report_id pentru view !\n");
            exit(-1);
        }
        int report_id = atoi(argv[7]);//convertim din text in numar
        printf("Comanda view cu ID-ul %d.\n",report_id);
        view(target_district, report_id);
        log_action(target_district, current_role, current_user, "view");
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
        log_action(target_district, current_role, current_user, "remove_report");
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
        update_treshold(target_district, value);
        log_action(target_district, current_role, current_user, "update_threshold");
    }
    else if (strcmp(command, "--filter") == 0) {
        if (argc < 8) {
            printf("eroare! lipseste condition pentru filter !\n");
            exit(-1);
        }
        printf("Comanda filter.\n");
        int condition_count = argc - 7;
        filter(target_district, condition_count, &argv[7]);
        log_action(target_district, current_role, current_user, "filter");
    }
    else if (strcmp(command, "--remove_district") == 0) {
        if(strcmp(current_role,"manager")!=0) {
            printf("eroare de permisiune! numai managerii pot sterge districturi !\n");
            exit(-1);
        }
        printf("Comanda remove_district.\n");
        remove_district(target_district);
        log_action(target_district, current_role, current_user, "remove_district");
    }
    else {
        printf("Comanda necunoscuta (%s) !\n",command);
        exit(-1);
    }
    return 0;
}
