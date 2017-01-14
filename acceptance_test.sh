#!/usr/bin/env bash

# shellcheck source=server.inc
. "$(dirname "${BASH_SOURCE[0]}")/server.inc"

# shellcheck disable=SC2119
IMAGETAG="$(getImageTag)"

SHUNIT=$(which shunit)



function testBuild() {
  build "$IMAGETAG"
  assertEquals "build failed" 0 $?
}



function testShip() {
  ship "$IMAGETAG"
  assertEquals "ship failed" 0 $?
}



function testRun() {
  run "$IMAGETAG"
  assertEquals "run failed" 0 $?
}



function testExternalPluginsFolderIsLink() {
  local imageName=$1
  containerName=${imageName//:/-}
  docker exec "$containerName" /bin/bash -c "[ -L /var/lib/go-server/plugins/external ]"
  assertEquals "/var/lib/go-server/plugins/external is not a symbolic link" 0 $?
}



function testExternalPluginsFolderIsNotEmpty() {
  local imageName=$1
  containerName=${imageName//:/-}
  count="$(docker exec "$containerName" ls /var/lib/go-server/plugins/external | wc -l)"
  assertNotEquals "/var/lib/go-server/plugins/external is empty" 0 "$count"
}



function testDestroy() {
  destroy "$IMAGETAG"
  assertEquals "destroy failed" 0 $?
}



# function oneTimeSetup()
# function oneTimeTearDown()
# function setup()
# function teardown()



# shellcheck source=/dev/null
. "$SHUNIT"

# TODO: tag as :accepted [ $? -eq 0 ] && docker tag $(getImageTag) && docker push
