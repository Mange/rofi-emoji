#ifndef EMOJI_LIST_H
#define EMOJI_LIST_H

#include "emoji.h"

typedef struct EmojiList {
  int capacity;
  int length;
  Emoji *emojis;
} EmojiList;

EmojiList *emoji_list_new(int capacity);
int emoji_list_resize(EmojiList *self, int capacity);
int emoji_list_push(EmojiList *self, Emoji* emoji);
Emoji *emoji_list_get(EmojiList *self, unsigned int index);
void emoji_list_free(EmojiList *self);

void debug_emoji_list(EmojiList *list);

#endif // EMOJI_LIST_H
