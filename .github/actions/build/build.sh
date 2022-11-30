#!/bin/bash
set -eux -o pipefail

SAMPLE_DIR=$1
BOARD=$2
CMAKE_EXTRA=$3
OUTPUT_BIN=$4

cd "${GITHUB_WORKSPACE}/${SAMPLE_DIR}"

west build --build-dir ./build --pristine --board "${BOARD}" -- $CMAKE_EXTRA

mv build/zephyr/zephyr.hex build/zephyr/"${OUTPUT_BIN}"