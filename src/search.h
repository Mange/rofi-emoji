#ifndef SEARCH_H
#define SEARCH_H

#include "actions.h"
#include "plugin.h"

void emoji_search_init(EmojiModePrivateData *pd);
void emoji_search_destroy(EmojiModePrivateData *pd);

unsigned int emoji_search_get_num_entries(const EmojiModePrivateData *pd);
char *emoji_search_get_message(const EmojiModePrivateData *pd);
char *emoji_search_get_display_value(const EmojiModePrivateData *pd,
                                     unsigned int line);

int emoji_search_token_match(const EmojiModePrivateData *pd,
                             rofi_int_matcher **tokens, unsigned int line);

char *emoji_search_preprocess_input(EmojiModePrivateData *pd,
                                    const char *input);

Action emoji_search_on_event(EmojiModePrivateData *pd, const Event event,
                             unsigned int line);

#endif // SEARCH_H
