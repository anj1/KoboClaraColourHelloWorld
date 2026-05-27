#!/bin/sh
set -eu

APPDIR="/mnt/onboard/.adds/myapp"
PIDFILE="$APPDIR/myapp.pid"

if [ -f "$PIDFILE" ]; then
  PID="$(cat "$PIDFILE")"
  kill "$PID" 2>/dev/null || true
  sleep 1
  kill -9 "$PID" 2>/dev/null || true
  rm -f "$PIDFILE"
fi
