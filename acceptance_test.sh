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
