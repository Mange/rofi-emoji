#ifndef LOADER_H
#define LOADER_H

#include "emoji_list.h"

typedef enum {
  SUCCESS = 1,
  NOT_A_FILE = 0,
  CANNOT_DETERMINE_PATH = -1
} FindEmojiFileResult;

FindEmojiFileResult find_emoji_file(char **path);
EmojiList *read_emojis_from_file(char *path);

#endif // LOADER_H
