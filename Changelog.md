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
