#include <rofi/helper.h>

#include "formatter.h"
#include "menu.h"

const int NUM_MENU_ITEMS = 5;
typedef enum {
  EMOJI_MENU_PRIMARY = 0,
  EMOJI_MENU_SECONDARY = 1,
  EMOJI_MENU_NAME = 2,
  EMOJI_MENU_CODEPOINT = 3,
  EMOJI_MENU_BACK = 4,
} MenuItem;

char *emoji_menu_get_display_value(const EmojiModePrivateData *pd,
                                   unsigned int line) {
  switch (line) {
  case EMOJI_MENU_BACK:
    return g_strdup("⬅ Back to search");
  case EMOJI_MENU_PRIMARY:
    return format_emoji(pd->selected_emoji,
                        pd->search_default_action == INSERT_EMOJI ?
                          "Copy emoji ({emoji})" : "Insert emoji ({emoji})");
  case EMOJI_MENU_SECONDARY:
    return format_emoji(pd->selected_emoji,
                        pd->search_default_action == INSERT_EMOJI ?
                          "Insert emoji ({emoji})" : "Copy emoji ({emoji})");
  case EMOJI_MENU_NAME:
    return format_emoji(pd->selected_emoji, "Copy name (<tt>{name}</tt>)");
  case EMOJI_MENU_CODEPOINT:
    return format_emoji(pd->selected_emoji,
                        "Copy codepoint (<tt>{codepoint}</tt>)");
  default:
    return g_strdup("<invalid menu entry>");
  }
}

void emoji_menu_init(EmojiModePrivateData *pd) {
  if (pd->menu_matcher_strings != NULL) {
    emoji_menu_destroy(pd);
  }

  if (pd->selected_emoji != NULL) {
    char **items = g_new(char *, NUM_MENU_ITEMS + 1);
    for (int i = 0; i < NUM_MENU_ITEMS; ++i) {
      items[i] = emoji_menu_get_display_value(pd, i);
    }
    items[NUM_MENU_ITEMS] = NULL;

    pd->menu_matcher_strings = items;
  }
}

void emoji_menu_destroy(EmojiModePrivateData *pd) {
  if (pd->menu_matcher_strings != NULL) {
    g_strfreev(pd->menu_matcher_strings);
    pd->menu_matcher_strings = NULL;
  }
}

unsigned int emoji_menu_get_num_entries(const EmojiModePrivateData *pd) {
  return NUM_MENU_ITEMS;
}

char *emoji_menu_get_message(const EmojiModePrivateData *pd) {
  Emoji *emoji = pd->selected_emoji;
  if (emoji == NULL) {
    return NULL;
  }

  return format_emoji(emoji, "{emoji} <span weight='bold'>{name}</span>\n"
                             "{group} [» {subgroup}]\n"
                             "[<span font-size='small'>Keywords: <span "
                             "style='oblique'>{keywords}</span></span>]");
}

char *emoji_menu_preprocess_input(EmojiModePrivateData *pd, const char *input) {
  return g_strdup(input);
}

int emoji_menu_token_match(const EmojiModePrivateData *pd,
                           rofi_int_matcher **tokens, unsigned int line) {
  return line < NUM_MENU_ITEMS &&
         helper_token_match(tokens, pd->menu_matcher_strings[line]);
}

Action emoji_menu_select_item(EmojiModePrivateData *pd, unsigned int line) {
  if (line >= NUM_MENU_ITEMS) {
    return NOOP;
  }

  switch (line) {
  case EMOJI_MENU_BACK:
    return EXIT_MENU;
  case EMOJI_MENU_PRIMARY:
    return pd->search_default_action == INSERT_EMOJI ? COPY_EMOJI : INSERT_EMOJI;
  case EMOJI_MENU_SECONDARY:
    return pd->search_default_action == INSERT_EMOJI ? INSERT_EMOJI : COPY_EMOJI;
  case EMOJI_MENU_NAME:
    return COPY_NAME;
  case EMOJI_MENU_CODEPOINT:
    return COPY_CODEPOINT;
  default:
    g_assert_not_reached();
  }
}

Action emoji_menu_on_event(EmojiModePrivateData *pd, const Event event,
                           unsigned int line) {
  switch (event) {
  case SELECT_DEFAULT:
    return emoji_menu_select_item(pd, line);
  case EXIT:
    return EXIT_MENU;
  default:
    return NOOP;
  }
}
