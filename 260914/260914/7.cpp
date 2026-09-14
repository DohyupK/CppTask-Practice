#include <stdio.h>

void FileRW() {
    int a = 11, b = 12, x, y;
    FILE* f = NULL;
    fopen_s(&f, "a.txt", "w");
    fprintf(f, "%d %d", a, b);
    fclose(f);
    fopen_s(&f, "a.txt", "r");
    fscanf_s(f, "%d %d", &x, &y);
    fclose(f);
    printf("%d\n", x + y);
}