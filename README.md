# qrMqtt

# QR-Code scanned at door for remote MQTT-client

after prepare SD-card with OS, you need to install -

1. OpenCV dev library [apt-get libopencv-dev]

2. The mqtt application [mosquitto-1.4.1]

3. Mosquitto dev library [apt-get libmosquitto-dev]

4. Zbar dev library [libzbar0 & libzbar-dev] 


@:~/qrmosq# mosquitto -c /etc/mosquitto/mosquitto.conf

@:~/qrmosq# ./qrMqtt
##-Connected with Broker-## 
Main Data ->AMAZON
MOUSE
30-APR-2016
## - Message published successfully


@:~# mosquitto_sub -h 192.168.178.100 -p 1883 -t 'pcktatDoor'

