#!/bin/bash
#
# A modular test that renders a scene and compares it with a golden image.

# --- begin runfiles.bash initialization v2 ---
# Copy-pasted from the Bazel Bash runfiles library v2.
set -uo pipefail; f=bazel_tools/tools/bash/runfiles/runfiles.bash
source "${RUNFILES_DIR:-/dev/null}/$f" 2>/dev/null || \
  source "$(grep -sm1 "^$f " "${RUNFILES_MANIFEST_FILE:-/dev/null}" | cut -f2- -d' ')" 2>/dev/null || \
  source "$0.runfiles/$f" 2>/dev/null || \
  source "$(grep -sm1 "^$f " "$0.runfiles_manifest" | cut -f2- -d' ')" 2>/dev/null || \
  source "$(grep -sm1 "^$f " "$0.exe.runfiles_manifest" | cut -f2- -d' ')" 2>/dev/null || \
  { echo>&2 "ERROR: cannot find $f"; exit 1; }; f=; set -e
# --- end runfiles.bash initialization v2 ---

# Uncomment this to get runfiles debug output.
# RUNFILES_LIB_DEBUG=1

if ! hash compare &> /dev/null; then
  echo "ERROR: ImageMagick compare command not found"
  exit 1
fi

SCENE="$1"
if [[ "$SCENE" == "" ]]; then
  echo "ERROR: Scene file not provided"
  exit 1
fi

MUON="$(rlocation __main__/muon/muon)"
if [[ ! -f $MUON ]]; then
  echo "ERROR: muon binary not found"
  exit 1
fi

SCENE_FILE="$(rlocation "__main__/test/$SCENE")"
if [[ ! -f $SCENE_FILE ]]; then
  echo "ERROR: Scene file __main__/test/$SCENE not found"
  exit 1
fi

GOLDEN_IMAGE="$(rlocation "__main__/test/testdata/${SCENE%.*}.png")"
if [[ ! -f $GOLDEN_IMAGE ]]; then
  echo "ERROR: Golden image __main__/test/testdata/${SCENE%.*}.png not found"
  exit 1
fi

# Renders an image via the given scene file and compares the output to the
# golden.
function test_diff {
  OUTPUT_FILE="$TEST_UNDECLARED_OUTPUTS_DIR/output.png"
  DIFF_FILE="$TEST_UNDECLARED_OUTPUTS_DIR/difference.png"

  # Render the image.
  $MUON --scene="$SCENE_FILE" --output="$OUTPUT_FILE"

  # Generate a diff image.
  abs_error=$(compare -metric AE "$OUTPUT_FILE" "$GOLDEN_IMAGE" "$DIFF_FILE" 2>&1 || :)
  echo "Absolute image error: $abs_error"

  if (( abs_error > 0 )); then
    echo "ERROR: Image had error"
    exit 1
  fi
}

test_diff
