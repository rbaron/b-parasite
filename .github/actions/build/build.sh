#!/bin/bash
set -eux -o pipefail

export SDK_ROOT=/opt/nRF5_SDK
export GNU_INSTALL_ROOT=/usr/bin/

cd "$GITHUB_WORKSPACE/code/b-parasite"
make clean
make lint
make