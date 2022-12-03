#!/bin/bash
set -eux -o pipefail

SAMPLE_DIR=$1
BOARD=$2
REVISION=$3
CMAKE_EXTRA=$4
OUTPUT_BIN=$5

cd "${GITHUB_WORKSPACE}/${SAMPLE_DIR}"

west build --build-dir ./build --pristine --board "${BOARD}@${REVISION}" -- $CMAKE_EXTRA

mv build/zephyr/zephyr.hex build/zephyr/"${OUTPUT_BIN}"