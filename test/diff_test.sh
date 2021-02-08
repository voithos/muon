#!/bin/bash
#
# A modular test that renders a scene and compares it with a golden image.
# Since rendering is not always deterministic, this test allows specifying a
# "ground truth" image in addition to the golden, which is then compared to
# what the renderer generates by computing the normalized mean absolute error.
#
# Usage for determinstic tests:
#   diff_test.sh <test_scene> <golden_image>
#
# Usage for nondeterministic tests:
#   diff_test.sh <test_scene> <golden_image> <truth_image> <mae_tolerance>

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

MUON="$(rlocation __main__/muon/muon || :)"
if [[ ! -f $MUON ]]; then
  echo "ERROR: muon binary not found"
  exit 1
fi

# Unpack arguments.
SCENE="$1"
GOLDEN="$2"
TRUTH=""
TOLERANCE=""

if [[ $# -gt 2 ]]; then
  TRUTH="$3"
  TOLERANCE="$4"
fi

if [[ "$SCENE" == "" ]]; then
  echo "ERROR: Scene file not provided"
  exit 1
fi

SCENE_FILE="$(rlocation "__main__/$SCENE" || :)"
if [[ ! -f $SCENE_FILE ]]; then
  echo "ERROR: Scene file __main__/$SCENE not found"
  exit 1
fi

GOLDEN_IMAGE="$(rlocation "__main__/$GOLDEN" || :)"
if [[ ! -f $GOLDEN_IMAGE ]]; then
  echo "ERROR: Golden image __main__/$GOLDEN not found"
  exit 1
fi

# Renders an image via the given scene file and compares the output to the
# golden.
function test_diff_equality {
  OUTPUT_FILE="$TEST_UNDECLARED_OUTPUTS_DIR/output.png"
  DIFF_FILE="$TEST_UNDECLARED_OUTPUTS_DIR/diff.png"

  # Render the image.
  $MUON --scene="$SCENE_FILE" --output="$OUTPUT_FILE"

  # Generate a diff image.
  abs_error=$(compare -metric AE "$OUTPUT_FILE" "$GOLDEN_IMAGE" "$DIFF_FILE" 2>&1 || :)
  echo "Absolute image error: $abs_error"

  if (( $(bc -l <<< "$abs_error > 0") )); then
    echo "ERROR: Image had error"
    exit 1
  fi
}

# Renders an image via the given scene file and compares the output's mean
# absolute error to the ground truth file with that of the golden.
function test_diff_mae {

  OUTPUT_FILE="$TEST_UNDECLARED_OUTPUTS_DIR/output.png"
  DIFF_FILE="$TEST_UNDECLARED_OUTPUTS_DIR/diff.png"
  GOLDEN_DIFF_FILE="$TEST_UNDECLARED_OUTPUTS_DIR/golden_diff.png"

  TRUTH_IMAGE="$(rlocation "__main__/$TRUTH" || :)"
  if [[ ! -f $TRUTH_IMAGE ]]; then
    echo "ERROR: Ground truth image __main__/$TRUTH not found"
    exit 1
  fi

  # Render the image.
  $MUON --scene="$SCENE_FILE" --output="$OUTPUT_FILE"

  # Generate a diff image with the truth, and also generate a diff image with
  # the golden.
  mae_error=$(compare -metric MAE "$OUTPUT_FILE" "$TRUTH_IMAGE" "$DIFF_FILE" 2>&1 | cut -d "(" -f2 | cut -d ")" -f1 || :)
  golden_mae_error=$(compare -metric MAE "$GOLDEN_IMAGE" "$TRUTH_IMAGE" "$GOLDEN_DIFF_FILE" 2>&1 | cut -d "(" -f2 | cut -d ")" -f1 || :)
  echo "Normalized mean absolute error:            $mae_error"
  echo "Normalized mean absolute error of golden:  $golden_mae_error"

  if (( $(bc -l <<< "($golden_mae_error - $mae_error)^2 > $TOLERANCE^2") )); then
    echo "ERROR: Image exceeded error tolerance"
    exit 1
  fi
}

if [[ "$TRUTH" != "" ]]; then
  test_diff_mae
else
  test_diff_equality
fi
