#!/usr/bin/env python

import numpy as np
import cv2
import video
import common
from common import draw_str

lk_params = dict(winSize  = (19, 19),
                 maxLevel = 2,
                 criteria = (cv2.TERM_CRITERIA_EPS | cv2.TERM_CRITERIA_COUNT, 10, 0.03))

feature_params = dict(maxCorners = 1000,
                      qualityLevel = 0.01,
                      minDistance = 8,
                      blockSize = 19)


def checkedTrace(img0, img1, p0, back_threshold = 1.0):
    p1, st, err = cv2.calcOpticalFlowPyrLK(img0, img1, p0, None, **lk_params)
    p0r, st, err = cv2.calcOpticalFlowPyrLK(img1, img0, p1, None, **lk_params)
    d = abs(p0-p0r).reshape(-1, 2).max(-1)
    status = d < back_threshold
    return p1, status


green = (0, 255, 0)
red = (0, 0, 255)


class App:
    def __init__(self, video_src):
        self.cam = self.cam = video.create_capture(video_src)
        self.p0 = None
        self.p1 = None
        self.gray0 = None
        self.gray1 = None
        self.frame0 = None
        self.inselection = None
        cv2.namedWindow('Hgraphy')
        self.rect_sel = common.RectSelector('Hgraphy', self.on_rect)

    def on_rect(self, rect):
        self.inselection = rect
        # print self.inselection

    def run(self):
        while True:
            ret, frame = self.cam.read()
            if not ret:
                break
            frame_gray = cv2.cvtColor(frame, cv2.COLOR_BGR2GRAY)
            vis = frame.copy()
            if self.p0 is not None:
                p2, trace_status = checkedTrace(self.gray1, frame_gray, self.p1)

                self.p1 = p2[trace_status].copy()
                self.p0 = self.p0[trace_status].copy()
                self.gray1 = frame_gray

                if len(self.p0) < 4:
                    self.p0 = None
                    continue

                H, status = cv2.findHomography(self.p0, self.p1)
                h, w = frame.shape[:2]
                overlay = cv2.warpPerspective(self.frame0, H, (w, h))
                vis = cv2.addWeighted(vis, 0.5, overlay, 0.5, 0.0)

                for (x0, y0), (x1, y1), good in zip(self.p0[:,0], self.p1[:,0], status[:,0]):
                    if good:
                        cv2.line(vis, (x0, y0), (x1, y1), (0, 128, 0))
                    cv2.circle(vis, (x1, y1), 2, (red, green)[good], -1)

                draw_str(vis, (20, 20), 'track count: %d' % len(self.p1))

            else:
                p = cv2.goodFeaturesToTrack(frame_gray, **feature_params)
                if p is not None:
                    for x, y in p[:,0]:
                        cv2.circle(vis, (x, y), 2, green, -1)
                    draw_str(vis, (20, 20), 'feature count: %d' % len(p))

            self.rect_sel.draw(vis)
            cv2.imshow('Hgraphy', vis)

            ch = cv2.waitKey(1)
            if ch == 27:
                break
            if ch == ord(' '):
                self.frame0 = frame.copy()
                mask1 = None
                if self.inselection is not None:
                    mask1 = np.zeros_like(frame_gray)
                    x0, y0, x1, y1 = self.inselection
                    mask1[y0:y1, x0:x1] = np.ones((y1-y0, x1-x0))

                self.p0 = cv2.goodFeaturesToTrack(frame_gray, mask = mask1, **feature_params)

                if self.p0 is not None:
                    self.p1 = self.p0
                    self.gray0 = frame_gray
                    self.gray1 = frame_gray


def main():
    # video_src = "optFlow1.mp4"
    video_src = 0
    App(video_src).run()
    cv2.destroyAllWindows()


if __name__ == '__main__':
    main()
