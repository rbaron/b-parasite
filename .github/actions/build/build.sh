#!/bin/bash
set -eux -o pipefail

cd "${GITHUB_WORKSPACE}/${1}"

west build --build-dir ./build --pristine --board bparasite_nrf52840