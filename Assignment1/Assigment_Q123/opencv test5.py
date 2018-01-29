import numpy as np
import cv2
from matplotlib import pyplot as plt

file_name = 'photo_512x512'
ext = '.jpeg'
img = cv2.imread(file_name+ext)


# Box Filter - Square Footprint
def gen_box_sq(filter_size):
    box_sq = np.ones((filter_size, filter_size), np.uint8)
    return box_sq


# Box Filter - Circular Footprint
def gen_box_cir(filter_size):
    if (filter_size == 2):
        return gen_box_sq(2)

    box_cir = np.ones((filter_size, filter_size), np.uint8)
    for fy in range(filter_size):
        for fx in range(filter_size):
            radius = (filter_size / 2)
            xr = radius - 0.5
            yr = radius - 0.5
            if (fx - xr) ** 2 + (fy - yr) ** 2 > radius ** 2:
                box_cir[fy, fx] = 0
    return box_cir


# Triangle Filter - Square Footprint
def gen_tri_sq(filter_size):
    tri_sq = np.ones((filter_size, filter_size), np.uint8)
    for fy in range(filter_size):
        for fx in range(filter_size):
            if fx > fy:
                tri_sq[fy, fx] = 0
    return tri_sq


# Triangle Filter - Circular Footprint
def gen_tri_cir(filter_size):
    tri = gen_tri_sq(filter_size)
    for fy in range(filter_size):
        for fx in range(filter_size):
            radius = (filter_size / 2)
            xr = radius - 0.5
            yr = radius - 0.5
            if (fx - xr) ** 2 + (fy - yr) ** 2 > radius ** 2:
                tri[fy, fx] = 0
    return tri


# Pyramidal Filter
def gen_pyr(filter_size):
    pyr = np.ones((filter_size, filter_size), np.uint8)
    for fy in range(filter_size):
        for fx in range(filter_size):
            if fy < filter_size / 2 and fx < filter_size / 2:
                pyr[fy, fx] = (fy + 1) * (fx + 1)
            elif fy >= filter_size / 2 > fx:
                pyr[fy, fx] = ((filter_size / 2) * 2 - fy) * (fx + 1)
            elif fx >= filter_size / 2 > fy:
                pyr[fy, fx] = ((filter_size / 2) * 2 - fx) * (fy + 1)
            else:
                pyr[fy, fx] = ((filter_size / 2) * 2 - fy) * ((filter_size / 2) * 2 - fx)
    return pyr


def mip(img, filter):
    f = filter.shape[0]
    w = img.shape[0]
    h = img.shape[1]
    c = img.shape[2]
    map_w = w // f if w % f == 0 else (w // f) + 1
    map_h = h // f if h % f == 0 else (h // f) + 1

    mip_map = np.ones((map_h, map_w, c), np.uint8)

    print filter
    print f
    print mip_map.shape

    for y in range(0,h,f):
        for x in range(0,w,f):
            weighted_sum = 0
            f_sum = 0
            for fy in range(f):
                for fx in range(f):
                    weighted_sum = weighted_sum + filter[fy, fx].astype(np.uint64) * img[y, x].astype(np.uint64)
                    f_sum = f_sum + filter[fy, fx].astype(np.uint64)
            mip_map[y / f, x / f] = weighted_sum / f_sum

    return mip_map


img2 = np.ones((img.shape[0], 3*img.shape[1]/2, img.shape[2]), np.uint8)
img2[0:img.shape[0], 0:img.shape[1]] = img


for i in range(9):
    m_filter_size = 2 ** (i+1)
    m_filter = gen_pyr(m_filter_size)
    new_img = mip(img, m_filter)
    op_file_name = file_name + '_' + str(m_filter_size) + ext
    #cv2.imwrite(op_file_name, new_img)
    img2[new_img.shape[0]: 2*new_img.shape[0], img.shape[1]: img.shape[1] + new_img.shape[1]] = new_img


cv2.imshow('MipMap Levels', img2)

cv2.waitKey(0)
cv2.destroyAllWindows()