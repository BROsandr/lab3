#!/bin/bash

sudo ./light_detect -q 1000 ./light_data 2000 &
sudo ./sound_detect -q ./sound_data &

exec 4<>./sound_data
exec 5<>./light_data

read -u 4 line

while :
do
  while :
  do
    read -u 5 line
    threshold=5000
    if [ $line -ge $threshold ]
    then
      break
    fi
  done

  start=$SECONDS
  read -u 4 line
  read -u 4 line
  duration=$(( SECONDS - start ))
  soundspeed=343
  echo $(($duration * $soundspeed))
done
