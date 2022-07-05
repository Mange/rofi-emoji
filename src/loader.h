#ifndef LOADER_H
#define LOADER_H

#include <glib.h>

#include "emoji.h"

GPtrArray *read_emojis_from_file(const char *path);
Emoji *parse_emoji_from_line(const char *line);

const char *scan_until(const char until, const char *input, char **result);

#endif // LOADER_H
