#!/bin/sh
# Clipboard adapter for rofi-emoji.
#
# Usage:
#   clipboard-adapter.sh copy "the text"
#     Set clipboard to "the text"
#
# Detects Wayland and X and finds the appropriate tool for the current
# environment.
#
# If stderr is bound to /dev/null, then the caller won't display
# error messages. Do it manually in that case, for example by sending a
# notification.
#
# When rofi is run from a terminal, the output both on stderr and stdout should
# be visible to the user and makes it possible to do some debugging.

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
  case "$1" in
    xsel)
      xsel --clipboard --input
      ;;
    xclip)
      xclip -selection clipboard -in
      ;;
    wl-copy)
      wl-copy
      ;;
    *)
      show_error "$1 has no implementation for copying yet"
      exit 1
  esac
}

# Print out the first argument and return true if that argument is an installed
# command. Prints nothing and returns false if the argument is not an installed
# command.
try_tool() {
  if hash "$1" 2>/dev/null; then
    echo "$1"
    return 0
  else
    return 1
  fi
}

# Find the best clipboard tool to use.
determine_tool() {
  if [ "$XDG_SESSION_TYPE" = wayland ] || [ -n "$WAYLAND_DISPLAY" ]; then
    try_tool wl-copy ||
      return 1
  else
    try_tool xsel ||
      try_tool xclip ||
      return 1
  fi
}

tool=$(determine_tool)
if [ -z "$tool" ]; then
  show_error "Could not find a supported clipboard tool installed. Install xsel."
  exit 1
fi

case "$1" in
  copy)
    shift
    printf "%s" "$*" | handle_copy "$tool"
    ;;
  *)
    show_error "$0: Unknown command \"$1\""
    exit 1
esac
