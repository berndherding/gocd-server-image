#!/bin/bash

shopt -s extglob

[ ! -d "$(dirname "${BASH_SOURCE[0]}")/target" ] && mkdir "$(dirname "${BASH_SOURCE[0]}")/target"

ERROR_build_dockerBuild=1
ERROR_build_dockerRun=2
ERROR_build_dockerCommit=3
ERROR_build_dockerExec_waitForGoServer=5
ERROR_build_dockerRemove_container=6
ERROR_destroy_dockerRemove_container=7
ERROR_run_dockerExec_waitForGoServer=8
ERROR_run_dockerRun=9
ERROR_ship_dockerPush=10
ERROR_noDockerHubUsername=11
ERROR_noDockerHubPassword=12



function getRelease() {
  gpc="${GO_PIPELINE_COUNTER:-0}"
  rev=${GO_REVISION:-$(git rev-parse HEAD)}
  echo "$gpc-${rev:0:7}"
}



function getImageTag() {
  local imageName=${1:-gocd-server}

  imageTag=$imageName:${GO_PIPELINE_COUNTER:-0}

  if [ "$imageTag" != "$imageName:0" ] ; then

    gitHash=${GO_REVISION_GOCD_SERVER_IMAGE:0:7}
    gitHash=${gitHash:-$(git rev-parse --short HEAD)}

    imageTag="$imageTag-$gitHash"
  fi

  echo "$imageTag"
}



function build() {
  local imageName=$1

  [ -z "$DOCKER_HUB_USERNAME" ] && return $ERROR_noDockerHubUsername

  containerName=${imageName//:/-}
  imageName="$DOCKER_HUB_USERNAME/$imageName"

  echo "IMAGE_RELEASE=$(getRelease)" > docker/release

  docker build --no-cache -t "$imageName" "$(dirname "${BASH_SOURCE[0]}")"/docker | tee /tmp/server-build.out \
  || return $ERROR_build_dockerBuild

  # why run at all? -> the gocd server extracts itself on first run
  containerId=$(docker run --name "$containerName" -d "$imageName") \
  || return $ERROR_build_dockerRun

  containerId=${containerId:0:12}

  docker exec -t "$containerName" /bin/bash /usr/bin/wait_for_go_server.sh 300 \
  || return $ERROR_build_dockerExec_waitForGoServer

  docker exec -t "$containerName" /bin/bash /bin/bash -c 'rm /etc/go/*store*'

  docker commit "$containerId" "$imageName" || return $ERROR_build_dockerCommit

  docker rm -f "$containerName" || return $ERROR_build_dockerRemove_container

  dead="$(docker ps --filter status=dead --filter status=exited -aq)"
  [ -n "$dead" ]     && docker rm -v $dead

  untagged="$(docker images --no-trunc | grep '<none>' | awk '{ print $3 }')"
  [ -n "$untagged" ] && docker rmi $untagged

  dangling="(docker volume ls -qf dangling=true)" 
  [ -n "$dangling" ] && docker volume rm $dangling

  return 0
}



function ship() {
  local imageName=$1

  [ -z "$DOCKER_HUB_USERNAME" ] && return $ERROR_noDockerHubUsername
  [ -z "$DOCKER_HUB_PASSWORD" ] && return $ERROR_noDockerHubPassword

  docker login --username "$DOCKER_HUB_USERNAME" --password "$DOCKER_HUB_PASSWORD"

  docker push "$DOCKER_HUB_USERNAME/$imageName" || return $ERROR_ship_dockerPush

  echo "$DOCKER_HUB_USERNAME/$imageName" > "$(dirname "${BASH_SOURCE[0]}")"/target/gocdServerImageName

  # docker rmi "$(docker images "$imageName" -q)"  || return $ERROR_ship_dockerRemoveImage

  docker logout
}



function run() {
  local imageName=$1

  [ -z "$DOCKER_HUB_USERNAME" ] && return $ERROR_noDockerHubUsername

  containerName=${imageName//:/-}

  containerId=$(docker run --name "$containerName" -d \
    "$DOCKER_HUB_USERNAME/$imageName"
  ) || return $ERROR_run_dockerRun

  containerId=${containerId:0:12}

  docker exec -t "$containerName" /bin/bash /usr/bin/wait_for_go_server.sh 300 \
  || return $ERROR_run_dockerExec_waitForGoServer
}



function destroy() {
  local imageName=$1

  containerName=${imageName//:/-}

  docker rm -f "$containerName" \
  || return $ERROR_destroy_dockerRemove_container
}
