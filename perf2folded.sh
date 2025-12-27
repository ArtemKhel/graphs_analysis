#!/usr/bin/env bash
set -euo pipefail

# Usage: ./perf2folded.sh [perf.data] [out.folded]
PERF_DATA="${1:-perf.data}"
OUT_FOLDED="${2:-out.folded}"

# Ensure perf.data is readable by the invoking user
if [[ -e "$PERF_DATA" ]]; then
    sudo chown "$(id -u):$(id -g)" "$PERF_DATA"
fi

# Find stackcollapse-perf.pl
if [[ -z "${FLAMEGRAPH_DIR:-}" ]]; then
    if [[ -d "./FlameGraph" ]]; then
        FLAMEGRAPH_DIR="./FlameGraph"
    elif command -v stackcollapse-perf.pl >/dev/null; then
        FLAMEGRAPH_DIR=""
    else
        echo "Error: stackcollapse-perf.pl not found."
        exit 1
    fi
fi

STACKCOLLAPSE="${FLAMEGRAPH_DIR:+$FLAMEGRAPH_DIR/}stackcollapse-perf.pl"

TMP="$(mktemp -d)"
trap 'rm -rf "$TMP"' EXIT

# Convert perf.data → folded
perf script -i "$PERF_DATA" > "$TMP/out.perf"
"$STACKCOLLAPSE" "$TMP/out.perf" > "$OUT_FOLDED"

echo "✓ Folded stacks written to $OUT_FOLDED"
