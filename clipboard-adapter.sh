#!/bin/sh
# Clipboard adapter for rofi-emoji.
#
# Usage:
#   clipboard-adapter.sh copy "emoji"
#     Set clipboard to "emoji".
#     Example: clipboard-adapter.sh copy "😁"
#
#   clipboard-adapter.sh insert "emoji" "codepoints"
#     Tries to insert the emoji in the focused application like a keypress. If
#     insertion is not enabled, then copy instead.
#     Example: clipboard-adapter.sh insert "😁" "U1F601"
#
# Detects Wayland and X and finds the appropriate tool for the current
# environment.
#
# If stderr is bound to /dev/null, then error messages will be emitted as
# notifications using `notify-send` (if available).
# When rofi is run from a terminal, the output both on stderr and stdout should
# be visible to the user and makes it possible to do some debugging.
#

main() {
  # Determine command
  case "$1" in
    insert)
      shift
      do_insert "$1" "$2" || do_copy "$1"
      ;;
    copy)
      shift
      do_copy "$1"
      ;;
    *)
      show_error "$0: Unknown command \"$1\""
      exit 1
  esac
}

# Insert the emoji into the focused application.
#
# $1 = Emoji bytes
# $2 = Emoji codepoints, expressed as "U<hex> U<hex>"
do_insert() {
  # Determine environment
  if is_wayland; then
    # No adapter available for this yet
    return 1
  else
    # Only xdotool is supported on Xorg for now.
    if has xdotool; then
      xdotool_insert "$2"
    else
      return 1
    fi
  fi
}

# Copy the emoji into the clipboard.
#
# $1 = Emoji bytes
do_copy() {
  # Determine environment
  if is_wayland; then
    if has wl-copy; then
      wl_copy_copy "$1"
    else
      show_error "No supported adapter could be found."
      return 1
    fi
  else
    if has xsel; then
      xsel_copy "$@"
    elif has xclip; then
      xclip_copy "$@"
    else
      show_error "No supported adapter could be found."
      return 1
    fi
  fi
}

# Returns successfully if the current desktop environment is running on top of
# Wayland.
is_wayland() {
  [ "$XDG_SESSION_TYPE" = wayland ] || [ -n "$WAYLAND_DISPLAY" ]
}

# Returns successfully if stderr is a null device.
stderr_is_null() {
  test /proc/self/fd/2 -ef /dev/null
}

# Returns successfully if the given command can be found on the path.
has() {
  hash "$1" 2>/dev/null
}

# Notify the user; try to send a notification if `notify-send` is available,
# else fall back on plain `echo`.
notify() {
  if hash notify-send 2>/dev/null; then
    notify-send rofi-emoji "$@"
  else
    echo "$@" >&2
  fi
}

# Show an error to the user, in the best way possible.
show_error() {
  if stderr_is_null; then
    notify "$@"
  else
    echo "$@" >&2
  fi
}


############
# Adapters #
############

# xdotool (insert only)
xdotool_insert() {
  xdotool key "$1" || false
}

# xsel (copy only)
xsel_copy() {
  printf "%s" "$1" | xsel --clipboard --input
}

# xclip (copy only)
xclip_copy() {
  printf "%s" "$1" | xclip -selection clipboard -in
}

# wl-copy (copy only)
wl_copy_copy() {
  printf "%s" "$1" | wl-copy
}

################

main "$@"
