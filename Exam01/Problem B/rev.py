
import math
import cv2
import numpy as np

with open('out.txt', 'r') as f:
    a = f.read().split('\n')
    hw = a[0].split(' ')
    h, w = int(hw[0]), int(hw[1])

    ex = np.zeros((h, w, 3))

    b = a[1].split(' ')
    tt = 0
    for i in range(h):
        for j in range(w):
            for k in range(3):
                ex[i][j][k] = int(b[tt])
                tt += 1

cv2.imwrite('save.png', ex)
