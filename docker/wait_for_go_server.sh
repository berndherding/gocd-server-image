#!/bin/bash

i=0
timeout=$1

while true ; do
  echo -e -n "\r$i"
  if [ $i -eq $timeout ] ; then
    exit 1
  fi
  grep 'cachedGoConfigRefreshExecutorThread.*CachedGoConfig.*Finished notifying all listeners' /var/log/go-server/go-server.log &> /dev/null
  status=$?    
  if [ $status -eq 0 -a $i -ge 10 ] ; then   # wait a min 10s
    exit 0
  fi
  sleep 1
  i=$((i + 1))
done
