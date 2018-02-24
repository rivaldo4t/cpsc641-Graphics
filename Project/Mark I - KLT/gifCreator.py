import numpy as np
import cv2
import video
import common
from moviepy.editor import *
import moviepy.video.tools.drawing as dw

src = "icceland2.mp4"
vid = VideoFileClip(src)
inselection = None
mask = None
paused = False


def on_rect(rect):
    global inselection
    inselection= rect


cap = video.create_capture(src)
cv2.namedWindow('cin')
rect_sel = common.RectSelector('cin', on_rect)

while True:
    playing = not paused #and not rect_sel.dragging
    if playing:
        ret, frame = cap.read()
        if not ret:
            break

    rect_sel.draw(frame)
    cv2.imshow('cin', frame)

    ch = cv2.waitKey(1)
    if ch == 27:
        break
    if ch == ord(' '):
        paused = not paused
    if ch == ord('c'):
        if inselection is not None:
            maskShape = (frame.shape[0], frame.shape[1])
            mask = np.ones(maskShape)
            x0, y0, x1, y1 = inselection
            mask[y0:y1, x0:x1] = np.zeros((y1 - y0, x1 - x0))
            mask = cv2.GaussianBlur(mask, (15, 15), 0)
            break


snapshot = (vid.to_ImageClip(5)
            .set_duration(vid.duration)
            .set_mask(ImageClip(mask, ismask=True)))

composition = CompositeVideoClip([vid, snapshot])
composition.write_gif('vid13.gif', fps=15)
cv2.destroyAllWindows()
