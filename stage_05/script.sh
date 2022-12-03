#!/bin/bash

trap 'sudo pkill light_detect;sudo pkill sound_detect;exit 0' INT

sudo ./light_detect -q 1000 ./light_data 200 &
sudo ./sound_detect -q ./sound_data &

exec 4<>./sound_data
exec 5<>./light_data

read -u 4 line

while :
do
  read -u 5 lihe

  start=$SECONDS
  read -u 4 line
  duration=$(( SECONDS - start ))
  soundspeed=343
  echo $(($duration * $soundspeed))
done

