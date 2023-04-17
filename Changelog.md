# Version 3.2.0 (2023-04-17)

## Changed

* Updated [`emoji-data`][emoji-data] to [version 2.5][emoji-data-2.5], adding
  more emojis (Emoji 15) and better keywords.

# Version 3.1.0 (2022-09-12)

## Added

* Added menu option to insert emoji no matter which mode is currently active.
  ([Alexander Schulz (hlfbt)](https://github.com/hlfbt))
* Change default menu item between Copy and Insert based on the current mode
  such that the default is the opposite of the mode. ([Alexander Schulz
  (hlfbt)](https://github.com/hlfbt))

# Version 3.0.1 (2022-07-24)

## Fixed

* Make project build without `pkgconf` dependency; only `pkg-config` binary
  and other listed dependencies should be required.

# Version 3.0.0 (2022-07-05)

## Breaking changes

* Adapter script has a new call signature. Read the `--help` output to see it.

## Added

* Insert mode (that tries) to insert emoji directly into foreground app.
* Menu mode with options on what to do with the emoji.
* Stdout mode that emits the selected emoji to stdout.
* Group and subgroup filter for searches using `@groupname` or `#subgroup`.
* The `-emoji-mode` option to set default selection mode.
* Quick shortcut to open menu, no matter what the default mode is.
* Quick shortcut to copy emoji, no matter what the default mode is.
* The `-emoji-file` option to read custom emoji databases.
* Documentation about the format of the Emoji database.
* The `-emoji-format` option to set custom rendering of lines.

## Changed

* New default selection mode: Insert.
* The default rendering of Emoji entries.
  * No longer showing group and subgroup.
  * No empty parenthesis for entries without keywords.
  * Names are capitalized.

  **Before:**
  > ☺️ **smiling face** () [Smileys & Emotion / face-affection]

  **After:**
  > ☺️ **Smiling face**

# Version 2.3.0 (2022-02-02)

## Added

* Support for `copyq` X11 clipboard adapter. ([Muhammad Mabrouk
  (M-Mabrouk1)](https://github.com/M-Mabrouk1))
* Emoji 14.0 emojis and latest keywords from CLDR

## Changed

* Allow clipboard-adapter.sh script to be replaced and run by a different
  interpreter than `/bin/sh`.

# Version 2.2.0 (2021-05-19)

## Added

* Full `LICENSE` file, detailing the MIT license mentioned in the README.

## Changed

* Updated [`emoji-data`][emoji-data] to version 2.3, adding more emojis and
  better keywords.

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

## Changed

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

## Changed

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

# Version 1.0 (2018-05-11)

Initial release with Unicode 11.0.

[emoji-data]: https://github.com/Mange/emoji-data
[emoji-data-2.5]: https://github.com/Mange/emoji-data/releases/tag/v2.5
