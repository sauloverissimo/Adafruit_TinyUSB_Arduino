#!/usr/bin/env bash
# tools/sync_tinyusb_fork.sh, idempotent TinyUSB tree sync.
#
# Replaces the vendored TinyUSB tree under
# src/{class,device,host,portable,common,osal,tusb.{c,h},tusb_option.h}
# with the content of sauloverissimo/tinyusb at the pinned SHA
# (TinyUSB PR #3571 head).
#
# NEVER touches src/Adafruit_TinyUSB.h, src/arduino/, src/tusb_config.h,
# library.{properties,json}, examples/, .github/, README.md, changelog.md.
#
# Re-runnable: full clone every run, atomic replace at the end. Bump
# the SHA in this script when the PR #3571 fork advances.
set -euo pipefail

REPO_URL="https://github.com/sauloverissimo/tinyusb.git"
SHA="31d730d8bb0b5c0832c5490378a2a2dd60ab72aa"   # PR #3571 head

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
FORK_ROOT="$(cd "${SCRIPT_DIR}/.." && pwd)"
TUSB_TMP="$(mktemp -d)"
trap 'rm -rf "${TUSB_TMP}"' EXIT

git clone --filter=tree:0 "${REPO_URL}" "${TUSB_TMP}/tinyusb"
git -C "${TUSB_TMP}/tinyusb" checkout -q "${SHA}"

# Wholesale replace the vendored TinyUSB tree. The list MUST track
# every top-level src/ directory in the upstream TinyUSB repo. Missing
# any directory leaves dangling #include lines in the .c files we DO
# copy (e.g. src/portable/st/typec/typec_stm32.c includes typec/tcd.h
# which lives at src/typec/tcd.h).
for d in class device host portable common osal typec; do
    rm -rf "${FORK_ROOT}/src/${d}"
    if [[ -d "${TUSB_TMP}/tinyusb/src/${d}" ]]; then
        cp -r "${TUSB_TMP}/tinyusb/src/${d}" "${FORK_ROOT}/src/${d}"
    fi
done
cp "${TUSB_TMP}/tinyusb/src/tusb.c"        "${FORK_ROOT}/src/tusb.c"
cp "${TUSB_TMP}/tinyusb/src/tusb.h"        "${FORK_ROOT}/src/tusb.h"
cp "${TUSB_TMP}/tinyusb/src/tusb_option.h" "${FORK_ROOT}/src/tusb_option.h"

echo "[sync] TinyUSB fork synced to ${SHA}"
