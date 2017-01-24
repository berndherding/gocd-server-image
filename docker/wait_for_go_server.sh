#!/bin/bash

i=0
timeout=$1

while true ; do
  [ $i -ge $timeout ] && exit 1
  curl -s -o /dev/null http://localhost && exit 0
  i=$((i + 5))
  sleep 5
done
