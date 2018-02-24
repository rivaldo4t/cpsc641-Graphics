
import numpy as np
import cv2
import video
import common

from moviepy.editor import *
import moviepy.video.tools.drawing as dw

# vid = (VideoFileClip("iceland.mp4")
#        .subclip(122, 124)
#        .resize(.8))
#
# vid = (VideoFileClip("norway.mp4")
#        .subclip(7, 12)
#        .resize(.8))

# vid = (VideoFileClip("hg2.mp4")
#         .subclip(4, 24)
#        .resize(0.5))

# vid.write_videofile("hg2_2.mp4")
# vid.write_gif('hg2_2.gif', fps = 15)

vid = (VideoFileClip("is02.mp4")
        .fx( vfx.resize, width=460) # resize (keep aspect ratio)
        .fx( vfx.speedx, 2)) # double the speed
vid.write_gif('is2.gif', fps = 24)

# mask = dw.color_split(vid3.size, p2=(vid3.w, vid3.h), p1=(vid3.w/2, vid3.h/2),
#                       grad_width=0)

# snapshot = (vid.to_ImageClip()
#             .set_duration(vid.duration)
#             .set_mask(ImageClip(mask, ismask=True)))

# snapshot = (vid
#             .crop(x2 = 0.66*vid.w)
#             .to_ImageClip()
#             .set_duration(vid.duration))
#
# snapshot2 = (vid
#             .crop(y2 = 0.7*vid.h)
#             .to_ImageClip()
#             .set_duration(vid.duration))

# composition = CompositeVideoClip([vid, snapshot, snapshot2])
