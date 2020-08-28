from aim_estimators import AimEstimator360
import cv2 as cv
from gimx_api import initGIMXConnetion, sendAimSpeedCommand
from time import sleep
import pandas as pd


def removeBuffer(vcap):
    for _ in range(8):
        vcap.grab()


FPS = 60


def saveData(data, fpath):
    df = pd.DataFrame(data, columns=['x', 'y', 'speed'])
    df.to_csv(fpath, index=False)


def estimateWithCaptureCard_auto(vcap, xlist, ylist):
    aim_estimator = AimEstimator360()
    data = []
    for x, y in zip(xlist, ylist):
        print("Estimating speed of (%d,%d)..." % (x, y))
        removeBuffer(vcap)
        sendAimSpeedCommand(x, y)
        sleep(0.4)  # dealing with latency
        degrees_persec = aim_estimator.estimateSpeed(vcap) * FPS
        data.append([x, y, degrees_persec])
        print("Estimated speed is: %f degrees per seconds" % degrees_persec)
    return data


def estimateVideoFile(vcap, x, y):
    fps = vcap.get(cv.CAP_PROP_FPS)
    assert(fps > 0), "Could not get FPS property from video file."
    degrees_persec = AimEstimator360().estimateSpeed(vcap) * fps
    print("Estimated speed of (%d,%d) is: %f degrees per seconds" % (x, y, degrees_persec))
    return degrees_persec


if __name__ == '__main__':
    import argparse
    parser = argparse.ArgumentParser()
    parser.add_argument('-i', '--input', type=str, required=True, help="Video file or id")
    parser.add_argument('-x', '--speeds-x',  metavar='N', type=int, nargs='+',
                        required=True, help='X speeds from -128 to 127', choices=range(-128, 128))
    parser.add_argument('-y', '--speeds-y',  metavar='N', type=int, nargs='+',
                        required=True, help='Y speeds from -128 to 127', choices=range(-128, 128))
    parser.add_argument('--gimx-server', type=str,
                        help="format: IP_ADDRESS:PORT. This enables auto mode")
    # parser.add_argument('--method', type=str, choices=['360'], default='360')
    args = parser.parse_args()
    assert(len(args.speeds_x) == len(args.speeds_y))

    if(args.gimx_server is not None):
        gimx_ip, gimx_port = args.gimx_server.split(':')
        initGIMXConnetion(gimx_ip, int(gimx_port))
    if(args.input.isdigit()):
        vcap = cv.VideoCapture(int(args.input))
        vcap.set(cv.CAP_PROP_FRAME_WIDTH, 1280)
        vcap.set(cv.CAP_PROP_FRAME_HEIGHT, 720)
        vcap.set(cv.CAP_PROP_FPS, FPS)
        data = estimateWithCaptureCard_auto(vcap, args.speeds_x, args.speeds_y)
        saveData(data, 'data.csv')
    else:
        assert(len(args.speeds_x) == 1), "Currently this implementation is only accepting one speed value."
        x = args.speeds_x[0]
        y = args.speeds_y[0]
        vcap = cv.VideoCapture(args.input)
        degrees_persec = estimateVideoFile(vcap, x, y)
        print("%d,%d,%f" % (x, y, degrees_persec))
    vcap.release()
