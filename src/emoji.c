#include <glib.h>
#include <stdlib.h>
#include <string.h>

#include <rofi/helper.h>

#include "emoji.h"
#include "utils.h"

Emoji *emoji_new(char *bytes, char *name, char *group, char *subgroup,
                 char **keywords) {
  Emoji *emoji = g_new(Emoji, 1);
  emoji->bytes = bytes;
  emoji->name = name;
  emoji->group = group;
  emoji->subgroup = subgroup;
  emoji->keywords = keywords;
  return emoji;
}

void emoji_free_inside(Emoji *emoji) {
  g_free(emoji->bytes);
  g_free(emoji->name);
  g_free(emoji->group);
  g_free(emoji->subgroup);
  g_strfreev(emoji->keywords);
}

char *new_format_entry(const char *text, int capitalize) {
  if (text == NULL) {
    return NULL;
  }

  if (strlen(text) == 0) {
    return NULL;
  }

  char *escaped = g_markup_escape_text(text, -1);
  if (capitalize) {
    escaped[0] = g_ascii_toupper(escaped[0]);
  }

  return escaped;
}

char *emoji_format(const Emoji *emoji, const char *format) {
  char *bytes = new_format_entry(emoji->bytes, FALSE);
  char *name = new_format_entry(emoji->name, TRUE);
  char *group = new_format_entry(emoji->group, TRUE);
  char *subgroup = new_format_entry(emoji->subgroup, TRUE);

  char **keywords = g_strdupv(emoji->keywords);
  capitalize_v(keywords);

  char *keywords_str = g_strjoinv(", ", keywords);
  g_free(keywords);

  char *keywords_entry = new_format_entry(keywords_str, FALSE);
  g_free(keywords_str);

  // clang-format off
  char *formatted = helper_string_replace_if_exists(
    (char *) format, // LOL C. "trust me bro"
    "{emoji}", bytes,
    "{name}", name,
    "{group}", group,
    "{subgroup}", subgroup,
    "{keywords}", keywords_entry,
    NULL
  );
  // clang-format on

  g_free(bytes);
  g_free(name);
  g_free(group);
  g_free(subgroup);
  g_free(keywords_entry);

  return formatted;
}
