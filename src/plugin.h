#ifndef PLUGIN_H
#define PLUGIN_H

#include <glib.h>

// Must be included before other rofi includes.
#include <rofi/mode.h>

#include "emoji.h"

typedef struct {
  GPtrArray *emojis;
  Emoji *selected_emoji;
  char *message;

  // For search
  char **search_matcher_strings;
  char *format;
  rofi_int_matcher **group_matchers;
  rofi_int_matcher **subgroup_matchers;

  // For menu
  char **menu_matcher_strings;
} EmojiModePrivateData;

typedef enum {
  SELECT,
  CUSTOM_ACTION,
  EXIT,
} Event;

#endif // PLUGIN_H
