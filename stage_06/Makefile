all: light sound combiner run

light: .PHONY
	g++ light_detect.c Adafruit_ADS1X15_RPi/Adafruit_ADS1015.cpp -o light_detect -lwiringPi -Wall

sound: .PHONY
	gcc sound_detect.c -o sound_detect -Wall

combiner: .PHONY
	gcc combiner.c -o combiner -Wall -pthread

run: .PHONY
	./script5.sh

.PHONY:
