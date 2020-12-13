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
    def __init__(self, similarity_threshold=4):
        self.similarity_threshold = similarity_threshold
        self.retry = 1

    def estimateSpeed(self, vcap: cv.VideoCapture):
        """
        returns: degrees per frame
        """
        retry = self.retry
        similarity_threshold = self.similarity_threshold

        _, first_img = vcap.read()
        # cv.imshow('first img', first_img)
        # cv.waitKey(1)
        is_starting = True
        n_frames = 0
        backup_frame_id = 0
        while(True):
            try:
                while(True):
                    _, img = vcap.read()
                    if(img is None):
                        raise AimEstimationException()
                    n_frames += 1
                    distance = imageDistance(first_img, img)
                    if(distance > 2*similarity_threshold+1):
                        is_starting = False

                    #cv.imshow('frame', img)
                    #key = cv.waitKey(1) & 0xFF
                    # if(key == ord('q')):
                    #    break
                    if(not is_starting):
                        if(distance < similarity_threshold):
                            backup_frame_id = vcap.get(cv.CAP_PROP_POS_FRAMES)
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
            except AimEstimationException as e:
                if(retry == 0):
                    raise AimEstimationException()
                retry -= 1
                similarity_threshold *= 2
                vcap.set(cv.CAP_PROP_POS_FRAMES, backup_frame_id)


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

    def _estimateAngle(self, img, degree_begin=0, degree_end=360):
        idx_begin = int(degree_begin/self.pivot_degrees_perframe)
        idx_end = int(degree_end/self.pivot_degrees_perframe)+2
        if(len(img.shape) == 3):
            dims = (1, 2, 3)
        else:
            dims = (1, 2)

        idx_begin -= 1  # conservative
        idx_begin = max(idx_begin, 0)
        if(idx_begin > idx_end):
            frames1 = self.frames[idx_begin:]
            frames2 = self.frames[:idx_end+1]
            distance1 = ((frames1-img)**2).mean(axis=dims)
            distance2 = ((frames2-img)**2).mean(axis=dims)
            i1 = np.argmin(distance1)
            i2 = np.argmin(distance2)
            if(distance1[i1] < distance2[i2]):
                i = i1+idx_begin
                mindist = distance1[i1]
            else:
                i = i2
                mindist = distance2[i2]
        else:
            frames = self.frames[idx_begin:idx_end+1]
            distance = ((frames-img)**2).mean(axis=dims)
            i = np.argmin(distance)
            mindist = distance[i]
            i += idx_begin
        assert(mindist <= 30), "Pivot video does not seem to have something in common with the video file!"
        return i*self.pivot_degrees_perframe

    def estimateSpeed(self, vcap):
        _, img = vcap.read()
        estimated_speeds = []
        angle1 = self._estimateAngle(img)
        while(img is not None):
            _, img = vcap.read()
            if(len(estimated_speeds) >= 2):
                avg_speed = np.mean(estimated_speeds)
                max_speed = np.max(estimated_speeds)
                angle2 = self._estimateAngle(img, angle1, angle1+avg_speed+max_speed)
            else:
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
