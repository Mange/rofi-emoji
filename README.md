# Rofi emoji plugin

An emoji selector plugin for Rofi that copies the selected emoji to the
clipboard.

## Screenshot

![Screenshot showing a Rofi window searching for emojis containing "uni", the
emoji for "Unicorn face" being selected](screenshot.png)

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

### Command line arguments

Due to a limitation in Rofi's plugin system, this plugin cannot append
additional options to the output of `rofi -help`.

The plugin adds the following command line arguments to `rofi`:

| Name            | Description                                              |
|-----------------|----------------------------------------------------------|
| `-emoji-file`   | Path to custom emoji database file.                      |
| `-emoji-format` | Custom formatting string for rendering lines. See below. |

#### Format

The formatting string should be valid Pango markup with placeholders for the
Emoji values found in the database.

The logic of this follows the same rule as Rofi's `-ssh-command` option,
quickly summarized as such:

* Items between curly braces (`{}`) are replaced with Pango-escaped text.
* Wrapping an item inside brackets (`[]`) will hide the entire section if the
  value is empty.

The default format string is this:

```
{emoji} <span weight='bold'>{name}</span>[ <span size='small'>({keywords})</span>]
```

This will render the emoji with its name next to it in bold, and if the emoji
has any keywords they will be shown in a parenthesised list with a smaller font
size.

| Item        | Example                                                                 |
|-------------|-------------------------------------------------------------------------|
| `emoji`     | 🤣                                                                      |
| `name`      | Rolling on the floor laughing                                           |
| `group`     | Smileys & Emotion                                                       |
| `subgroup`  | Face-smiling                                                            |
| `keywords`  | Face, Floor, Laugh, Rofl, Rolling, Rolling on the floor laughing, Rotfl |

| | |
|---|---|
| ⚠️ **NOTE:** | Rofi does not have a way to escape brackets, so you may not use literal `[]` inside your output. |

## Dependencies

| Dependency | Version      |
|------------|--------------|
| rofi       | 1.4 (or git) |

### Optional dependencies

In order to actually use rofi-emoji an "adapter" need to be installed, as
appropriate for your environment.

| Dependency   | Environment             |
|--------------|-------------------------|
| xsel         | X11                     |
| xclip        | X11                     |
| copyq        | X11                     |
| wl-clipboard | Wayland                 |

You only need to install one of them for your environment.

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
- `automake`
- `libtool-bin`
- `libtool`

Download the source and run the following to install it:

```bash
autoreconf -i
mkdir build
cd build/
../configure
make
sudo make install
```

If you plan on developing the code and want to test the plugin, you can also
run `./run-development.sh`, which will do all setup steps for you and then
start Rofi using the locally compiled plugin and clipboard adapter script. This
will not affect your system and does not require root.

### Running tests

Also install `check` and run the following commands after doing the **Compile
from source** steps above.

```bash
# In project root
automake -a
cd build
make check VERBOSE=true
```

There is not a lot of things to test here since Rofi doesn't expose any of its
internal methods as a library to link the test binaries against, which means
it's not possible to compile and link any tests for any files where a Rofi
dependency is used.

## Emoji database

When installing, the emoji database is installed in
`$PREFIX/share/rofi-emoji/all_emojis.txt`.

The plugin will search `$XDG_DATA_DIRS` for a directory where
`rofi-emoji/all_emojis.txt` exists in if no `-emoji-file` option is set.

If the plugin cannot find the file, make sure `$XDG_DATA_DIRS` is set
correctly. If it is unset it should default to `/usr/local/share:/usr/share`,
which works with the most common prefixes.

### Custom database

The emoji database is a plain-text file that lists one emoji per line. It has
the following format:

```
EMOJI_BYTES        - The bytes of the emoji, for example "🤣". This is what is acted on.
\t                 - Tab character
GROUP_NAME         - The name of the group, for example "Smileys & Emotion".
\t                 - Tab character
SUBGROUP           - The name of the subgroup, for example "face-smiling".
\t                 - Tab character
NAME               - Name of emoji, for example "rolling on the floor laughing".
\t                 - Tab character
KEYWORD_1          - Keyword of the emoji, for example "rofl".
(" | " KEYWORD_n)… - Additional keywords are added with pipes and spaces between them.
\n                 - Newline ends the current record.
```

**Example rows:**

```
🤣	Smileys & Emotion	face-smiling	rolling on the floor laughing	face | floor | laugh | rofl | rolling | rolling on the floor laughing | rotfl
😂	Smileys & Emotion	face-smiling	face with tears of joy	face | face with tears of joy | joy | laugh | tear
🙂	Smileys & Emotion	face-smiling	slightly smiling face	face | slightly smiling face | smile
🙃	Smileys & Emotion	face-smiling	upside-down face	face | upside-down | upside down | upside-down face
```

### Updating default database to a newer version

The list is copied from the [Mange/emoji-data][emoji-data] repo.

## License

This plugin is released under the MIT license. See `LICENSE` for more details.

[emoji-data]: https://github.com/Mange/emoji-data
