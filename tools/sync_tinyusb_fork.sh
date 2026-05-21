#!/usr/bin/env bash
# tools/sync_tinyusb_fork.sh, read-only verifier for cherry-picked MIDI 2.0.
#
# This branch (feat/midi2) cherry-picks the MIDI 2.0 device + host class
# drivers from hathach/tinyusb at the pinned SHA (PR #3571 merge commit).
# Only 4 files are vendored verbatim from upstream:
#
#   src/class/midi/midi2_device.c
#   src/class/midi/midi2_device.h
#   src/class/midi/midi2_host.c
#   src/class/midi/midi2_host.h
#
# Everything else in src/{class,device,host,portable,common,osal,...} is
# Adafruit-owned and MUST NOT be touched by this script (that policy is
# why the previous wholesale-replace implementation was retired: it
# nuked Adafruit-local patches like the ESP32 dispatch branch in
# tusb_option.h).
#
# What this script does:
#   1. Clones hathach/tinyusb at the pinned SHA into a temp dir.
#   2. Compares each of the 4 vendored files to upstream byte-for-byte.
#   3. Prints a unified diff for any file that drifted and exits 1.
#   4. Exits 0 if everything matches upstream.
#
# When upstream advances and we want to bump:
#   - Bump SHA below.
#   - cp the 4 files manually from a fresh hathach clone.
#   - Re-run this script to confirm match.
#   - Commit as "chore(vendor): bump TinyUSB MIDI 2.0 cherry-pick to <SHA>".
set -euo pipefail

REPO_URL="https://github.com/hathach/tinyusb.git"
SHA="4c87db341e4af7d53ce9cbbdf693593a520dc538"   # PR #3571 merge commit (2026-05-19)

FILES=(
    "src/class/midi/midi2_device.c"
    "src/class/midi/midi2_device.h"
    "src/class/midi/midi2_host.c"
    "src/class/midi/midi2_host.h"
)

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
FORK_ROOT="$(cd "${SCRIPT_DIR}/.." && pwd)"
TUSB_TMP="$(mktemp -d)"
trap 'rm -rf "${TUSB_TMP}"' EXIT

echo "[verify] cloning hathach/tinyusb @ ${SHA}..."
git clone --filter=tree:0 -q "${REPO_URL}" "${TUSB_TMP}/tinyusb"
git -C "${TUSB_TMP}/tinyusb" checkout -q "${SHA}"

drift=0
for f in "${FILES[@]}"; do
    if [[ ! -f "${FORK_ROOT}/${f}" ]]; then
        echo "[FAIL] missing: ${f}"
        drift=1
        continue
    fi
    if ! diff -q "${TUSB_TMP}/tinyusb/${f}" "${FORK_ROOT}/${f}" >/dev/null 2>&1; then
        echo "[FAIL] drift: ${f}"
        diff -u "${TUSB_TMP}/tinyusb/${f}" "${FORK_ROOT}/${f}" | head -40
        drift=1
    else
        echo "[ok]   ${f}"
    fi
done

if (( drift )); then
    echo ""
    echo "Vendored MIDI 2.0 files drifted from hathach/tinyusb @ ${SHA}."
    echo "To restore upstream parity, manually cp the files from a hathach clone"
    echo "at that SHA and re-run this script."
    exit 1
fi

echo ""
echo "[ok] all 4 vendored MIDI 2.0 files match hathach/tinyusb @ ${SHA}"
