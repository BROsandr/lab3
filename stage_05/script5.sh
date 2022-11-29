#!/bin/bash

trap 'sudo pkill light_detect;sudo pkill sound_detect;exit 0' INT

sudo ./light_detect -q 1000 ./light_data 200 &
sudo ./sound_detect -q ./sound_data &
sudo ./combiner &
