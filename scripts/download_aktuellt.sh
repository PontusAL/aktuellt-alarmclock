#!/usr/bin/env bash

set -euo pipefail

PROJECT_DIR="/home/ppp/aktuellt-alarmclock"
VIDEO_DIR="$PROJECT_DIR/videos"
LOG_DIR="$PROJECT_DIR/logs"
LOG_FILE="$LOG_DIR/download.log"
RESOLVER="$PROJECT_DIR/scripts/resolve_latest_aktuellt"

mkdir -p "$VIDEO_DIR"
mkdir -p "$LOG_DIR"

echo "[$(date --iso-8601=seconds)] Resolving latest Aktuellt URL..." >> "$LOG_FILE"

EPISODE_URL="$("$RESOLVER")"

echo "[$(date --iso-8601=seconds)] Found episode URL: $EPISODE_URL" >> "$LOG_FILE"

svtplay-dl \
  --output-format mp4 \
  --output "$VIDEO_DIR" \
  "$EPISODE_URL" >> "$LOG_FILE" 2>&1

echo "[$(date --iso-8601=seconds)] Download finished." >> "$LOG_FILE"

