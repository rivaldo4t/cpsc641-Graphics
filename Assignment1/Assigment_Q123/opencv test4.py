import numpy as np
import cv2

file_name = 'checkered_512x512'
ext = '.png'
img = cv2.imread(file_name+ext)


# Box Filter - Square Footprint
def gen_box_sq(filter_size):
    box_sq = np.ones((filter_size, filter_size), np.uint8)
    return box_sq


# Box Filter - Circular Footprint
def gen_box_cir(filter_size):
    box_cir = np.ones((filter_size, filter_size), np.uint8)
    for fy in range(filter_size):
        for fx in range(filter_size):
            if ((fx - filter_size / 2) ** 2 + (fy - filter_size / 2) ** 2) > ((filter_size / 2) ** 2):
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
    tri_cir = np.ones((filter_size, filter_size), np.uint8)
    return tri_cir


# Pyramidal Filter
def gen_pyr(filter_size):
    pyr = np.ones((filter_size, filter_size), np.uint8)
    for fy in range(filter_size):
        for fx in range(filter_size):
            if fy <= filter_size / 2 and fx <= filter_size / 2:
                pyr[fy, fx] = (fy + 1) * (fx + 1)
            elif fy > filter_size / 2 >= fx:
                pyr[fy, fx] = ((filter_size / 2) * 2 - fy + 1) * (fx + 1)
            elif fx > filter_size / 2 >= fy:
                pyr[fy, fx] = ((filter_size / 2) * 2 - fx + 1) * (fy + 1)
            else:
                pyr[fy, fx] = ((filter_size / 2) * 2 - fy + 1) * ((filter_size / 2) * 2 - fx + 1)
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
    #temp = 0
    for y in range(0,h,f):
        for x in range(0,w,f):
            weighted_sum = 0
            f_sum = 0
            for fy in range(f):
                for fx in range(f):
                    temp = temp+1
                    weighted_sum = weighted_sum + filter[fy, fx].astype(np.uint64) * img[y, x].astype(np.uint64)
                    f_sum = f_sum + filter[fy, fx].astype(np.uint64)
            mip_map[y / f, x / f] = weighted_sum / f_sum
    #print temp
    return mip_map


m_filter_size = 2
m_filter = gen_box_sq(m_filter_size)
new_img = mip(img, m_filter)
op_file_name = file_name + '_' + str(m_filter_size) + ext
cv2.imwrite(op_file_name, new_img)

m_filter_size = 4
m_filter = gen_box_sq(m_filter_size)
new_img = mip(img, m_filter)
op_file_name = file_name + '_' +  str(m_filter_size) + ext
cv2.imwrite(op_file_name, new_img)

m_filter_size = 8
m_filter = gen_box_sq(m_filter_size)
new_img = mip(img, m_filter)
op_file_name = file_name +  '_' + str(m_filter_size) + ext
cv2.imwrite(op_file_name, new_img)

m_filter_size = 16
m_filter = gen_box_sq(m_filter_size)
new_img = mip(img, m_filter)
op_file_name = file_name +  '_' + str(m_filter_size) + ext
cv2.imwrite(op_file_name, new_img)

m_filter_size = 32
m_filter = gen_box_sq(m_filter_size)
new_img = mip(img, m_filter)
op_file_name = file_name +  '_' + str(m_filter_size) + ext
cv2.imwrite(op_file_name, new_img)

#cv2.imshow('i',newimg)
#cv2.waitKey(0)
#cv2.destroyAllWindows()