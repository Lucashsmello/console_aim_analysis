from aim_estimators import AimEstimator360, AimEstimatorPivot
import cv2 as cv
from gimx_api import initGIMXConnection, sendAimSpeedCommand
from time import sleep
import pandas as pd
import numpy as np
import os
from cvutils import CroppedVideoCapture


def removeBuffer(vcap):
    for _ in range(8):
        vcap.grab()


FPS = 60


def saveData(data, fpath):
    df = pd.DataFrame(data, columns=['x', 'y', 'speed'])
    df.to_csv(fpath, index=False)


def estimateSpeed(vcap, aim_estimator):
    fps = vcap.get(cv.CAP_PROP_FPS)
    assert(fps > 0), "Could not get FPS property from video."
    degrees_persec = aim_estimator.estimateSpeed(vcap) * fps  # AimEstimator360(cropx=cropx, cropy=cropy)
    return degrees_persec


def estimateWithCaptureCard_auto(vid, xlist, ylist, aim_estimator, cropx=None, cropy=None):
    vcap = CroppedVideoCapture(cropx, cropy, vid)
    vcap.set(cv.CAP_PROP_FRAME_WIDTH, 1280)
    vcap.set(cv.CAP_PROP_FRAME_HEIGHT, 720)
    vcap.set(cv.CAP_PROP_FPS, FPS)
    degrees_persec_list = []
    for x, y in zip(xlist, ylist):
        print("Estimating speed of (%d,%d)..." % (x, y))
        sendAimSpeedCommand(x, y)
        sleep(0.5)  # dealing with latency
        removeBuffer(vcap)
        degrees_persec = estimateSpeed(vcap, aim_estimator)
        degrees_persec_list.append(degrees_persec)
        print("Estimated speed is: %f degrees per second" % degrees_persec)
    vcap.release()
    return degrees_persec_list


def estimateVideoFile(video_files_list, aim_estimator, cropx=None, cropy=None):
    degrees_persec_list = []
    for vfile in video_files_list:
        assert(os.path.isfile(vfile)), "File not found: %s" % vfile
        vcap = CroppedVideoCapture(cropx, cropy, vfile)
        degrees_persec = estimateSpeed(vcap, aim_estimator)
        degrees_persec_list.append(degrees_persec)
        vcap.release()
        print("Estimated speed of %s is: %f degrees per second" % (vfile, degrees_persec))
    return degrees_persec_list


if __name__ == '__main__':
    import argparse
    parser = argparse.ArgumentParser()
    parser.add_argument('-i', '--input', type=str,
                        required=True, help="Video file or csv file with video files or video capture id.")
    parser.add_argument('-o', '--output', type=str,
                        default='data_estimated-speed.csv', help="output csv file")
    parser.add_argument('-x', '--speeds-x',  metavar='N', type=int, nargs='+',
                        required=False, help='X speeds from -128 to 127', choices=range(-128, 128))
    parser.add_argument('-y', '--speeds-y',  metavar='N', type=int, nargs='+',
                        required=False, help='Y speeds from -128 to 127', choices=range(-128, 128))
    parser.add_argument('--gimx-server', type=str,
                        help="format: IP_ADDRESS:PORT. This enables auto mode")
    parser.add_argument('--crop',  metavar='N', type=int, nargs=4,
                        default=[200, 320, 480, 600],
                        help='Crop video. Format: X1 X2 Y1 Y2. Example: --crop 200 320 480 600, will crop to box with top left coordinates (200,480) and bottom right (320,600).')
    parser.add_argument('--dont-crop', action='store_true')
    parser.add_argument('--pivot', type=str, help="Video file")
    args = parser.parse_args()
    assert(args.speeds_x is None or (len(args.speeds_x) == len(args.speeds_y)))

    if(args.dont_crop):
        cropx = cropy = None
    else:
        crop = args.crop
        cropx = (crop[0], crop[1])
        cropy = (crop[2], crop[3])
    if(args.pivot is not None):
        assert(os.path.isfile(args.pivot)), "File not found: %s" % args.pivot
        vcap = CroppedVideoCapture(cropx, cropy, args.pivot)
        degrees_perframe = AimEstimator360().estimateSpeed(vcap)
        vcap.release()
        vcap = CroppedVideoCapture(cropx, cropy, args.pivot)  # FIXME: Dont open twice
        aim_estimator = AimEstimatorPivot(vcap, degrees_perframe)
        vcap.release()
    else:
        aim_estimator = AimEstimator360()

    if(args.gimx_server is not None):
        gimx_ip, gimx_port = args.gimx_server.split(':')
        initGIMXConnection(gimx_ip, int(gimx_port))
    if(args.input[0].isdigit()):
        assert(len(args.speeds_x) >= 1), "No speed x or y provided! See options --speed-x and --speed-y"
        speeds_x = args.speeds_x
        speeds_y = args.speeds_y
        degrees_persec_list = estimateWithCaptureCard_auto(int(args.input), speeds_x, speeds_y,
                                                           aim_estimator, cropx, cropy)
    else:
        if(args.input.endswith(".csv")):
            data = pd.read_csv(args.input, skipinitialspace=True)
            assert((data.columns == ['file name', 'x', 'y']).all()), 'Header of csv file should be "file name,x,y"'
            video_files = data['file name']
            assert(data['x'].dtype <= data['y'].dtype <= np.int64)
            speeds_x = data['x']
            speeds_y = data['y']
        else:
            assert(len(args.speeds_x) >= 1), "No speed x or y provided! See options --speed-x and --speed-y"
            speeds_x = args.speeds_x
            speeds_y = args.speeds_y
            video_files = [args.input]
        degrees_persec_list = estimateVideoFile(video_files, aim_estimator, cropx, cropy)
    data = [(x, y, d) for x, y, d in zip(speeds_x, speeds_y, degrees_persec_list)]

    saveData(data, args.output)
    print("FINISHED!")
