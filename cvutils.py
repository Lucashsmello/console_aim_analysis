import cv2 as cv


class CroppedVideoCapture:
    def __init__(self, cropx, cropy, *args):
        self.vcap = cv.VideoCapture(*args)
        if(cropx is None):
            self.cropx = self.cropy = None
        else:
            assert(cropx[1]-cropx[0] < self.get(cv.CAP_PROP_FRAME_WIDTH) and
                   cropy[1]-cropy[0] < self.get(cv.CAP_PROP_FRAME_HEIGHT)), "Crop is bigger than the video resolution!"
            self.cropx = slice(*cropx)
            self.cropy = slice(*cropy)

    def read(self):
        retval, img = self.vcap.read()
        if(self.cropx is None):
            return retval, img
        if(img is None):
            return retval, None
        return retval, img[self.cropy, self.cropx]

    def release(self):
        self.vcap.release()

    def get(self, *args):
        return self.vcap.get(*args)
