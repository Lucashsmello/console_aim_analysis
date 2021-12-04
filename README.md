# Console aim analysis
A software for collecting data with respect to angular speed for each joystick input of PS4 fps games.
It uses a video file (or video capture card), GIMX and a simple image comparison algorithm.


## Requirements
- Python 3.8 or newer
- `pip install -r requirements.txt`
- gimx installed

## How to use
### Collect data
You should first collect your data (video files).
1. Run gimx in server mode. `gimx --src 127.0.0.1:51914 -p /dev/ttyUSB0`. I am assuming your joystick is connected to usb. You can specify a configuration file (--config), if you want. See Section [Controller Options](https://gimx.fr/wiki/index.php?title=Command_line) of gimx official wiki for more details.
2. Disable motion blur, fog, UI, and other stuff (if possible) that may add random motion to your video even when you are not moving.
3. Find a place in your game where the image is mostly static (when your are not moving) and there is no object moving. For example, if its raining or there is a fog, avoid them by going inside a house. Just a small portion of your image needs to be static.
4. Look all the way down to the ground (recommended) or all the way up to the ceiling. 
5. Run gimx command line with your desired X_SPEED (example: 15): `gimx --event "rel_axis_2(X_VALUE)" --event "rel_axis_3(0)" -d 127.0.0.1:51914`. Your character should start rotating now. For X_SPEED, I suggest a lot of low values right above the dead zone and some mid/high values. For example, assuming deadzone is 13 and we are using playstation/ds4, we can test 14,15,16,17,18,19,110, and 127.
6. Record the videos, one for each desired X_SPEED, and write their respective X_SPEED somewhere. I suggest to write the X_SPPED in the name of the video file.
7. **IMPORTANT:** The recorded video should last at least a full rotation (>=360 degrees) and less than two rotations (<720 degrees). I recommend anything above 370 degrees and less than 720 degrees. For very high speeds, I recommend more than 420 degrees.
8. Make a csv file with the meta info of your collected data in this format (see [this example](https://drive.google.com/drive/u/0/folders/1FuS7wlk8MzDVKK0Us2UqGMjYV5WFslC3)): 
```
file name, x, y
data/video1.avi,13,0
data/video2.avi,14,0
data/video3.avi,15,0
data/video4.avi,19,0
```
The first column should be the path to your video file.

### Measure angular speed automatically
With your data collected, run
```
python main.py --input metainfo.csv --crop 100 200 100 200 -o measured_speeds.csv
```
The crop option, crops your video at position X1 X2 Y1 Y2 (in pixels). Find a nice small crop of your video in which there are no objects moving. To test possible good crops, the above command shows the first cropped image of your video. You can also crop your videos with your own preffered tool and just pass option `--dont-crop` to main.py.
After running this command, a file of name measured_speeds.csv will appear with your measured speeds.

As an example, download all files from [example data](https://drive.google.com/drive/folders/1FuS7wlk8MzDVKK0Us2UqGMjYV5WFslC3?usp=sharing), save them in a folder named "example" in your working directory and run the following command:
```
python main.py --input example/data.csv --dont-crop -o measured_speeds.csv 
```
The output at the stdout should be:
```
Estimated speed of example/example1.avi is: 4.750385 degrees per second
Estimated speed of example/example2.avi is: 20.454545 degrees per second
Estimated speed of example/example3.avi is: 39.416058 degrees per second
Estimated speed of example/example4.avi is: 55.102041 degrees per second
```
All the examples files are already cropped. The original video files are found [here](https://drive.google.com/drive/folders/1A5IAhOi6PSwN49-yJ0K6AAsIlY1M1sT4?usp=sharing).

### Regression
Run
```
python regression.py --input measured_speeds.csv -o predicted_values.csv
```
This command will fit the best polynomial (up to degree 3) to the data and save predicted values for all possible axis values. The regression is not fitted on the 2-dimensional original data, but on the Euclidean norm ||(x+1,y+1)||<sub>2</sub>, that is, the square root of (x+1)<sup>2</sup>+(y+1)<sup>2</sup>.


### Command line examples:
Estimate speed of a single pre-recorded video file.
```
python main.py -i video.avi
```

Estimate speed of (x,y)=(18,0) using a video capture card of id 0:
```
python main.py -i 0 -x 18 -y 0 --gimx-server 192.168.50.251:51914
```

Estimate speed of all lines in "example/data.csv" with no automatic cropping, and output results to "measured_speeds.csv"
```
python main.py --input example/data.csv --dont-crop -o measured_speeds.csv 
```
