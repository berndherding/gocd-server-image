#!/usr/bin/env bash

# shellcheck source=server.inc
. "$(dirname "${BASH_SOURCE[0]}")/server.inc"

# shellcheck disable=SC2119
destroy "${1:-$(getImageTag)}"
