#include <rofi/helper.h>

#include "actions.h"
#include "formatter.h"
#include "search.h"
#include "utils.h"

const char *DEFAULT_FORMAT = "{emoji} <span weight='bold'>{name}</span>"
                             "[ <span size='small'>({keywords})</span>]";

char **generate_matcher_strings(GPtrArray *list);

void emoji_search_init(EmojiModePrivateData *pd) {
  pd->search_matcher_strings = generate_matcher_strings(pd->emojis);
}

void emoji_search_destroy(EmojiModePrivateData *pd) {
  g_strfreev(pd->search_matcher_strings);
  helper_tokenize_free(pd->group_matchers);
  helper_tokenize_free(pd->subgroup_matchers);
}

unsigned int emoji_search_get_num_entries(const EmojiModePrivateData *pd) {
  return pd->emojis->len;
}

char *emoji_search_get_message(const EmojiModePrivateData *pd) { return NULL; }

char *emoji_search_get_display_value(const EmojiModePrivateData *pd,
                                     unsigned int line) {
  if (line >= pd->emojis->len) {
    return g_strdup("");
  }

  Emoji *emoji = g_ptr_array_index(pd->emojis, line);
  const char *format = pd->format;
  if (format == NULL || format[0] == '\0') {
    format = DEFAULT_FORMAT;
  }

  if (emoji == NULL) {
    return g_strdup("n/a");
  } else {

    return format_emoji(emoji, format);
  }
}

char *emoji_search_preprocess_input(EmojiModePrivateData *pd,
                                    const char *input) {
  char *query;
  char *group_query;
  char *subgroup_query;

  if (pd->group_matchers != NULL) {
    helper_tokenize_free(pd->group_matchers);
    pd->group_matchers = NULL;
  }
  if (pd->subgroup_matchers != NULL) {
    helper_tokenize_free(pd->subgroup_matchers);
    pd->subgroup_matchers = NULL;
  }

  tokenize_search(input, &query, &group_query, &subgroup_query);

  if (group_query != NULL) {
    pd->group_matchers = helper_tokenize(group_query, FALSE);
  }

  if (subgroup_query != NULL) {
    pd->subgroup_matchers = helper_tokenize(subgroup_query, FALSE);
  }

  return query;
}

int emoji_search_token_match(const EmojiModePrivateData *pd,
                             rofi_int_matcher **tokens, unsigned int line) {
  if (line >= pd->emojis->len) {
    return FALSE;
  }

  if (pd->group_matchers != NULL || pd->subgroup_matchers != NULL) {
    Emoji *emoji = g_ptr_array_index(pd->emojis, line);

    if (pd->group_matchers != NULL) {
      if (!helper_token_match(pd->group_matchers, emoji->group)) {
        return FALSE;
      }
    }

    if (pd->subgroup_matchers != NULL) {
      if (!helper_token_match(pd->subgroup_matchers, emoji->subgroup)) {
        return FALSE;
      }
    }
  }

  return helper_token_match(tokens, pd->search_matcher_strings[line]);
}

Action emoji_search_on_event(EmojiModePrivateData *pd, const Event event,
                             unsigned int line) {
  switch (event) {
  case SELECT_DEFAULT:
    if (line >= pd->emojis->len) {
      return NOOP;
    }
    return pd->search_default_action;
  case SELECT_ALTERNATIVE:
    if (line >= pd->emojis->len) {
      return NOOP;
    }
    return OPEN_MENU;
  case SELECT_CUSTOM_1:
    return COPY_EMOJI;
  case EXIT:
    return EXIT_SEARCH;
  default:
    return NOOP;
  }
}

char **generate_matcher_strings(GPtrArray *list) {
  char **strings = g_new(char *, list->len + 1);
  for (int i = 0; i < list->len; ++i) {
    Emoji *emoji = g_ptr_array_index(list, i);

    strings[i] = format_emoji(emoji, "{emoji} {name} {keywords}");
  }
  strings[list->len] = NULL;
  return strings;
}
