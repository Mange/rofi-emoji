#include <stdlib.h>

#include "emoji_list.h"

EmojiList *emoji_list_new(int capacity) {
  EmojiList *list = malloc(sizeof(EmojiList));
  if (list == NULL) {
    return NULL;
  }

  list->capacity = capacity;
  list->length = 0;
  list->emojis = NULL;

  if (capacity > 0) {
    Emoji *buf = malloc(sizeof(Emoji) * capacity);
    if (buf == NULL) {
      free(list);
      return NULL;
    } else {
      list->emojis = buf;
    }
  }

  return list;
}

int emoji_list_resize(EmojiList *self, int capacity) {
  if (capacity >= self->capacity) {
    Emoji* new_buf = malloc(sizeof(Emoji) * capacity);
    memcpy(new_buf, self->emojis, sizeof(Emoji) * self->length);
    free(self->emojis);
    self->emojis = new_buf;
    self->capacity = capacity;
    return 1;
  } else {
    printf("emoji_list_resize does not support shrinking\n");
    exit(127);
  }
}


int emoji_list_push(EmojiList *self, Emoji* emoji) {
  if (self->length == self->capacity) {
    int result = emoji_list_resize(self, self->capacity * 2);
    if (!result) {
      return 0;
    }
  }

  self->emojis[self->length] = *emoji;
  free(emoji);
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
    emoji_free(&self->emojis[i]);
  }

  free(self->emojis);
  self->emojis = NULL;
  self->length = 0;
  self->capacity = 0;
  free(self);
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
