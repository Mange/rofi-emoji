# Rofi emoji plugin

An emoji selector plugin for Rofi that copies the selected emoji to the
clipboard.

## Usage

Run rofi like:

```bash
rofi -show emoji -modi emoji
```

| Key              | Effect                  |
|------------------|-------------------------|
| <kbd>Enter</kbd> | Copy emoji to clipboard |

**Note:** If you change your keybind for `kb-accept`, then your change will
also apply here.

## Screenshot

![Screenshot showing a Rofi window searching for emojis containing "uni", the
emoji for "Unicorn face" being selected](screenshot.png)

## Dependencies

| Dependency | Version      |
|------------|--------------|
| rofi       | 1.4 (or git) |

### Optional dependencies

In order to actually use rofi-emoji some "adapters" need to be installed, as
appropriate for your environment.

| Dependency   | Notes                        |
|--------------|------------------------------|
| xsel         | For X11.                     |
| xclip        | For X11.                     |
| wl-clipboard | For Wayland. (**Untested!**) |

## Installation

<a href="https://repology.org/metapackage/rofi-emoji/versions">
    <img src="https://repology.org/badge/vertical-allrepos/rofi-emoji.svg" alt="Packaging status" align="right">
</a>

### Arch Linux

```bash
pacman -S rofi-emoji
```

### Manjaro

```bash
pacman -S rofi-emoji
```

### Void Linux

```bash
xbps-install -S rofi-emoji
```

### NixOS

```bash
nix-env -e rofi-emoji
```

### Compile from source

`rofi-emoji` uses autotools as build system. On Debian/Ubuntu based systems you
will need to install the packages first:

- `rofi-dev`
- `autoconf`
- `libtool`
- `libtool-bin`

Download the source and run the following to install it:

```bash
autoreconf -i
mkdir build
cd build/
../configure
make
sudo make install
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

This plugin is released under the MIT license. See `LICENSE` for more details.

[emoji-data]: https://github.com/Mange/emoji-data
