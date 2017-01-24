# qrMqtt

![Alt text](https://github.com/meAbab/qrMqtt/blob/master/cover-qrMQTT.JPG?raw=true "Title")

# QR-Code scanned at door for remote MQTT-client

## After installing an OS on your SD card, install the necessary packages

1. OpenCV dev library [`sudo apt-get install libopencv-dev`]

2. The mqtt application [`sudo apt-get install mosquitto`]

3. The mosquitto client library [`sudo apt-get install mosquitto-clients`]

4. Mosquitto dev library [`sudo apt-get install libmosquitto-dev libmosquittopp-dev`

5. Zbar dev library [`sudo apt-get install libzbar0 libzbar-dev`] 


## Configuration

In the `main.cpp` file, update the provided configuration parameters:
`qr2sp = new qrMqtt("qr2sp", "pcktatDoor", "192.168.178.100", 1883);`

* `pcktatDoor` -> The MQTT topic name where you want your application to publish the QR scan results to
* `192.168.178.100` -> The hostname or IP address of the MQTT broker
* `1883` -> The port of the MQTT broker

## Compile & run

After the steps above, you can compile the code by running the command `make` in the root of the downloaded code.
When this step is completed, you can run the application by executing the command `./qrMqtt`

## Using the piCamera

If you are using the piCamera instead of a USB webcam, you should run this command to let OpenCV use the piCam:
`sudo modprobe bcm2835-v4l2`

## Running the program ##

![Alt text](https://github.com/meAbab/qrMqtt/blob/master/running_prog.png?raw=true "Title")


Program working video is here - https://www.youtube.com/watch?v=1rtJEr5uat0
