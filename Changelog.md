# `master` branch

Nothing yet.

# Version 2.2.0 (2021-05-19)

## Added

* Full `LICENSE` file, detailing the MIT license mentioned in the README.

## Changes

* Updated [`emoji-data`][emoji-data] to version 2.3, adding more emojis and better keywords.

# Version 2.1.2 (2020-03-30)

## Fixed

* Build configuration now includes undocumented Cairo dependency.

# Version 2.1.1 (2020-03-23)

## Fixed

* Wayland detection under Sway.

# Version 2.1.0 (2019-10-06)

Change clipboard adapter to use arguments instead of STDIN, which should
prevent some issues from occurring regarding subprocesses getting stuck in a
blocking read.

## Changes

* Clipboard adapter script now accepts emoji bytes as an argument instead of
  standard input.

# Version 2.0 (2019-07-23)

Due to a lot of issues with the "insert" action on many environments, and
inconsistent support for the primary selection, this feature has now been
dropped and the plugin is again only doing clipboard copying.

## Removed

* Direct insert via <kbd>Enter</kbd>; now this key also copies the emoji to the
  clipboard to let you paste it manually.
* `xdotool` as a supported adapter.

# Version 1.2 (2019-06-16)

This is a large upgrade to the emoji data, which restores a few things that
went missing in version 1.1.

## Fixed

* Named country flags are back!
* Emoji names are present again (from 1.0), together with all the keywords from
  1.1.

## Changes

* Updated to [`emoji-data`][emoji-data] version 2.0.

# Version 1.1 (2019-06-02)

First new feature release! This release does a lot of improvements and adds
some new features.

## Fixed

* Rofi plugin directory is now detected automatically via `pkg-config`.

## Added

* Changed default action to insert emoji via `xdotool`.
  * Hold <kbd>Shift</kbd> to copy it like before.
* Support for `xclip`.
* Experimental, untested support for Wayland via `wl-clipboard`.
* Adapter script for adding support for other clipboard manager.

## Changed

* More emojis: Unicode 12.0.
* Emojis have multiple keywords now (for example, 😎 is now also *cool*).

## Known issues

* Country flags are no longer searchable via country names. See
  [Mange/emoji-data][emoji-data].

[emoji-data]: https://github.com/Mange/emoji-data

# Version 1.0 (2018-05-11)

Initial release with Unicode 11.0.
