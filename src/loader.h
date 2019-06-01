#ifndef LOADER_H
#define LOADER_H

#include "emoji_list.h"
#include "utils.h"

FindDataFileResult find_emoji_file(char **path);
EmojiList *read_emojis_from_file(char *path);

#endif // LOADER_H
