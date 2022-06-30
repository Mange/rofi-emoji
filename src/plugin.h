#ifndef PLUGIN_H
#define PLUGIN_H

#include <glib.h>

// Must be included before other rofi includes.
#include <rofi/mode.h>

#include "actions.h"
#include "emoji.h"

typedef enum {
  SELECT_DEFAULT,
  SELECT_ALTERNATIVE,
  SELECT_CUSTOM_1,
  EXIT,
} Event;

typedef struct {
  GPtrArray *emojis;
  Emoji *selected_emoji;
  char *message;

  // For search
  Action search_default_action;
  char **search_matcher_strings;
  char *format;
  rofi_int_matcher **group_matchers;
  rofi_int_matcher **subgroup_matchers;

  // For menu
  char **menu_matcher_strings;
} EmojiModePrivateData;

#endif // PLUGIN_H
