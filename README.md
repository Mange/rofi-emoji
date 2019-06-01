# Rofi emoji plugin

An emoji selector plugin for Rofi that can either insert the emoji directly or copy it to the clipboard.

## Usage

Run rofi like:

```bash
rofi -show emoji -modi emoji
```

| Key                               | Effect                  |
|-----------------------------------|-------------------------|
| <kbd>Enter</kbd>                  | Insert emoji            |
| <kbd>Shift</kbd>+<kbd>Enter</kbd> | Copy emoji to clipboard |

**Note:** If you change your keybinds for `kb-accept` or `kb-accept-alt`, then
your changes will also apply here.

## Screenshot

![Screenshot showing a Rofi window searching for emojis containing "uni", the
emoji for "Unicorn face" being selected](screenshot.png)

## Installation

**Dependencies**

| Dependency | Version      |
|------------|--------------|
| rofi       | 1.4 (or git) |
| xsel       |              |
| xdotool    |              |

### Arch Linux

`rofi-emoji` can be installed via [AUR](https://aur.archlinux.org/):
[rofi-emoji](https://aur.archlinux.org/packages/rofi-emoji/)

### Compile from source

`rofi-emoji` uses autotools as build system. Download the source and run the following to install it:

```bash
$ autoreconf -i
$ mkdir build
$ cd build/
$ ../configure --prefix=/usr/local # use same prefix as rofi
$ make
$ sudo make install
```

#### Rofi prefix

You should be able to find rofi's prefix using these commands:

```bash
# Simplest to understand; resolve "which rofi" to the real path (in case it is a symlink),
# then resolve its grandparent (/usr/local/bin/rofi -> /usr/local/bin -> /usr/local)
realpath -mL "$(realpath -L "$(which rofi)")/../.."

# This might work if you don't have realpath(1) installed. It uses dirname
twice instead of realpath -Lm to go up two steps.
dirname "$(dirname "$(readlink -f "$(which rofi)")")"

# Still didn't work? That leaves you with your human intuition.
ls -l "$(which rofi)"
```

## Emoji database

When installing the emoji database (`all_emojis.txt` file) is installed in
`$PREFIX/share/rofi-emoji`. The plugin will search `$XDG_DATA_DIRS` for a
directory where `rofi-emoji/all_emojis.txt` exists in. If the plugin cannot
find the data, make sure `$XDG_DATA_DIRS` is set correctly.

If it is unset it should default to `/usr/local/share:/usr/share`, which works
with the most common prefixes.

### Updating to a newer version

The list is copied from the [Mange/emoji-data][emoji-data] repo.

## License

This plugin is released under the MIT license.

[emoji-data]: https://github.com/Mange/emoji-data
