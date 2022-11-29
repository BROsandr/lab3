all: light sound

light:
	g++ light_detect.c Adafruit_ADS1X15_RPi/Adafruit_ADS1015.cpp -o light_detect -lwiringPi

sound:
	gcc sound_detect.c -o sound_detect
