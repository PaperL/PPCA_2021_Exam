#include <cstdio>
#include <cstring>
#include <cstdlib>
#include <fstream>


void encode(int h, int w, void *_data, const char *path) {
    unsigned char data[h][w][3];
    memcpy(data, _data, sizeof(data));



    {
        using namespace std;
        fstream fs;
        fs.
        auto f = fopen(path, "w");
        fprintf(f, "%d %d\n", h, w);
        for (int i = 0; i < h; i++)
            for (int j = 0; j < w; j++)
                for (int k = 0; k < 3; k++)
                    fprintf(f, "%d ", data[i][j][k]);
        fclose(f);

    }
}

void decode(const char *path, int &h, int &w, void *_data) {
    {


        auto f = fopen(path, "r");
        fscanf(f, "%d%d", &h, &w);
        unsigned char data[h][w][3];

        for (int i = 0; i < h; i++)
            for (int j = 0; j < w; j++)
                for (int k = 0; k < 3; k++) {
                    int x;
                    fscanf(f, "%d", &x);
                    data[i][j][k] = (unsigned char) x;
                }
        fclose(f);
    }


    memcpy(_data, data, sizeof(data));
}
