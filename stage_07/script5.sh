#!/bin/bash

trap 'echo sending;sudo pkill -SIGINT light_detect;sudo pkill -SIGINT sound_detect;exit 0' INT

sudo ./combiner &
sudo ./light_detect -q 1000 ./light_data 10000 &
sudo ./sound_detect -q ./sound_data &

while true; do
  :
done
