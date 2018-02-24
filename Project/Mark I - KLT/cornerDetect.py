import numpy as np
import cv2
from matplotlib import pyplot as plt

img = cv2.imread('1.png')
gray = cv2.cvtColor(img,cv2.COLOR_RGB2GRAY)

corners = cv2.goodFeaturesToTrack(gray,50,0.01,10)
# print corners
corners = np.int0(corners)


for i in corners:
    x,y = i.ravel()
    # print i.ravel()
    #  print i
    cv2.circle(img,(x,y),3,255,-1)

plt.imshow(img),plt.show()