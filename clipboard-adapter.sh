#!/bin/sh
# Clipboard adapter for rofi-emoji.
# Usage:
#   clipboard-adapter.sh copy
#     Set clipboard from STDIN
#   clipboard-adapter.sh insert
#     Try your best to insert this STDIN text into focused window
#
# Detects wayland and X and finds the appropriate tool for the current
# environment.
#
# If stderr is bound to /dev/null, then the caller won't display
# error messages. Do it manually in that case.

TOOL="xsel"

stderr_is_null() {
  test /proc/self/fd/2 -ef /dev/null
}

notify() {
  if hash notify-send 2>/dev/null; then
    notify-send rofi-emoji "$@"
  else
    echo "$@" >&2
  fi
}

show_error() {
  if stderr_is_null; then
    notify "$@"
  else
    echo "$@" >&2
  fi
}

handle_copy() {
  case "$TOOL" in
    xsel)
      exec xsel --clipboard --input
      ;;
    *)
      exit 1
  esac
}

handle_insert() {
  case "$TOOL" in
    xsel)
      # Set PRIMARY clipboard…
      xsel --primary --input
      # …and call Shift+Insert in focused window to paste it
      xdotool key Shift+Insert
      ;;
    *)
      exit 1
  esac
}

case "$1" in
  copy)
    handle_copy
    ;;
  insert)
    handle_insert
    ;;
  *)
    show_error "$0: Unknown command \"$1\""
    exit 1
esac
