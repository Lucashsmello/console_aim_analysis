import cv2 as cv
from math import ceil
import numpy as np


def imageDistance(img1, img2):
    return ((img1-img2)**2).mean()


class AimEstimationException(Exception):
    def __init__(self):
        self.message = "Could not estimate speed!"
        super().__init__(self.message)


class AimEstimator360:
    def __init__(self, similarity_threshold=8):
        self.similarity_threshold = similarity_threshold

    def estimateSpeed(self, vcap: cv.VideoCapture):
        """
        returns: degrees per frame
        """
        _, first_img = vcap.read()
        # cv.imshow('first img', first_img)
        # cv.waitKey(1)
        is_starting = True
        m = 10000
        n_frames = 0
        while(True):
            _, img = vcap.read()
            if(img is None):
                break
            n_frames += 1
            distance = imageDistance(first_img, img)
            if(distance > 2*self.similarity_threshold+1):
                is_starting = False

            # cv.imshow('frame', img)
            # key = cv.waitKey(1) & 0xFF
            # if(key == ord('q')):
            #     break
            if(not is_starting):
                if(distance < self.similarity_threshold):
                    break
        min_distance = distance
        while(True):
            _, img = vcap.read()
            if(img is None):
                break
            distance = imageDistance(first_img, img)
            if(distance > min_distance):
                return 360/n_frames
            min_distance = distance
            n_frames += 1
        raise AimEstimationException()


class AimEstimatorPivot:
    def __init__(self, vcap_pivot, pivot_degrees_perframe):
        self.pivot_degrees_perframe = pivot_degrees_perframe
        nframes = ceil(360/pivot_degrees_perframe)
        self.frames = []
        _, img = vcap_pivot.read()
        while(img is not None and len(self.frames) < nframes):
            self.frames.append(img)
            _, img = vcap_pivot.read()
        self.frames = np.array(self.frames)

    def _estimateAngle(self, img):
        if(len(img.shape) == 3):
            dims = (1, 2, 3)
        else:
            dims = (1, 2)
        distance = ((self.frames-img)**2).mean(axis=dims)
        i = np.argmin(distance)
        assert(distance[i] <= 32), "Pivot video does not seem to have something in common with the video file!"
        return i*self.pivot_degrees_perframe

    def estimateSpeed(self, vcap):
        _, img = vcap.read()
        estimated_speeds = []
        angle1 = self._estimateAngle(img)
        while(img is not None):
            _, img = vcap.read()
            angle2 = self._estimateAngle(img)
            ang_diff = angle2-angle1
            if(ang_diff < -90):
                ang_diff += 360
            elif(ang_diff > 90):
                ang_diff -= 360
            estimated_speeds.append(ang_diff)
            if(len(estimated_speeds) >= 60):
                break
            angle1 = angle2
        return np.mean(estimated_speeds)
