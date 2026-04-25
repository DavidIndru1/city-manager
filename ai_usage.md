ai_usage.md va fi fisierul care va descrie AI-ul folosit, prompturile date, ce-a generat, ce am schimbat si de ce, si ce am invatat.

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