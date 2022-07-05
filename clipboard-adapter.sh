#!/bin/sh

usage() {
  cat <<EOF
Clipboard adapter for rofi-emoji.

Usage:
  echo "the text" | clipboard-adapter.sh copy
    Set clipboard to "the text"

  echo "the text" | clipboard-adapter.sh insert
    Try to write "the text" to the focused window. Will also copy the text in
    case insertion fails.

Detects Wayland and X and finds the appropriate tool for the current
environment.

When rofi is run from a terminal, the output both on stderr and stdout should
be visible to the user and makes it possible to do some debugging.
If stderr is bound to /dev/null, the script will try to send any error messages
to a notification service.
EOF
}

command=

main() {
  while [ "$#" -gt 0 ]; do
    case "$1" in
    --help)
      shift
      command=help
      break
      ;;
    copy | insert | help)
      command="$1"
      shift
      ;;
    --)
      shift
      break
      ;;
    *)
      show_error "Unknown option: $1"
      exit 1
      ;;
    esac
  done

  case "$command" in
  help)
    usage
    exit 0
    ;;
  copy)
    perform_copy
    ;;
  insert)
    # Also copy when doing insert to give users a fallback in case automatic
    # insertion didn't work. Inserting unicode characters in X windows is a
    # very erratic process that works only in some UI toolkits.
    #
    # Since stdin should be sent to two different processes, read it first and
    # replicate manually.
    input="$(cat -)"
    printf "%s" "$input" | perform_copy
    printf "%s" "$input" | perform_insert
    ;;
  *)
    usage >&2
    exit 1
    ;;
  esac
}

stderr_is_null() {
  test /proc/self/fd/2 -ef /dev/null
}

show_error() {
  if stderr_is_null && hash notify-send 2>/dev/null; then
    notify-send rofi-emoji "$@"
  else
    echo "$@" >&2
  fi
}

perform_copy() {
  tool=$(find_copy_tool)

  case "$tool" in
  xsel)
    xsel --clipboard --input
    ;;
  xclip)
    xclip -selection clipboard -in
    ;;
  copyq)
    copyq copy -
    ;;
  wl-copy)
    wl-copy
    ;;
  "")
    show_error "Could not find any tool to handle copying. Please install a clipboard handler."
    exit 1
    ;;
  *)
    show_error "$tool has no implementation for copying yet"
    exit 2
    ;;
  esac
}

perform_insert() {
  tool=$(find_insert_tool)

  case "$tool" in
  xdotool)
    xdotool type --clearmodifiers --file -
    ;;
  wtype)
    wtype -
    ;;
  "")
    show_error "Could not find any tool to handle insertion. Please install xdotool or wtype."
    exit 1
    ;;
  *)
    show_error "$tool has no implementation for insertion yet"
    exit 2
    ;;
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
find_copy_tool() {
  if [ "$XDG_SESSION_TYPE" = wayland ] || [ -n "$WAYLAND_DISPLAY" ]; then
    try_tool wl-copy || return 1
  else
    try_tool xsel || try_tool xclip || try_tool copyq || return 1
  fi
}

# Find the best insertion tool to use.
find_insert_tool() {
  if [ "$XDG_SESSION_TYPE" = wayland ] || [ -n "$WAYLAND_DISPLAY" ]; then
    try_tool wtype || return 1
  else
    try_tool xdotool || return 1
  fi
}

main "$@"
