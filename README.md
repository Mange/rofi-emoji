# Rofi emoji plugin

**NOTE:** This is a work-in-progress. C code needs to be audited for bugs, etc.

An emoji selector plugin for Rofi that copies the selected emoji to the X11
CLIPBOARD on selection.

To run this you need rofi 1.4+ and xsel installed.

Run rofi like:

```bash
    rofi -show emoji -modi emoji
```

You may need to manually specify the `-plugin-path` option:

```bash
    rofi -show emoji -modi emoji -plugin-path /usr/local/lib/rofi
```

## Screenshot

![Screenshot showing a Rofi window searching for emojis containing "uni", the
emoji for "Unicorn face" being selected](screenshot.png)

## Compilation

### Dependencies

| Dependency | Version         |
|------------|-----------------|
| rofi 	     | 1.4 (or git)	   |

### Initial building

**rofi-emoji** uses autotools as build system. If installing from git, the
following steps should install it:

```bash
$ autoreconf -i
$ mkdir build
$ cd build/
$ ../configure
$ make
```

Now you can run `make` inside the `build/` directory to build again, or follow
the **Installation** instructions below to install the plugin.

### Running tests

Run tests with `make check`. You can also call `../configure` with the
`--enable-always-buil-tests` to get tests to run automatically on `make all`.

### Installation

Install the plugin to try it out with Rofi:

```bash
# in build dir
$ sudo make install
$ mkdir -p "${XDG_DATA_HOME:-$HOME/.local/share}/rofi-emoji"
$ cp ../emoji-test.txt "${XDG_DATA_HOME:-$HOME/.local/share}/rofi-emoji/emoji-test.txt"
```

## Refreshing the list of emojis

The emoji list is based on this URL:
https://www.unicode.org/Public/emoji/11.0/emoji-test.txt

You should be able to find the latest version and download it to refresh them.
