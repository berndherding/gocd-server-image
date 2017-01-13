#!/bin/bash

shopt -s extglob

[ ! -e "$(dirname "${BASH_SOURCE[0]}")/inc"    ] && ln -s ../gocd-base-ami/inc    "$(dirname "${BASH_SOURCE[0]}")/inc"
[ ! -e "$(dirname "${BASH_SOURCE[0]}")/target" ] && ln -s ../gocd-base-ami/target "$(dirname "${BASH_SOURCE[0]}")/target"

# shellcheck source=/dev/null
. "$(dirname "${BASH_SOURCE[0]}")/inc/commons.inc"

# shellcheck source=/dev/null
. "$(dirname "${BASH_SOURCE[0]}")/inc/keys.inc"

ERROR_build_dockerBuild=1
ERROR_build_dockerRun=2
ERROR_build_dockerExec_linkPlugins=3
ERROR_build_dockerExec_waitForGoServer=4
ERROR_build_dockerCommit=5
ERROR_build_dockerRemove_container=6
ERROR_destroy_dockerRemove_container=7
ERROR_run_dockerExec_waitForGoServer=8
ERROR_run_dockerRun=9
ERROR_ship_dockerPush=10



#
# TODO: check for running docker daemon, e.g. by calling docker version
#



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

  containerName=${imageName//:/-}
  imageName="$(getDockerHubUsername)/$imageName"

  docker build --no-cache -t "$imageName" "$(dirname "${BASH_SOURCE[0]}")"/docker | tee /tmp/server-build.out \
  || return $ERROR_build_dockerBuild

  # why run at all? -> the gocd server extracts itself on first run
  containerId=$(docker run --name "$containerName" -d "$imageName") \
  || return $ERROR_build_dockerRun

  containerId=${containerId:0:12}

  docker exec -t "$containerName" /bin/bash /usr/bin/wait_for_go_server.sh 300 \
  || return $ERROR_build_dockerExec_waitForGoServer

  docker exec -t "$containerName" /bin/bash -c "
    rm -fr /var/lib/go-server/plugins/external
    ln -sf /plugins /var/lib/go-server/plugins/external
  " || return $ERROR_build_dockerExec_linkPlugins

  docker commit "$containerId" "$imageName" || return $ERROR_build_dockerCommit

  docker rm -f "$containerName" || return $ERROR_build_dockerRemove_container

# TODO CLEANUP:
# docker ps -a  | grep Exited | awk '{print $1}' | xargs docker rm -f
# docker images | grep none   | awk '{print $3}' | xargs docker rmi
# docker volume ls -qf dangling=true | xargs -r docker volume rm

  return 0
}



function ship() {
  local imageName=$1

  docker login --username "$(getDockerHubUsername)" --password "$(getDockerHubPassword)"

  docker push "$(getDockerHubUsername)/$imageName" || return $ERROR_ship_dockerPush

  echo "$(getDockerHubUsername)/$imageName" > "$(dirname "${BASH_SOURCE[0]}")"/target/gocdServerImageName

  # docker rmi "$(docker images "$imageName" -q)"  || return $ERROR_ship_dockerRemoveImage

  docker logout
}



function run() {
  local imageName=$1

  containerName=${imageName//:/-}

  containerId=$(docker run --name "$containerName" -d \
    "$(getDockerHubUsername)/$imageName"
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
