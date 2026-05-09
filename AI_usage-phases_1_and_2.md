Phase 1
 -

1. Tool-ul folosit: Gemini AI (Pro)
    -
2. Prompturile pe care le-am dat:
    -
- Am creat un nou chat cu Gemini iar primul lucru a fost sa-i trimit tot proiectul (Intreg documentul Phase 1). Am copiat tot codul meu, i-am trimis la AI, si i-am cerut sa mi faca functiile int parse_condition si int match_condition.
- I-am trimis codul meu ca sa poata sa-mi faca functiile astfel incat codul pe care il va genera sa nu aibe o structura complet diferita cu structura pe care o aveam pana in acel punct (de ex: la fiecare functie incepeam cu un char path[], sprintf(), un int fd = open(), creeandu-mi o structura pe care o foloseam pe intreg programul. Adica incepeam sa codez o noua functie si din prima cam stiam minimul pe care trebuie sa-l fac)

3. Ce-a fost generat:
    -
- Gemini-ul mi-a generat codul (functiile int parse_condition si int match_condition), cu tot cu explicatii, am verificat codul, astfel incat sa-l inteleg si eu, am mai pus cateva explicatii in plus (explicatiile pe care le-am pus, si nu doar pentru aceste 2 functii generate de AI, ci pentru toate functiile din codul meu, m-au ajutat si pe mine sa inteleg codul mai bine)
- Gemini-ul mi-a dat logica pentru parse_condition folosind strchr pentru a verifica pana la ':', strncpy pentru a copia caracterele in ordine (pentru primukl cuvant pana la primul ':', pentru al 2-lea cuvant de la primul ':' pana la al 2-lea ':', pentru al 3-lea cuvant de la al 2-lea ':' pana la final)
- Gemini-ul mi-a generat functia match_condition cu mai multe if else-uri, folosind strcmp pentru texte si atoi si atol pentru convertirea string-ului in intreg sau long, pentru comparatii matematice (==,<,>,!= etc.)

4. Ce am schimbat si de ce:
    -
- Marimea buffere-lor. Pentru campurile field, op si value, am schimbat marimea stringurilor ca sa fie egale cu implementatia mea, pentru ca sa nu se creeze buffer overflows

5. Ce am invatat:
    -
- Am invatat cum ca daca fac diferenta dintre 2 pointere in C (colon1 - input) imi da lungimea string-ului intre aceste 2 adrese de memorie. M-am folosit de asta la strncpy
- Am invatat sa folosesc strchr pentru a gasi prima aparenta a unui caracter in string
- Initial nu prea am inteles de ce a trebuit sa fac 2 conditii diferite (ma gandeam de ce nu fac o conditie pur si simplu), dar presupun ca datorita structurii ordonate in program, a fost mult mai bine impartirea pe 2 conditii

Phase 2
 -

1. Tool-ul folosit: Gemini AI (Pro)
   -
2. Prompturile pe care le-am dat:
   -
- Am continuat pe acelasi chat cu Gemini, i-am retrimis proiectul (care acum a inclus phase 2)
- I-am cerut sa-mi explice toate functiile care au fost nevoie pentru pahse 2 (wait, fork, kill etc)
- I-am cerut sugestii (nu cod) pentru cum as putea aborda monitor_reports, mi-a sugerat sa fac o functie numta handle_signal care trateaza semnalele SIGUSR1 si SIGINT
- I-am cerut ajutor la main, initial facea cu volatile sig_atomic_t si nu intelegeam. I-am zis sa refaca si mi-a facut primele 4 linii de cod din main, pe care dupa mai multe eplicatii am inteles rolul lor si ce faceau ele.
- Dupa mi-a zis sa tratez cazurile de eroare, sa creez fisierul .monitor_pid sa scriu PID-ul in in fisier, sa fac o bucla de asteptare (acestea in afara de primele 4 linii de cod au fost sugestii, nu cod)
- La functia remove_district din city_manager nu am stiut sa folosesc exclp, aia a fost generata de Gemini, dar am inteles ce a generat.
- Mi-a sugerat ca in main, initializarea fisierelor se modifica si ca trebuie sa initializez doar daca nu fac remove_district.
- Nu am stiut cum sa ma folosesc de monitor_reports in city_manager, asa ca Gemini a sugerat ca la functia --add am sa ma folosesc de monitor_reports.

3. Ce-a fost generat:
    -
- AI-ul mi-a generat explicatii detaliate pentru apelurile de sistem necesare in phase 2 (wait, fork, kill etc), ajutzndu-mz sz inteleg cum interactioneaza procesele intre ele.
- Sugestia de a crea o functie separata handle_signal care sa gestioneze exclusiv reactia programului la semnalele SIGUSR1 si SIGINT.
- Dupa ce am cerut refacerea codului prea complex, Gemini mi-a generat exact cele 4 linii de cod necesare in main pentru setarea structurii sigaction, insotite de explicatii pentru fiecare linie in parte.
- Mi-au fost generate ideile logice (fara cod direct) despre cum trebuie sa arate restul functiei main din monitor: tratarea erorilor, crearea fisierului .monitor_pid, scrierea PID-ului curent in el si bucla infinita de asteptare.
- Linia specifica de cod execlp("rm", "rm", "-rf", district_name, NULL); din interiorul functiei remove_district, pe care nu stiam cum sa o formulez corect, alaturi de logica pentru fork().
- Sugestia de a adauga o conditie if in jurul lui init_district_files in main, pentru a evita recrearea fisierelor fix inainte de comanda de stergere a districtului.
- Sugestia si logica de integrare a celor doua programe: sectiunea de cod din ramura --add unde se deschide fisierul .monitor_pid, se citeste PID-ul si se trimite semnalul folosind kill(monitor_pid, SIGUSR1).

4. Ce am schimbat si de ce:
   - 
- AI-ul mi-a propus initial sa folosesc tipul volatile sig_atomic_t pentru variabila globala care tine monitorul pornit. Deoarece era un concept prea avansat pe care nu l-am invatat, i-am cerut sa refaca si am schimbat-o intr-un simplu int keep_running = 1;, care este mult mai clar pentru mine.

5. Ce am invatat:
   -
- Am invatat ca mai sigur decat printf() ar fi write(STDOUT_FILENO,...) pentru semnale.
- Am inteles ca cu sigaction inregistrez functia dar nu o apelez direct. Folosesc pause() ca sa intrerupa programul si sa execute handler-ul doar cand intercepteaza semnalul asteptat.
- Am inteles ca kill() este functia principala de comunicare inter-proces, prin care pot trimite semnale personalizate (precum SIGUSR1 pentru notificari)./
- Am inteles fork() si cum ca poate fi inlocuita de execlp(),si cat de important este ca procesul parinte sa astepte cu wait() pentru ca fiul sa nu devina un proces "zombie" (sa ramana agatat in tabela de procese).