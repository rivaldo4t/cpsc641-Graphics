import numpy as np
import cv2
from math import exp, pi

img = cv2.imread('img33.jpeg')

#print img.size
#print img.dtype
#img[:,:,2] = 0
#print img1[0,300]
#print img2.shape
#print img3.shape

#width = img.shape[0]
#height = img.shape[1]
#channel = img.shape[2]
#print width, height
#newimg = np.ones((height, width, channel), np.uint8)

filtersize = 3
#print filtersize/2

#Box Filter - Square Footprint
filter = np.ones((filtersize,filtersize), np.uint8)

'''
#Box Filter - Circular Footprint
for fy in range(filtersize):
    for fx in range(filtersize):
        if ((fx-filtersize/2)**2 + (fy-filtersize/2)**2) > ((filtersize/2)**2) : filter[fy, fx] = 0
'''

'''
#Triangle Filter - Square Footprint
for fy in range(filtersize):
    for fx in range(filtersize):
        if fx>fy: filter[fy, fx] = 0
'''

'''
#Pyramidal Filter
for fy in range(filtersize):
    for fx in range(filtersize):
        if ((fy <= filtersize/2) and (fx <= filtersize/2)) : filter[fy, fx] = (fy+1)*(fx+1)
        elif ((fy > filtersize/2) and (fx <= filtersize/2)): filter[fy, fx] = ((filtersize/2)*2 - fy+1)*(fx+1)
        elif ((fx > filtersize/2) and (fy <= filtersize/2)):filter[fy, fx] = ((filtersize/2)*2 - fx+1) * (fy + 1)
        else : filter[fy, fx] = ((filtersize / 2) * 2 - fy+1) * ((filtersize / 2) * 2 - fx+1)

print filter
'''

padsize = 30
padimage= cv2.copyMakeBorder(img,padsize,padsize,padsize,padsize,cv2.BORDER_CONSTANT,value=[0,0,0])
width = padimage.shape[0]
height = padimage.shape[1]
channel = padimage.shape[2]
newimg2 = np.ones((height, width, channel), np.uint8)

#'''
for y in range(1+padsize, height-padsize-1):
    for x in range(1+padsize, width-padsize-1):
        sum = 0
        filtersum = 0
        for fy in range(filtersize):
            for fx in range(filtersize):
                sum = sum + filter[fy, fx].astype(np.uint64)*padimage[y-filtersize/2+fy, x-filtersize/2+fx].astype(np.uint64)
                filtersum = filtersum + filter[fy, fx].astype(np.uint64)
        newimg2[y, x] = sum / filtersum
        '''
        upleft      = 0 if (y-1<0 or x-1<0) else img[(y-1, x-1)].astype(np.uint16)
        upmid       = 0 if (y-1<0) else img[(y-1, x)].astype(np.uint16)
        upright     = 0 if (y-1<0 or x+1>width) else img[(y-1, x+1)].astype(np.uint16)
        left        = 0 if (x-1<0) else img[(y, x-1)].astype(np.uint16)
        mid         = img[y, x].astype(np.uint16)
        right       = 0 if (x+1>width) else img[(y, x+1)].astype(np.uint16)
        downleft    = 0 if (y+1>height or x-1<0) else img[(y+1, x-1)].astype(np.uint16)
        downmid     = 0 if (y+1>height) else img[(y+1, x)].astype(np.uint16)
        downright   = 0 if (y+1>height or x+1>width) else img[(y+1, x+1)].astype(np.uint16)

        sum         = (upleft+upmid+upright+left+mid+right+downleft+downmid+downright)
        avg         = sum/9
        newimg[y,x] = avg
        '''
cv2.imshow('image',newimg2)
#'''
cv2.waitKey(0)
cv2.destroyAllWindows()