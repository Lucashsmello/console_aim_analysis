import cv2 as cv
from gimx_api import initGIMXConnetion, sendAimSpeedCommand


def imageSimilarity(img1, img2):
    return ((img1-img2)**2).mean()


class AimEstimationException(Exception):
    def __init__(self):
        self.message = "Could not estimate speed!"
        super().__init__(self.message)


class AimEstimator360:
    def __init__(self, similarity_threshold=8, cropy=(200, 320), cropx=(480, 600)):
        self.similarity_threshold = similarity_threshold
        if(cropx is None):
            self.cropx = self.cropy = None
        else:
            self.cropx = slice(*cropx)
            self.cropy = slice(*cropy)

    def estimateSpeed(self, vcap: cv.VideoCapture):
        """
        returns: degrees per frame
        """
        _, first_img = vcap.read()
        if(self.cropx is not None):
            first_img = first_img[self.cropy, self.cropx]
        # cv.imshow('first img', first_img)
        # cv.waitKey(1)
        is_starting = True
        m = 10000
        n_frames = 0
        while(vcap.isOpened()):
            _, img = vcap.read()
            if(img is None):
                break
            if(self.cropx is not None):
                img = img[self.cropy, self.cropx]
            n_frames += 1
            similarity = imageSimilarity(first_img, img)
            if(similarity > 2*self.similarity_threshold+1):
                is_starting = False
            # cv.imshow('frame', img)

            # key = cv.waitKey(1) & 0xFF
            # if(key == ord('q')):
            #     break
            if(not is_starting):
                if(similarity < self.similarity_threshold):
                    # cv.waitKey(0)
                    return 360/n_frames
        raise AimEstimationException()
