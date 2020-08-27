import subprocess
import os
import socket
from enum import IntEnum
import struct
import sys

SOCKET = None


def initGIMXConnetion(server_ip, port=51914):
    global SOCKET
    SOCKET = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
    SOCKET.connect((server_ip, port))
    check_status()


class Ps4Controls(IntEnum):
    LEFT_STICK_X = 0
    LEFT_STICK_Y = 1
    RIGHT_STICK_X = 2
    RIGHT_STICK_Y = 3
    FINGER1_X = 4
    FINGER1_Y = 5
    FINGER2_X = 6
    FINGER2_Y = 7
    SHARE = 128
    OPTIONS = 129
    PS = 130
    UP = 131
    RIGHT = 132
    DOWN = 133
    LEFT = 134
    TRIANGLE = 135
    CIRCLE = 136
    CROSS = 137
    SQUARE = 138
    L1 = 139
    R1 = 140
    L2 = 141
    R2 = 142
    L3 = 143
    R3 = 144
    TOUCHPAD = 145
    FINGER1 = 146
    FINGER2 = 147


class XboxOneControls(IntEnum):
    LEFT_STICK_X = 0
    LEFT_STICK_Y = 1
    RIGHT_STICK_X = 2
    RIGHT_STICK_Y = 3
    VIEW = 128
    MENU = 129
    GUIDE = 130
    UP = 131
    RIGHT = 132
    DOWN = 133
    LEFT = 134
    Y = 135
    B = 136
    A = 137
    X = 138
    LB = 139
    RB = 140
    LT = 141
    RT = 142
    LS = 143
    RS = 144


class ButtonState(IntEnum):
    RELEASED = 0
    PRESSED = 255


def send_message(changes):
    global SOCKET
    packet = bytearray([0x01, len(changes)])  # type + axis count

    for axis, value in changes.items():
        # axis + value (network byte order)
        packet.extend([axis, (value & 0xff000000) >> 24, (value & 0xff0000)
                       >> 16, (value & 0xff00) >> 8, (value & 0xff)])
    SOCKET.send(packet)


def check_status():
    packet = bytearray([0x00, 0x00])
    SOCKET.send(packet)
    timeval = struct.pack('ll', 1, 0)  # seconds and microseconds
    SOCKET.setsockopt(socket.SOL_SOCKET, socket.SO_RCVTIMEO, timeval)
    try:
        data, (address, port) = SOCKET.recvfrom(2)
        response = bytearray(data)
        if (response[0] != 0x00):
            print("Invalid reply code: {0}".format(response[0]))
            return 1
    except socket.error as err:
        print(err)

    return 0


def sendAimSpeedCommand(x: int, y: int):
    assert(x <= 127 and x >= -128)
    assert(y <= 127 and y >= -128)
    changes = {Ps4Controls.RIGHT_STICK_X: x,
               Ps4Controls.RIGHT_STICK_Y: y}
    send_message(changes)
