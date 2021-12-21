#include <stdio.h>
#include <string.h>
#include <stdbool.h>
#include <stdlib.h>
#include <iso646.h>
#include <stdint.h>
typedef struct header
{
    char idv3[3];
    char version[2];
    char flag;
    char size[4];
}ID3header;
typedef struct frame
{
    char frameid[4];
    char size[4];
    char flag[2];
    char unicode;
}ID3frame;
struct frame frame;
struct header head;
int byte_to_int(char ls[4]) {
    return (int)ls[0] << 21 | (int)ls[1] << 14 | (int)ls[2] << 7 | (int)ls[3] << 0;
}
void int_to_byte(int x, char *ls, bool flag) {
    if (head.version[0] == 4 or flag) {
        ls[3] = x & 127;
        ls[2] = (x >> 7) & 127;
        ls[1] = (x >> 14) & 127;
        ls[0] = (x >> 21) & 127;
    }
    else {
        ls[3] = x & 127;
        ls[2] = (x >> 8) & 127;
        ls[1] = (x >> 16) & 127;
        ls[0] = (x >> 24) & 127;
    }
}
int show_get(char *filename, char *prop_name, int *old_size, bool flag) {
    FILE *fin = fopen(filename, "rb");
    if (fin == NULL) {
        printf("Здесь нет такого файла\n");
        exit(1);
    }
    fread(&head, 1, 10, fin);
    int tag_size = byte_to_int(head.size);
    int ready_position = 0;

    while (fread(&frame, 1, 11, fin)) {
        if ((ftell(fin) >= tag_size or frame.frameid[0] == 0) and (prop_name != NULL)) {
            exit(1);
        }
        else if (ftell(fin) >= tag_size or frame.frameid[0] == 0){
            break;}

        int frame_size = byte_to_int(frame.size);
        char *ls = (char *)calloc(frame_size, 1);
        fread(ls, 1, frame_size - 1, fin); // Запоминаем значение фрейма
        ls[frame_size - 1] = 0;             // обнуляем последний элемент фрейма
        if (prop_name == NULL) {
            printf("%s: ", frame.frameid, frame_size);
            if (frame.unicode) {
                wprintf(L"%ls\n", ls);
            }
            else printf("%s\n", ls);
        }
        else if (!strcmp(frame.frameid, prop_name)) {
            if (flag == 0) {
                printf("%s:", frame.frameid); //вывод одного фрейма
                if (frame.unicode) wprintf(L"%ls\n", ls);
                else printf("%s\n", ls);
            }
            else {
                *old_size = frame_size;
                ready_position = ftell(fin) - frame_size - 10;
                break;
            }
        }
        free(ls);
    }
    fclose(fin);
    return ready_position;
}
void set(char *filename, char *prop_name, char *prop_value) {
    FILE *file_in = fopen(filename, "rb");
    if (file_in == NULL) {
        printf("Здесь нет такого файла\n");
        exit(1);
    }
    FILE *temp_file = fopen("temp_file", "wb"); //временный файл для записи
    int *old_size = calloc(1, sizeof(int));
    int pos_to_write = show_get(filename, prop_name, old_size, 1);
    int size_difference = strlen(prop_value) - (*old_size - 1);
    char size_of_frame[4];
    fread(&head, 1, 10, file_in);

    int size_of_struct = byte_to_int(head.size); //размер всей структуры ID3 включая хедер
    size_of_struct = size_of_struct + size_difference;                //обновили размер структуры в соответствии с новым фреймом
    int_to_byte(size_of_struct, head.size, 0);
    fwrite(&head, 1, 10, temp_file);
    char *ls = (char *)malloc(pos_to_write - 10);
    fread(ls, 1, pos_to_write - 10, file_in);
    fwrite(ls, 1, pos_to_write - 10, temp_file); //записали все байты до фрейма, теперь пишем фрейм
    free(ls);

    memcpy(frame.frameid, prop_name, 4); //копируем 4 байта из prop_name в frameid
    int size_of_tag = byte_to_int(head.size);

    size_of_tag = size_of_tag + size_difference;
    int_to_byte(size_of_tag, head.size, 0);
    int_to_byte(strlen(prop_value) + 1, size_of_frame, 1);

    memcpy(frame.size, size_of_frame, 4); //копируем 4 байта из size_of_frame в size
    fwrite(&frame, 1, 11, temp_file);
    fwrite(prop_value, 1, strlen(prop_value), temp_file);
    fseek(file_in, 0, SEEK_END);

    int read_from = *old_size ? pos_to_write + 10 + *old_size : pos_to_write;
    int read_size = ftell(file_in) - read_from;

    ls = (char *)malloc(read_size);//выделяем место для нового фрейма
    fseek(file_in, read_from, SEEK_SET);
    fread(ls, 1, read_size, file_in);//запоминаем значение фрейма
    fwrite(ls, 1, read_size, temp_file);//записали новый фрейм
    free(ls);
    fclose(file_in);
    fclose(temp_file);
    remove(filename);
    rename("temp_file", filename);
}

int main(int argc, char *argv[]) {
    if (argc<3){
        printf ("Неверное число аргументов");
        exit(1);
    }
    strtok(argv[1], "=");
    char* filepath = strtok(NULL, "");
    if (argc == 3) {
        if ((strcmp(argv[2], "--show") == 0)) {
            show_get(filepath, NULL, NULL, 0);
        }
        else  if (strtok(argv[2], "=")){
            char* name = strtok(NULL, "");
            show_get(filepath, name, NULL, 0);}
    }
    else if (strtok(argv[2], "=")){
        strtok(argv[2], "=");
        char* prop_name = strtok(NULL, "");
        strtok(argv[3], "=");
        char* prop_value = strtok(NULL, "");
        set(filepath, prop_name, prop_value);
    }
    return 0;
}