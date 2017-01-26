#!/bin/bash

i=0
timeout=$1

while true ; do
  echo -e -n "\r$i"
  [ $i -ge $timeout ] && exit 1
  curl -s -o /dev/null http://localhost:8153 && exit 0
  i=$((i + 5))
  sleep 5
done
