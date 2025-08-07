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
void getCommaSeparatedValuesFromStream(FILE* in, T* value, Args... args) {
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

void getLegendFromStream(FILE* in, std::map<char, std::string>* legend) {
    char c;
    char key = 0;
    std::string value = "";
    int v = 0;
    int n = 0;
    while ((c = getc(in)) != EOF) {
        if (c == ',') {
            v++;
        } else if (c == '\n') {
            if (n > 0) {
                return;
            }
            (*legend)[key] = value;
            v = 0;
            value.clear();
            n++;
        } else {
            if (v == 0) {
                key = c;
            } else {
                value += c;
            }
            n = 0;
        }
    }
}

/* NOTES:
    Could use fwrite from <cstdio> instead of printf for output to stdout (more efficient)
*/

int main(int argc, char** argv) {
    FILE *input_file = fopen("test_input.txt", "r");
    int x_count, y_count, z_count, parent_x, parent_y, parent_z;
    getCommaSeparatedValuesFromStream(input_file, &x_count, &y_count, &z_count, &parent_x, &parent_y, &parent_z);
    printf("%d, %d, %d, %d, %d, %d\n", x_count, y_count, z_count, parent_x, parent_y, parent_z);

    std::map<char, std::string> legend;
    getLegendFromStream(input_file, &legend);

    for (const auto& e : legend) {
        printf("%c, %s\n", e.first, e.second.c_str());
    }

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