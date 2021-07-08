import os
import cv2
import numpy as np

img_dir = "image"
data_dir = "data"
img_paths = os.listdir(img_dir)
print(img_paths)

for img_path in img_paths:
    if img_path.startswith('.'):
        continue
    img = cv2.imread(img_dir + "/" + img_path)
    r, c, _ = img.shape
    print(r, c)

    in_path = data_dir + "/" + img_path[:-3] + "in"
    print(in_path)
    with open(in_path, 'w') as f:
        f.write(str(r) + " " + str(c) + "\n")
        for i in range(r):
            for j in range(c):
                for _ in range(3):
                    f.write(str(img[i][j][_]) + "\n")
