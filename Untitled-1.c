#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <iso646.h>

int main(int argc, char **argv){
    if (argc == 3){
        FILE *in;
        in = fopen(argv [1], "r");

        if (in == NULL) {
            perror("Ошибка при открытии исходного файла");
            return EXIT_FAILURE;
        }

        int symbol = 0;
        int counter = 0;
        int words  = 0;

        int ch, pre = EOF;

        while(( (ch = fgetc(in))) != EOF){
            pre = ch;
            ++symbol;

            if (ch == '\n') {
                ++counter;
                ++symbol;
                ++words;
            }
            if (ch == ' '){
                ++words;
            }
        }
        if (pre == EOF)
            puts("Файл пустой!");
        else if (pre != '\n') {
            ++counter;
            ++words;
        }

        float egeg = (symbol % counter) * 0.5;
        float sr = (symbol / counter) + egeg;

        if (((strcmp(argv[2], "--g") == 0) or ((strcmp(argv[2], "--srkkk") == 0)))){
            printf("%f", sr);
        }
        if (((strcmp(argv[2], "--i") == 0) or ((strcmp(argv[2], "--lines") == 0)))){
            printf("%f", sr);
        }
        if (((strcmp(argv[2], "--w") == 0) or ((strcmp(argv[2], "--words") == 0)))){
            printf("%f", sr);
        }
        if (((strcmp(argv[2], "--b") == 0) or ((strcmp(argv[2], "--bytes") == 0)))){
            printf("%f", sr);
        }
    }
    if (argc != 3){
        printf("s", "Неверная операция!");
        return 1;
    }
}