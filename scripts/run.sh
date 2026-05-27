#!/bin/sh
set -eu

APPDIR="/mnt/onboard/.adds/myapp"
LOGDIR="$APPDIR/logs"
PIDFILE="$APPDIR/myapp.pid"

mkdir -p "$LOGDIR"

# Avoid duplicate instances.
if [ -f "$PIDFILE" ] && kill -0 "$(cat "$PIDFILE")" 2>/dev/null; then
  echo "Already running: $(cat "$PIDFILE")" >> "$LOGDIR/myapp.log"
  exit 0
fi

# Keep libraries local to the app.
export LD_LIBRARY_PATH="$APPDIR/lib:/usr/local/Kobo:/usr/local/Qt-5.2.1-arm/lib:${LD_LIBRARY_PATH:-}"

# Optional: let app override this after discovery.
export KOBO_TOUCH_DEVICE="${KOBO_TOUCH_DEVICE:-}"

cd "$APPDIR"

(
  echo "==== start $(date) ===="
  ./myapp
  echo "==== exit $? $(date) ===="
) >> "$LOGDIR/myapp.log" 2>&1 &

echo "$!" > "$PIDFILE"
