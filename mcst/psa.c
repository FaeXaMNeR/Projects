#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#define PSA_LEN 128
#define MAX_ODN 32

// Структура для одночлена
typedef struct {
    char var[MAX_ODN];    // строка с переменной
    int koef;             // коэффициент
    int pow;              // степень
    int size;             // размер массива
} Monochlen;


// Сравнение для сортировки
int compareStrings(const void *a, const void *b) {
    return strcmp(*(const char **)a, *(const char **)b);
}

// Парсер для выделения одночленов
Monochlen *parse(char *form, char **monoms) {
    int signs[MAX_ODN] = {0};
    for(int i = 0; i < MAX_ODN; i++) signs[i] = 1;

    // Цикл по строке, поиск знаков
    int minus_flag = 0;   // флаг для минуса
    int count = 0;
    int form1_count = 0; // кол-во слагаемых в 1 форме
    
    if(form[0] == '-') {
        signs[0] *= -1;
    }
    count++;

    for(int i = 1; i < strlen(form); i++) {
        if(form[i] == '+') {
            count++;
        }
        else if(form[i] == '-') {
            signs[count++] = -1;
        }
        else if(form[i] == '^') {
            minus_flag = 1;
            form1_count = count;
        }
    }

	char *monom_token = strtok(form, " ^$+-\n");   // ^ для вычитания, $ для сложения

    // Парсинг многочлена на одночлены
    int n = 0;
	while (monom_token != NULL) {
		monoms[n++] = monom_token;
		monom_token = strtok(NULL, " ^$+-\n");
	}

    // Заполнение одночленами, n - количество
	monoms[n] = NULL;

    
    Monochlen *monochlen = (Monochlen *)malloc(n*sizeof(Monochlen));
    if (monochlen == NULL) {
        return NULL;
    }


    char *factors[MAX_ODN];
    for(int i = 0; i < n; i++) {
        monochlen[i].pow = 0;
        char *token = strtok(monoms[i], "*");
        while (token != NULL) {
            factors[monochlen[i].pow] = token;
            token = strtok(NULL, "*");
            monochlen[i].pow++;
        }

        // Заполнение структуры:

        // Одночлен с коэффициентом 1 и степенью 1 (х)
        if (monochlen[i].pow == 1) {
            monochlen[i].koef = 1;
            strcpy(monochlen[i].var, factors[0]);
        }

        // Одночлен вида 5*х, x*x
        else if (monochlen[i].pow == 2) {
            // x*x
            if(atoi(factors[0]) == 0) {
                monochlen[i].koef = 1;
                qsort(factors, monochlen[i].pow, sizeof(char*), compareStrings);
                monochlen[i].var[0] = '\0';
                strcat(monochlen[i].var, factors[0]);
                strcat(monochlen[i].var, factors[1]);
            }
            // 5*y
            else {
                monochlen[i].koef = atoi(factors[0]);
                monochlen[i].pow--;
                strcpy(monochlen[i].var, factors[1]);
            }
        }

        // Одночлен вида 5*х*y, x*y*z
        else if(monochlen[i].pow > 2) {
            if(atoi(factors[0]) == 0) {
                monochlen[i].koef = 1;
                qsort(factors, monochlen[i].pow, sizeof(char*), compareStrings);

                monochlen[i].var[0] = '\0';
                for(int j = 0; j < monochlen[i].pow; j++) {
                    strcat(monochlen[i].var, factors[j]);
                }
            }
            else {
                monochlen[i].koef = atoi(factors[0]);
                qsort(factors, monochlen[i].pow, sizeof(char*), compareStrings);

                monochlen[i].var[0] = '\0';
                for(int j = 1; j < monochlen[i].pow; j++) {
                    strcat(monochlen[i].var, factors[j]);
                }
            }

            if(atoi(factors[i]) != 0) {
                monochlen[i].pow--;
            }
        }

        
        monochlen[i].koef *= signs[i];
        monochlen[i].size = n;

        //printf("koef = %d, var = %s\n", monochlen[i].koef, monochlen[i].var);
    }

    if(minus_flag) {
        for(int i = form1_count; i < n; i++) {
            monochlen[i].koef *= -1;
        }
    }

    return monochlen;
}

// Отдельная функция для вставки знака умножения
char* insert_stars(char* input) {
    int len = strlen(input);
    if (len == 0) {
        return NULL;
    }

    char* result = (char*)malloc(2 * len * sizeof(char));

    if (result == NULL) {
        return NULL;
    }

    int j = 0;
    for (int i = 0; i < len; i++) {
        result[j++] = input[i];
        if (i < len - 1) {
            result[j++] = '*';
        }
    }

    result[j] = '\0';
    return result;
}

// Склеивание одночленов и приведение к каноническому виду
void kanon(Monochlen *mono, char *form, int len) {
    form[0] = '\0';

    for(int i = 0; i < len; i++) {
        char koeff[8];
        koeff[0] = '\0';

        if(mono[i].koef == 0) {
            continue;
        }

        if (i != 0 && atoi(form) != 0) {
            if(mono[i].koef > 0) strcat(form, "+");
        }

        snprintf(koeff, sizeof(koeff), "%d", mono[i].koef);

        strcat(form, koeff);

        // Если при делении пропали переменные
        if(mono[i].var[0] != '\0') {
            strcat(form, "*");
            strcat(form, insert_stars(mono[i].var));
        }
    }
}

void plus(char *psa_form1, char *psa_form2) {
    strcat(psa_form1, "$");                 // символ окончания 1 формы для парсера
    strcat(psa_form1, psa_form2);

    char *form[PSA_LEN];
    
    Monochlen *monochlen = parse(psa_form1, form);
    int len = monochlen[0].size; // количество одночленов
    int new_len = len;           // количество приведенных слагаемых
    

    // Начало сложения
    for(int i = 0; i < len; i++) {
        for(int j = i+1; j < len; j++) {
            if(strcmp(monochlen[i].var, monochlen[j].var) == 0) {
                monochlen[i].koef += monochlen[j].koef;
                monochlen[j].koef = 0;
                new_len--;
            }
        }
    }


    char result_form[2*PSA_LEN];
    kanon(monochlen, result_form, len);
    printf("%s\n", result_form);
}

void minus(char *psa_form1, char *psa_form2, int equal_flag) {
    strcat(psa_form1, "^");            // сигнал для парсера
    strcat(psa_form1, psa_form2);

    char *form[PSA_LEN];
    
    Monochlen *monochlen = parse(psa_form1, form);
    int len = monochlen[0].size; // количество одночленов
    int new_len = len;           // количество приведенных слагаемых
    

    // Начало сложения
    for(int i = 0; i < len; i++) {
        for(int j = i+1; j < len; j++) {
            if(strcmp(monochlen[i].var, monochlen[j].var) == 0) {
                monochlen[i].koef += monochlen[j].koef;
                monochlen[j].koef = 0;
                new_len--;
            }
        }
    }


    char result_form[2*PSA_LEN];
    kanon(monochlen, result_form, len);
    if(equal_flag == 1 && atoi(result_form) == 0) printf("equal\n");
    else if(equal_flag == 1) printf("not equal");
    else if(equal_flag == 0) {
        if(atoi(result_form) == 0) printf("0\n");
        else printf("%s\n", result_form);
    }
}

void mult(char *psa_form1, char *psa_form2) {
    // Списки списков(одночленов)
    char *form1[PSA_LEN];
    char *form2[PSA_LEN];

    Monochlen *monochlen1 = parse(psa_form1, form1);
    Monochlen *monochlen2 = parse(psa_form2, form2);

    int size1 = monochlen1[0].size;
    int size2 = monochlen2[0].size;

    int size = size1 * size2;
    Monochlen res_mono[size];
    int current_index = 0;

    for(int i = 0; i < size1; i++) {
        for(int j = 0; j < size2; j++) {
            res_mono[current_index].koef = monochlen1[i].koef * monochlen2[j].koef;

            int length = strlen(monochlen1[i].var);
            char temp_var[length];
            strcpy(temp_var, monochlen1[i].var);
            strcat(temp_var, monochlen2[j].var);

            res_mono[current_index].var[0] = '\0';
            strcat(res_mono[current_index].var, temp_var);
            current_index++;
        }
    }

    //for(int i = 0; i < size; i++) printf("%d %s\n", res_mono[i].koef, res_mono[i].var);

    char result_form[2*PSA_LEN];
    kanon(res_mono, result_form, size);
    printf("%s\n", result_form);
}

void divide(char *psa_form1, char *psa_form2) {
    char *form1[PSA_LEN];
    char *form2[PSA_LEN];
    Monochlen *monochlen2 = parse(psa_form2, form2);

    if (monochlen2[0].size > 1)  {
        printf("Error\n");
        return;
    }

    Monochlen *monochlen1 = parse(psa_form1, form1);

    int size = monochlen1[0].size;
    int flag = 1; // флаг на возможность деления (1 = можно)
    int count[32] = {0};

    for(int i = 0; i < size; i++) {
        if(monochlen1[i].koef % monochlen2[0].koef != 0) {
            flag = 0;
            break;
        }

        // Проверка переменных
        for(int j = 0; monochlen1[i].var[j] != '\0'; j++) {
            count[monochlen1[i].var[j] - 'a']++;
        }

        for(int j = 0; monochlen2[0].var[j] != '\0'; j++) {
            if(count[monochlen2[0].var[j] - 'a'] == 0) {
                flag == 0;
                printf("Error\n");
                return;
            }
            count[monochlen2[0].var[j] - 'a']--;
        }
    }

    if(flag == 1) {
        for(int i = 0; i < size; i++) {
            monochlen1[i].koef /= monochlen2[0].koef;

            int current_index = 0;
            for(int j = 0; monochlen1[i].var[j] != '\0'; j++) {
                if(count[monochlen1[i].var[j] - 'a']) {
                    monochlen1[i].var[current_index++] = monochlen1[i].var[j];
                    count[monochlen2[i].var[j] - 'a']--;
                }
            }

            monochlen1[i].var[current_index] = '\0';
        }
    }

    else {
        printf("Error\n");
    }

    char result_form[2*PSA_LEN];
    kanon(monochlen1, result_form, size);
    printf("%s\n", result_form);
        
}

int main() {
    char operator;
    char psa_form1[PSA_LEN];
    char psa_form2[PSA_LEN];

    scanf("%c", &operator);
    while (getchar() != '\n');

    fgets(psa_form1, sizeof(psa_form1), stdin);
    fgets(psa_form2, sizeof(psa_form2), stdin);

    psa_form1[strcspn(psa_form1, "\n")] = '\0';
    psa_form2[strcspn(psa_form2, "\n")] = '\0';

    if (operator == '+') {
        plus(psa_form1, psa_form2);
    }

    else if (operator == '-') {
        minus(psa_form1, psa_form2, 0);
    }

    else if (operator == '=') {
        minus(psa_form1, psa_form2, 1);
    }

    else if (operator == '*') {
        mult(psa_form1, psa_form2);
    }

    else if (operator == '/') {
        divide(psa_form1, psa_form2);
    }


    return 0;
}