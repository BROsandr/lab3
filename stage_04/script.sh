#!/bin/bash

exec 4<>p1
exec 5<>p2

while :
do
  while :
  do
    read -u 5 line
    threshold=10000
    if [ $line -ge $threshold ]
    then
      break
    fi
  done

  start=$SECONDS
  read -u 4 line
  duration=$(( SECONDS - start ))
  soundspeed=343
  echo $(($duration * $soundspeed))
done
