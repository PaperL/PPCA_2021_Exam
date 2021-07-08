#include <cstdio>
#include <cstring>
#include <cstdlib>
#include <fstream>
#include <map>
#include <queue>

struct pixel {
    int r = 0, g = 0, b = 0;
    int l = 0, c = 0;   // 亮度 & 对比度
};

struct sPixel {
    unsigned char r, g, b;

    explicit sPixel(unsigned char r = 0, unsigned char g = 0, unsigned b = 0) : r(r), g(g), b(b) {}

    void setLSBz() {
        r &= 0b1111'1110u;
        g &= 0b1111'1110u;
        b &= 0b1111'1110u;
    }

    bool operator<(const sPixel &other) const {
        if (r == other.r) {
            if (g == other.g) {
                return (b < other.b);
            } else return (g < other.g);
        } else return (r < other.r);
    }
};

struct cntPixel {
    int cnt;
    unsigned char r, g, b;

    bool operator<(const cntPixel &other) const {
        return (cnt < other.cnt);
    }
};

#define CMSize 64

class colorMap {
private:
    inline void safeSet(int x, int y, int z, int v) {
        x /= 4, y /= 4, z /= 4;
        if (x >= 0 && x < CMSize && y >= 0 && y < CMSize && z >= 0 && z < CMSize) {
            if (map[x][y][z] == 0) {
                nzCnt++;
            }
            map[x][y][z] += v;
        }
    }

public:
    int map[CMSize][CMSize][CMSize] = {0};   // 色彩分布
    int nzCnt = 0;  // not zero cnt

    void set(sPixel pix) {
//        const int _k = 1;
//        for (int i = -_k; i <= _k; ++i)
//            for (int j = -_k; j <= _k; ++j)
//                for (int k = -_k; k <= _k; ++k)
        safeSet(pix.r, pix.g, pix.b, 1);
    }


};

void encode(int h, int w, void *_data, const char *path) {
    unsigned char data[h][w][3];
    memcpy(data, _data, sizeof(data));
// ==============================================================

    sPixel image[h][w];
    auto *cm = new colorMap;

    for (int i = 0; i < h; ++i) {
        for (int j = 0; j < w; ++j) {
            image[i][j] = sPixel(data[i][j][2], data[i][j][1], data[i][j][0]);
            cm->set(image[i][j]);
        }
    }
//    printf("CM Init Finish. (%d)\n", cm->nzCnt);


    std::map<sPixel, unsigned char> Cmap;
    std::priority_queue<cntPixel> pq;
    for (unsigned char i = 0; i < CMSize; ++i)
        for (unsigned char j = 0; j < CMSize; ++j)
            for (unsigned char k = 0; k < CMSize; ++k) {
                // printf("RGB: %d\t%d\t%d,\tCNT: %d\n", i, j, k, cm->map[i][j][k]);
                pq.push(cntPixel{cm->map[i][j][k], i, j, k});
            }

    for (unsigned char id = 0; id < 128; id++) {
        if (pq.empty()) break;
        auto i = pq.top();
        pq.pop();
        Cmap[sPixel(i.r, i.g, i.b)] = id;
//        if (id % 8 == 0)putchar(10);
//        printf("<(%d,%d,%d), %d>\t", i.r, i.g, i.b, id);
    }

//    printf("%d %d\n", sizeof(h), sizeof(w));
//    printf("%d\n", sizeof(int));
//    printf("%d %d\n", sizeof(Cmap.begin()->first), sizeof(Cmap.begin()->second));
//    printf("%d %d\n", sizeof(unsigned char), sizeof(sPixel));

    std::fstream fo;
    fo.open(path, std::ios::binary | std::ios::out | std::ios::trunc);
    fo.write(reinterpret_cast<char *>(&h), 4);
    fo.write(reinterpret_cast<char *>(&w), 4);

    int oi;
    oi = Cmap.size();
    fo.write(reinterpret_cast<char *>(&oi), 4);
    for (auto &i:Cmap) {
        auto pix = i.first;
        auto id = i.second;
        fo.write(reinterpret_cast<char *>(&pix), 3);
        fo.write(reinterpret_cast<char *>(&id), 1);
    }

    for (int i = 0; i < h; i++)
        for (int j = 0; j < w; j++) {
            if (Cmap.count(image[i][j])) {
                unsigned char k = 1u;
                k |= Cmap[image[i][j]] << 1u;
                fo.write(reinterpret_cast<char *>(&k), 1);
            } else {
                image[i][j].setLSBz();
                fo.write(reinterpret_cast<char *>(&image[i][j]), 3);
            }
        }
    fo.close();
}

void decode(const char *path, int &h, int &w, void *_data) {
    std::fstream fi;
    fi.open(path, std::ios::binary | std::ios::in);
    fi.read(reinterpret_cast<char *>(&h), 4);
    fi.read(reinterpret_cast<char *>(&w), 4);

    unsigned char data[h][w][3];

    std::map<unsigned char, sPixel> RCmap;
    int CmapSize;
    fi.read(reinterpret_cast<char *>(&CmapSize), 4);
    for (int i = 0; i < CmapSize; ++i) {
        sPixel pix;
        unsigned char id;
        fi.read(reinterpret_cast<char *>(&pix), 3);
        fi.read(reinterpret_cast<char *>(&id), 1);
        RCmap[id] = pix;
    }

    for (int i = 0; i < h; i++)
        for (int j = 0; j < w; j++) {
            unsigned char ci;
            fi.read(reinterpret_cast<char *>(&ci), 1);
            if (ci & 0b1u) {
                unsigned char id = ci >> 1u;
                const auto &pix = RCmap[id];
                data[i][j][2] = pix.r;
                data[i][j][1] = pix.g;
                data[i][j][0] = pix.b;
            } else {
                data[i][j][2] = ci;
                fi.read(reinterpret_cast<char *>(&ci), 1);
                data[i][j][1] = ci;
                fi.read(reinterpret_cast<char *>(&ci), 1);
                data[i][j][0] = ci;
            }
        }

    fi.close();



// ==============================================================
    memcpy(_data, data, sizeof(data));
}
