#!/bin/bash

shopt -s extglob

[ ! -e "$(dirname "${BASH_SOURCE[0]}")/inc"    ] && ln -s ../gocd-base-ami/inc    "$(dirname "${BASH_SOURCE[0]}")/inc"
[ ! -e "$(dirname "${BASH_SOURCE[0]}")/target" ] && ln -s ../gocd-base-ami/target "$(dirname "${BASH_SOURCE[0]}")/target"

# shellcheck source=/dev/null
. "$(dirname "${BASH_SOURCE[0]}")/inc/commons.inc"

# shellcheck source=/dev/null
. "$(dirname "${BASH_SOURCE[0]}")/inc/keys.inc"



#
# TODO: check for running docker daemon, e.g. by calling docker version
#



function buildServer() {
  local imageName=$1

  containerName=${imageName//:/-}
  imageName="$(getDockerHubUsername)/$imageName"

  whoami
  groups
  ls -la /var/run/docker.sock
  tail -n 2 /etc/group

  docker build --no-cache -t "$imageName" "$(dirname "${BASH_SOURCE[0]}")"/docker | tee /tmp/server-build.out || return 1

  # why run at all? -> the gocd server extracts itself on first run
  containerId=$(docker run --name "$containerName" -e RUN_MODE=init -d "$imageName") || return 2

  containerId=${containerId:0:12}

  docker exec -t "$containerName" /bin/bash /usr/bin/wait_for_go_server.sh 600 || return 3

  docker exec -t "$containerName" /bin/bash -c "
    mv /gocd-plugins/* /var/lib/go-server/plugins/external
    chown go.go /var/lib/go-server/plugins/external/*
  " || return 4

  docker commit -m "go-server with github KnownHost and some plugins" "$containerId" "$imageName" || return 5

  docker rm -f "$containerName" || return 6

  return 0
}



function shipServer() {
  local imageName=$1

  docker login --username "$(getDockerHubUsername)" --password "$(getDockerHubPassword)"

  docker push "$(getDockerHubUsername)/$imageName" || return 7

  echo "$(getDockerHubUsername)/$imageName" > "$(dirname "${BASH_SOURCE[0]}")"/target/gocdServerImageName

  # docker rmi "$(docker images "$imageName" -q)"  || return 8

  docker logout
}



function destroyServer() {
  local imageName=$1

  containerName=${imageName//:/-}

  docker rm -f "$containerName" || return 9
}
