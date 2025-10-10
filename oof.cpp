#include <stdio.h>
#include <cstdint>
#include <cstring>
#include <cstdlib>

struct bitmap {
    uint64_t *map;
    int x;
    int y;
    int z;
    int x_size;
    
    bitmap(int c_x, int c_y, int c_z) {
        x = c_x;
        y = c_y;
        z = c_z;
        x_size = x / 64;
        if (x % 8 > 0) {
            x_size++;
        }
        map = (uint64_t*)calloc(x_size * y * z, sizeof(uint64_t));
    }

    ~bitmap() {
        free(map);
    }

    uint64_t& get_seg(int a, int b, int c) {
        return (map[(a / 64) + (b * x_size) + (c * x_size * y)]);
    }

    bool get(int a, int b, int c) {
        int xi = a % 64;
        return ((map[(a / 64) + (b * x_size) + (c * x_size * y)]) >> (63-xi)) & 1ULL;
    }

    void set(int a, int b, int c, uint64_t value) {
        int xi = a % 64;
        if (value > 0) {
            while (value < 9223372036854775808) {
                value <<= 1;
            }
            get_seg(a, b, c) |= (value << (63-xi));
        } else {
            get_seg(a, b, c) &= ~(1ULL << (63-xi));
        }
    }

    inline bool compare(const bitmap* a, int a_x, int a_y, int a_z, int b_y, int b_z, int x_len) {
        if (x_len == 0) {
            return false;
        }

        uint64_t a = 0ULL;
        a |= ((get_seg(a_x, a_y, a_z) << a_xi));

        for (int i = a_y; i <= b_y; i++) {
            for (int o = a_z; o <= b_z; o++) {
                uint64_t b = 0ULL;
                b |= ((get_seg(a_x, i, o) << a_xi));
                if (a != b) {
                    return ;
                }
            }
        }
    }

    void print() {
        for (int zi = 0; zi < z; zi++) {
            for (int yi = 0; yi < y; yi++) {
                for (int xi = 0; xi < x; xi++) {
                    printf("%d", get(xi, yi, zi));
                }
                printf("\n");
            }
            printf("\n");
        }
    }
};

int main() {
    bitmap b(68, 3, 1);
    b.set(3, 1, 0, 1);
    b.set(4, 1, 0, 1);
    b.set(3, 1, 0, 0);
    b.set(66, 1, 0, 1);
    b.print();
    return 0;
}