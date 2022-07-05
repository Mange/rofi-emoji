#ifndef MENU_H
#define MENU_H

#include "actions.h"
#include "plugin.h"

void emoji_menu_init(EmojiModePrivateData *pd);
void emoji_menu_destroy(EmojiModePrivateData *pd);

unsigned int emoji_menu_get_num_entries(const EmojiModePrivateData *pd);
char *emoji_menu_get_message(const EmojiModePrivateData *pd);
char *emoji_menu_get_display_value(const EmojiModePrivateData *pd,
                                   unsigned int line);

int emoji_menu_token_match(const EmojiModePrivateData *pd,
                           rofi_int_matcher **tokens, unsigned int line);

char *emoji_menu_preprocess_input(EmojiModePrivateData *pd, const char *input);

Action emoji_menu_on_event(EmojiModePrivateData *pd, const Event event,
                           unsigned int line);

#endif // MENU_H
