#include <stdio.h>
#include <stdlib.h>
#include <cstdio>
#include <iostream>
#include <string>
#include <map>

void PreviousVersion_getDimensionsFromStdin(int* xc, int* yc, int* zc, int* px, int* py, int* pz) {
    char c;
    printf("%p, %p, %p, %p, %p, %p\n", xc, yc, zc, px, py, pz);
    int *value = xc;
    while ((c = getc(stdin)) != EOF) {
        if (c == ',' || c == '\n') {
            value = yc;
            *value = 0;
        } else {
            *value *= 10;
            *value += (int)c - '0';
        }
    }
}

void getCommaSeparatedValuesFromStream(FILE *in) {}

template <typename T, typename... Args>
void getCommaSeparatedValuesFromStream(FILE *in, T* value, Args... args) {
    char c;
    *value = 0;
    while ((c = getc(in)) != EOF) {
        if (c == ',' || c == '\n') {
            break;
        } else {
            *value *= 10;
            *value += (int)c - '0';
        }
    }
    getCommaSeparatedValuesFromStream(in, args...);
}

int main(int argc, char** argv) {
    FILE *input_file = fopen("test_input.txt", "r");
    int x_count, y_count, z_count, parent_x, parent_y, parent_z;
    getCommaSeparatedValuesFromStream(input_file, &x_count, &y_count, &z_count, &parent_x, &parent_y, &parent_z);
    printf("%d, %d, %d, %d, %d, %d\n", x_count, y_count, z_count, parent_x, parent_y, parent_z);
    std::map<char, std::string> legend;
    char key;
    char value[8];
    getCommaSeparatedValuesFromStream(input_file, &key, &value);
    printf("%s, %s\n", key, value);
    /*
    std::string line;
    while (getline(input_file, line)) {
        printf("%s\n", line.c_str());
    }
    */
    fclose(input_file);
    printf("Done!\n");
    return 0;
}