import cv2 as cv
from math import ceil
import numpy as np


def imageDistance(img1, img2):
    return ((img1-img2)**2).mean()


class AimEstimationException(Exception):
    def __init__(self, message="Could not estimate speed!"):
        self.message = message
        super().__init__(self.message)


class AimEstimator360:
    def __init__(self, similarity_threshold=32, max_images_read_buffer=30):
        self.similarity_threshold = similarity_threshold
        self.retry = 0  # number of times to estimate aim. Each new retry, the similarity_threshold is doubled.
        self.max_images_read_buffer = max_images_read_buffer

    def estimateSpeed(self, vcap: cv.VideoCapture):
        """
        returns: degrees per frame
        """

        def most_similar_image(previous_images, cur_img):
            """
            Returns the most similar image of cur_img from the list of images previous_images.
            """
            distances = [imageDistance(img, cur_img) for img in previous_images]
            i = np.argmin(distances)
            return i, distances[i]

        retry = self.retry
        similarity_threshold = self.similarity_threshold

        _, first_img = vcap.read()
        imgs_read = [first_img]
        n_frames = 0
        backup_frame_id = 0

        ####read images until a 'very' different image, with respect to first_image, is found. ###
        while(True):
            _, img = vcap.read()
            if(img is None):
                raise AimEstimationException()
            n_frames += 1
            distance = imageDistance(first_img, img)
            if(len(imgs_read) < self.max_images_read_buffer):
                imgs_read.append(img)
            if(distance > 2*similarity_threshold+1 and n_frames >= 7):
                break
        ############################

        # cv.imshow('first_img', first_img)
        # cv.waitKey(1)
        backup_frame_id = vcap.get(cv.CAP_PROP_POS_FRAMES)  # used on the next try, if happens.
        backup_frame_num = n_frames

        while(self.retry >= 0):
            try:
                while(distance >= similarity_threshold):
                    _, img = vcap.read()
                    if(img is None):
                        raise AimEstimationException("No similar image found")
                    n_frames += 1
                    similar_img_idx, distance = most_similar_image(imgs_read[:n_frames-backup_frame_num+1], img)
                    if(len(imgs_read) < self.max_images_read_buffer):
                        imgs_read.append(img)

                min_distance = distance
                similar_img_idx_best = similar_img_idx

                # cv.imshow('best match', imgs_read[similar_img_idx_best])
                # cv.waitKey(1)

                # Continue to read images until similarity no longer decreases.
                while(True):
                    _, img = vcap.read()
                    if(img is None):
                        break
                    similar_img_idx, distance = most_similar_image(imgs_read[:n_frames-backup_frame_num+1], img)
                    if(distance > min_distance):
                        break
                    min_distance = distance
                    similar_img_idx_best = similar_img_idx
                    n_frames += 1
                return 360/(n_frames-similar_img_idx_best)
            except AimEstimationException as e:
                if(retry == 0):
                    raise e
                retry -= 1
                similarity_threshold *= 2
                vcap.set(cv.CAP_PROP_POS_FRAMES, backup_frame_id)
                n_frames = backup_frame_num
                imgs_read = imgs_read[:n_frames+1]
                distance = similarity_threshold+1


class AimEstimatorPivot:
    def __init__(self, vcap_pivot, pivot_degrees_perframe, sample_size=60):
        self.pivot_degrees_perframe = pivot_degrees_perframe
        self.sample_size = sample_size
        nframes = ceil(360/pivot_degrees_perframe)
        self.frames = []
        _, img = vcap_pivot.read()
        while(img is not None and len(self.frames) < nframes):
            self.frames.append(img)
            _, img = vcap_pivot.read()
        self.frames = np.array(self.frames)

    def _estimateAngle(self, img, degree_begin=0, degree_end=360):
        degree_begin = max(0, degree_begin)
        degree_end = min(360, degree_end)
        idx_begin = int(degree_begin/self.pivot_degrees_perframe)
        idx_end = int(degree_end/self.pivot_degrees_perframe)+2
        if(len(img.shape) == 3):
            dims = (1, 2, 3)
        else:
            dims = (1, 2)

        idx_begin -= 1  # conservative
        idx_begin = max(idx_begin, 0)
        if(idx_begin > idx_end):
            # The separation of self.frames comes of this array being a circular one (after 360=0 in degrees).
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
            j = 0
            weight = 0
        else:
            frames = self.frames[idx_begin:idx_end+1]
            distance = ((frames-img)**2).mean(axis=dims)
            i = np.argmin(distance)
            mindist = distance[i]
            #cv.imshow('best match', self.frames[i])
            #cv.imshow('current img', img)
            # print(mindist)
            # cv.waitKey(0)
            if(i >= 1 and i <= len(distance)-2):
                weights = 1/(distance[i-1:i+2]+1e-7)
                weights /= weights.sum()
                i = ([i-1, i, i+1]*weights).sum()
            i += idx_begin

        assert(mindist <= 50), "Pivot video does not seem to have something in common with the video file! (mindist=%.1f)" % mindist

        # weight /= 2 # This just gives more weight to i.
        return i*self.pivot_degrees_perframe, mindist

    def estimateSpeed(self, vcap):
        similarity_threshold = 20
        estimated_speeds = []
        dists = []
        dist = similarity_threshold
        while(dist >= similarity_threshold):
            _, img = vcap.read()
            if(img is None):
                raise AimEstimationException()
            angle1, dist = self._estimateAngle(img)
        while(img is not None):
            dist = similarity_threshold
            n_frames = 0
            while(dist >= similarity_threshold):
                _, img = vcap.read()
                if(img is None):
                    break
                n_frames += 1
                if(len(estimated_speeds) >= 2):
                    std_speed = np.std(estimated_speeds)
                    max_speed = np.max(estimated_speeds)
                    angle2, dist = self._estimateAngle(img, angle1, angle1+n_frames*(1.1*max_speed+std_speed))
                else:
                    angle2, dist = self._estimateAngle(img)
            if(img is None):
                break
            dists.append(dist)
            ang_diff = (angle2-angle1)/n_frames
            if(ang_diff < -90):
                ang_diff += 360
            elif(ang_diff > 90):
                ang_diff -= 360
            estimated_speeds.append(ang_diff)
            if(len(estimated_speeds) >= self.sample_size):
                break
            angle1 = angle2
        # for d, e in zip(dists, estimated_speeds):
        #    print(d, e*60)
        #print(np.std(estimated_speeds) * 60)
        return np.mean(estimated_speeds)
