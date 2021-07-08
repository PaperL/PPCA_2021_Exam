#include <cstdio>
#include <cstring>
#include <cstdlib>
#include <fstream>
#include <map>
#include <queue>

#define CMSize 32
#define CompressK 14

struct sPixel {
    unsigned char r, g, b;

    explicit sPixel(unsigned char r = 0, unsigned char g = 0, unsigned b = 0) : r(r), g(g), b(b) {}

    void setL2Bz() {
        r &= 0b1111'1100u;
        g &= 0b1111'1100u;
        b &= 0b1111'1100u;
    }

    bool operator<(const sPixel &other) const {
        if (r == other.r) {
            if (g == other.g) {
                return (b < other.b);
            }
            else return (g < other.g);
        }
        else return (r < other.r);
    }

    sPixel compressed() const {
        return sPixel(r / CompressK, g / CompressK, b / CompressK);
    }

    sPixel released() const {
        return sPixel(r * CompressK, g * CompressK, b * CompressK);
    }
};

struct cntPixel {
    int cnt;
    unsigned char r, g, b;

    bool operator<(const cntPixel &other) const {
        return (cnt < other.cnt);
    }
};


class colorMap {
private:
    inline void safeSet(int x, int y, int z, int v) {
        x /= CompressK, y /= CompressK, z /= CompressK;
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
        safeSet(pix.r, pix.g, pix.b, 1);
    }


};

void outputRepeatedPixel(std::fstream &fo, unsigned int time) {
    unsigned char k;
    while (time > 0) {
        if (time <= 0b1'1111u) {
            k = (time << 3u) | 0b011u;
            fo.write(reinterpret_cast<char *>(&k), 1);
            break;
        }
        else if (time <= 0b1'1111'1111'1111u) {
            k = ((time & 0b1'1111u) << 3u) | 0b111u;
            fo.write(reinterpret_cast<char *>(&k), 1);
            k = ((time & 0b1'1111'1110'0000u) >> 5u);
            fo.write(reinterpret_cast<char *>(&k), 1);
            break;
        }
        else {
            k = 0b1111'1111u;
            fo.write(reinterpret_cast<char *>(&k), 1);
            fo.write(reinterpret_cast<char *>(&k), 1);
            time -= 0b1'1111'1111'1111u;
        }
    }
}

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


    std::map<sPixel, unsigned char> Cmap1;
    std::map<sPixel, unsigned int> Cmap2;
    std::priority_queue<cntPixel> pq;
    for (unsigned char i = 0; i < CMSize; ++i)
        for (unsigned char j = 0; j < CMSize; ++j)
            for (unsigned char k = 0; k < CMSize; ++k) {
                // printf("RGB: %d\t%d\t%d,\tCNT: %d\n", i, j, k, cm->map[i][j][k]);
                pq.push(cntPixel{cm->map[i][j][k], i, j, k});
            }

    auto pq_backup(pq);

    for (int id = 0; id < 64; id++) {
        if (pq.empty()) break;
        auto i = pq.top();
        pq.pop();
        Cmap1[sPixel(i.r, i.g, i.b)] = id;
//        if (id % 4 == 0)putchar(10);
//        printf("1:(%d,%d,%d)\t", i.r, i.g, i.b);
    }

    int threshold = 16384;
    if (cm->nzCnt <= 1000) threshold = 512;
    else if (cm->nzCnt <= 2000) threshold = 1024;
    else if (cm->nzCnt <= 3000) threshold = 2048;
    else if (cm->nzCnt <= 5000) threshold = 4096;
    else if (cm->nzCnt <= 7000) threshold = 8192;
    else threshold = 16384;


    for (int id = 0; id < threshold; id++) {
        if (pq.empty()) break;
        auto i = pq.top();
        pq.pop();
        Cmap2[sPixel(i.r, i.g, i.b)] = id;

//        if(id < 1000) {
//            if (id % 4 == 0)putchar(10);
//            printf("2:(%d,%d,%d)\t", i.r, i.g, i.b);
//        }
    }
//    printf("\nnzCnt: %d\nthreshold: %d\n",cm->nzCnt,threshold);
//    printf("ENCODE:\t1:%ld\t2:%ld\n", Cmap1.size(), Cmap2.size());

    std::fstream fo;
    fo.open(path, std::ios::binary | std::ios::out | std::ios::trunc);
    fo.write(reinterpret_cast<char *>(&h), 4);
    fo.write(reinterpret_cast<char *>(&w), 4);

    int oi;
    oi = Cmap1.size();
    fo.write(reinterpret_cast<char *>(&oi), 4);
    for (int id = 0; id < oi; id++) {
        auto i = pq_backup.top();
        pq_backup.pop();
        sPixel pix(i.r, i.g, i.b);
        fo.write(reinterpret_cast<char *>(&pix), 3);
    }
    for (auto &i:Cmap1)


        oi = Cmap2.size();
    fo.write(reinterpret_cast<char *>(&oi), 4);
    for (int id = 0; id < oi; id++) {
        auto i = pq_backup.top();
        pq_backup.pop();
        sPixel pix(i.r, i.g, i.b);
        fo.write(reinterpret_cast<char *>(&pix), 3);
    }

    for (int i = 0; i < h; i++) {
        for (int j = 0; j < w; j++) {
            auto pix = image[i][j].compressed();
            if (Cmap1.count(pix)) {
                unsigned char k = 0b01u;
                auto id = Cmap1[pix];
                k |= id << 2u;
                fo.write(reinterpret_cast<char *>(&k), 1);


                unsigned int repeatCnt = 0;
                int endI, endJ;
                bool breakFlag = false;
                for (int ii = i; ii < h; ++ii) {
                    for (int jj = 0; jj < w; ++jj) {
                        if (ii == i && jj <= j) continue;
                        auto nxtPix = image[ii][jj].compressed();
                        if (Cmap1.count(nxtPix)) {
                            unsigned char nxtK = 0b01u;
                            auto nxtId = Cmap1[nxtPix];
                            nxtK |= nxtId << 2u;
                            if (nxtK == k) {
                                endI = ii, endJ = jj;
                                repeatCnt++;
                            }
                            else {
                                breakFlag = true;
                                break;
                            }
                        }
                        else {
                            breakFlag = true;
                            break;
                        }
                    }
                    if (breakFlag) break;
                }
                if (repeatCnt > 0) {
                    i = endI;
                    j = endJ;
//                    printf("E<%d, %d>: %d\n", i, j, repeatCnt);
                    outputRepeatedPixel(fo, repeatCnt);
                }

            }
            else if (Cmap2.count(pix)) {
                unsigned char k1, k2;
                auto id = Cmap2[pix];
                k1 = ((id & 0b0000'0000'0011'1111u) << 2u) | 0b10u;
                k2 = (id & 0b0011'1111'1100'0000u) >> 6u;
                fo.write(reinterpret_cast<char *>(&k1), 1);
                fo.write(reinterpret_cast<char *>(&k2), 1);


                unsigned int repeatCnt = 0;
                int endI, endJ;
                bool breakFlag = false;
                for (int ii = i; ii < h; ++ii) {
                    for (int jj = 0; jj < w; ++jj) {
                        if (ii == i && jj <= j) continue;
                        auto nxtPix = image[ii][jj].compressed();
                        if (Cmap2.count(nxtPix)) {
                            unsigned char nxtK1, nxtK2;
                            auto nxtId = Cmap2[nxtPix];
                            nxtK1 = ((nxtId & 0b0000'0000'0011'1111u) << 2u) | 0b10u;
                            nxtK2 = (nxtId & 0b0011'1111'1100'0000u) >> 6u;
                            if (nxtK1 == k1 && nxtK2 == k2) {
                                endI = ii;
                                endJ = jj;
                                repeatCnt++;
                            }
                            else {
                                breakFlag = true;
                                break;
                            }
                        }
                        else {
                            breakFlag = true;
                            break;
                        }
                    }
                    if (breakFlag) break;
                }
                if (repeatCnt > 0) {
                    i = endI;
                    j = endJ;
//                    printf("E<%d, %d>: %d\n", i, j, repeatCnt);
                    outputRepeatedPixel(fo, repeatCnt);
                }

            }
            else {
                image[i][j].setL2Bz();
                fo.write(reinterpret_cast<char *>(&image[i][j]), 3);
            }
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

    std::vector<sPixel> RCmap1;
    std::vector<sPixel> RCmap2;
    int CmapSize;
    fi.read(reinterpret_cast<char *>(&CmapSize), 4);
    for (int i = 0; i < CmapSize; ++i) {
        sPixel pix;
        fi.read(reinterpret_cast<char *>(&pix), 3);
        RCmap1.emplace_back(pix);
    }

    fi.read(reinterpret_cast<char *>(&CmapSize), 4);
    for (int i = 0; i < CmapSize; ++i) {
        sPixel pix;
        fi.read(reinterpret_cast<char *>(&pix), 3);
        RCmap2.emplace_back(pix);
    }

//    printf("\nDECODE:\t1:%d\t2:%d\n",RCmap1.size(),RCmap2.size());
//
//    int cnt = 0;
//    for (const auto &i:RCmap1) {
//        if (cnt % 4 == 0) putchar(10);
//        ++cnt;
//        printf("1:(%d,%d,%d)\t", i.r, i.g, i.b);
//    }
//    cnt = 0;
//    for (const auto &i:RCmap2) {
//        if (cnt == 1000) break;
//        if (cnt % 4 == 0) putchar(10);
//        ++cnt;
//        printf("2:(%d,%d,%d)\t", i.r, i.g, i.b);
//    }

    sPixel lastPix;
    unsigned int repetedTime = 0;
    for (int i = 0; i < h; i++)
        for (int j = 0; j < w; j++) {
            if (repetedTime > 0) {
                data[i][j][2] = lastPix.r;
                data[i][j][1] = lastPix.g;
                data[i][j][0] = lastPix.b;
                repetedTime--;
                continue;
            }

            unsigned char ci;
            fi.read(reinterpret_cast<char *>(&ci), 1);
            if ((ci & 0b11u) == 0b01u) {
                unsigned char id = ci >> 2u;
                auto pix = RCmap1[id].released();
                data[i][j][2] = pix.r;
                data[i][j][1] = pix.g;
                data[i][j][0] = pix.b;
                lastPix = pix;
            }
            else if ((ci & 0b11u) == 0b10u) {
                unsigned int id = ci >> 2u;
                unsigned char tid;
                fi.read(reinterpret_cast<char *>(&tid), 1);
                id |= tid << 6u;
                auto pix = RCmap2[id].released();
                data[i][j][2] = pix.r;
                data[i][j][1] = pix.g;
                data[i][j][0] = pix.b;
                lastPix = pix;
            }
            else if ((ci & 0b11u) == 0b11u) {
                if (ci & 0b100u) {
                    repetedTime = (ci & 0b1111'1000u) >> 3u;
                    fi.read(reinterpret_cast<char *>(&ci), 1);
                    repetedTime |= ci << 5u;
                }
                else repetedTime = (ci & 0b1111'1000u) >> 3u;
//                printf("D<%d, %d>: %d\n", i, j, repetedTime);

                data[i][j][2] = lastPix.r;
                data[i][j][1] = lastPix.g;
                data[i][j][0] = lastPix.b;
                repetedTime--;
            }
            else {
                data[i][j][2] = ci;
                fi.read(reinterpret_cast<char *>(&ci), 1);
                data[i][j][1] = ci;
                fi.read(reinterpret_cast<char *>(&ci), 1);
                data[i][j][0] = ci;
                lastPix = sPixel(data[i][j][2], data[i][j][1], data[i][j][0]);
            }
        }

    fi.close();



// ==============================================================
    memcpy(_data, data, sizeof(data));
}