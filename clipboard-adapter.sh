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
      exec xsel --clipboard --input
      ;;
    xclip)
      exec xclip -selection clipboard -in
      ;;
    wl-copy)
      exec wl-copy
      ;;
    *)
      show_error "$1 has no implementation for copying yet"
      exit 1
  esac
}

paste() {
  if [ "$XDG_SESSION_TYPE" = wayland ]; then
    show_error "paste is not implemented for wayland currently"
  else
    if hash xdotool 2>/dev/null; then
      # …Shift+Insert pastes primary selection
      xdotool key Shift+Insert
    else
      notify "xdotool must be installed for direct insert. Emoji was copied instead."
      handle_copy "$1"
    fi
  fi
}

handle_insert() {
  case "$1" in
    xsel)
      # Set PRIMARY clipboard…
      xsel --primary --input
      # …and paste in focused window
      paste xsel
      ;;
    xclip)
      # Set PRIMARY clipboard…
      xclip -selection primary -in
      # …and paste in focused window
      paste xclip
      ;;
    wl-copy)
      # Currently does not support inserting directly. Copy instead!
      notify "wl-copy does not allow for inserting text directly. Emoji is copied instead."
      handle_copy wl-copy
      ;;
    *)
      show_error "$1 has no implementation for inserting yet"
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
  if [ "$XDG_SESSION_TYPE" = wayland ]; then
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
    handle_copy "$tool"
    ;;
  insert)
    handle_insert "$tool"
    ;;
  *)
    show_error "$0: Unknown command \"$1\""
    exit 1
esac
