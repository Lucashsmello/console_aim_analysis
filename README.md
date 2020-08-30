# Console aim analysis
A software for collecting data with respect to angular speed for each joystick input of PS4 fps games.
It uses a video file (or video capture card), GIMX and a simple image comparison algorithm.

This is a rework from a c++ code to python code. The intention is to be easy to install, extend/improve, add new features and easy to fix bugs.

This is a work in progress...

## Requirements
TODO

## Installing
`pip install -r requirements.txt`
TODO

## How to use
### Command line examples:
Estimate speed of a pre-recorded video file where axis speed is set to (x,y)=(18,0):
```bash
python main.py -i video.avi -x 18 -y 0
```
Estimate speed of (x,y)=(18,0) using a video capture card of id 0:
```bash
python main.py -i 0 -x 18 -y 0 --gimx-server 10.0.0.102:51914
```
TODO
