#include <glib.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "emoji_list.h"

EmojiList *emoji_list_new(int capacity) {
  EmojiList *list = g_new(EmojiList, 1);

  list->capacity = capacity;
  list->length = 0;
  list->emojis = g_new(Emoji, capacity);

  return list;
}

int emoji_list_grow(EmojiList *self) {
  int new_capacity = self->capacity * 2;
  self->emojis = g_renew(Emoji, self->emojis, new_capacity);
  self->capacity = new_capacity;
  return 1;
}


int emoji_list_push(EmojiList *self, Emoji* emoji) {
  if (self->length == self->capacity) {
    int result = emoji_list_grow(self);
    if (!result) {
      return 0;
    }
  }

  self->emojis[self->length] = *emoji;
  g_free(emoji);
  self->length += 1;
  return 1;
}

Emoji *emoji_list_get(EmojiList *self, unsigned int index) {
  if (self->length > index) {
    return &(self->emojis[index]);
  } else {
    return NULL;
  }
}

void emoji_list_free(EmojiList *self) {
  if (self == NULL) {
    return;
  }

  for (int i = 0; i < self->length; ++i) {
    emoji_free_inside(&self->emojis[i]);
  }

  g_free(self->emojis);
  self->emojis = NULL;
  self->length = 0;
  self->capacity = 0;
  g_free(self);
}

void debug_emoji_list(EmojiList *list) {
  if (list == NULL) {
    printf("debug_emoji_list: Passed NULL\n");
  } else if (list->length == 0) {
    printf("debug_emoji_list: capacity=%5d, length=%5d\n", list->capacity, list->length);
  } else {
    Emoji first = list->emojis[0];
    Emoji last = list->emojis[list->length - 1];
    printf("debug_emoji_list: capacity=%5d, length=%5d. First=%s, Last=%s\n", list->capacity, list->length, first.bytes, last.bytes);
  }
}
