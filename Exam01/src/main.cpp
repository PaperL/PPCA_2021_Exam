
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <cmath>
#include "compress.hpp"

inline int read() {
    char c = getchar();
    int ret = 0, f = 1;
    while (c > '9' || c < '0') {
        if (c == '-')f = -1;
        c = getchar();
    }
    while (c >= '0' && c <= '9')ret = ret * 10 + c - '0', c = getchar();
    return ret * f;
}

inline double __pw(int a) {
    return 1. * a * a;
}

int main() {
    std::string fname[5] = {"1.in", "3.in", "5.in", "7.in", "9.in"};
    std::string compressedFilename[5] = {"c1", "c3", "c5", "c7", "c9"};
    long tot = 0;
    for (int ti = 0; ti < 5; ++ti) {
        freopen(fname[ti].c_str(), "r", stdin);

        bool PRINT = true;

        FILE *fout;
        if (PRINT) fout = fopen("out.txt", "w");

        int w, h;
        scanf("%d%d", &h, &w);

        unsigned char data[h][w][3];
        memset(data, 0, sizeof data);

        for (int i = 0; i < h; i++)
            for (int j = 0; j < w; j++)
                for (int k = 0; k < 3; k++) {
                    int x = read();
                    data[i][j][k] = (unsigned char) x;
                }

        const char *path = compressedFilename[ti].c_str();
        encode(h, w, (void *) data, path);

        int _w, _h;

        unsigned char _data[h][w][3];
        memset(_data, 0, sizeof _data);

        decode(path, _h, _w, (void *) _data);

        if (_w != w || _h != h) {
            printf("wrong size!!!!!!\n");
            return 0;
        }

        if (PRINT) fprintf(fout, "%d %d\n", h, w);

        double mse = 1., psnr;
        for (int i = 0; i < h; i++)
            for (int j = 0; j < w; j++)
                for (int k = 0; k < 3; k++) {
                    mse += 1. * __pw(data[i][j][k] - _data[i][j][k]);
                    if (PRINT) fprintf(fout, "%d\n", _data[i][j][k]);
                }

        mse /= (3. * h * w);
        psnr = 10 * log10(255. * 255. / mse);

        printf("PSNR: %.4lf dB\n", psnr);

        auto f = fopen(path, "r");
        fseek(f, 0, SEEK_END);
        auto p = ftell(f);
        fclose(f);

        printf("Size: %ld Bytes\n", p);
        tot += p;

        fclose(stdin);
    }
    printf("Tot: %ld\n", tot);
    return 0;
}


