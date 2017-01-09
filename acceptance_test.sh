#!/usr/bin/env bash

# shellcheck source=../server.inc
. "$(dirname "${BASH_SOURCE[0]}")/../server.inc"

GIT_HASH=${GO_REVISION_FNS_GOCD_SERVER_DOCKER:0:7}
GIT_HASH=${GIT_HASH:-$(git rev-parse --short HEAD)}

IMAGETAG=go-server:${GO_PIPELINE_COUNTER:-0}-$GIT_HASH

SHUNIT=$(which shunit)



function testBuildServer() {
  buildServer "$IMAGETAG"
  assertEquals "build server failed" 0 $?
}



function testShipServer() {
  shipServer "$IMAGETAG"
  assertEquals "ship server failed" 0 $?
}



function testRunServer() {
  runServer "$IMAGETAG"
  assertEquals "run server failed" 0 $?
}



# function oneTimeSetup()
# function oneTimeTearDown()
# function setup()
# function teardown()



# shellcheck source=/usr/local/bin/shunit
. "$SHUNIT"
