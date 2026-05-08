#!/usr/bin/env bash
set -e

VIDEO_DIR="/home/ppp/aktuellt-alarmclock/videos"
LOG_FILE="/home/ppp/aktuellt-alarmclock/logs/play.log"

mkdir -p "$(dirname "$LOG_FILE")"

pkill -f "mpv" || true

LATEST_FILE=$(find "$VIDEO_DIR" -type f | sort | tail -n 1)

if [ -z "$LATEST_FILE" ]; then
  echo "$(date): No video file found" >> "$LOG_FILE"
  exit 1
fi

echo "$(date): Playing $LATEST_FILE" >> "$LOG_FILE"
mpv --fs --no-terminal "$LATEST_FILE" >> "$LOG_FILE" 2>&1
