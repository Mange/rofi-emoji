#include <glib.h>
#include <stdlib.h>
#include <string.h>

#include <rofi/helper.h>

#include "emoji.h"
#include "utils.h"

Emoji *emoji_new(char *bytes, char *name, char *group, char *subgroup,
                 char **aliases) {
  Emoji *emoji = g_new(Emoji, 1);
  emoji->bytes = bytes;
  emoji->name = name;
  emoji->group = group;
  emoji->subgroup = subgroup;
  emoji->aliases = aliases;
  return emoji;
}

void emoji_free_inside(Emoji *emoji) {
  g_free(emoji->bytes);
  g_free(emoji->name);
  g_free(emoji->group);
  g_free(emoji->subgroup);
  g_strfreev(emoji->aliases);
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

  char **aliases = g_strdupv(emoji->aliases);
  capitalize_v(aliases);

  char *aliases_str = g_strjoinv(", ", aliases);
  g_free(aliases);

  char *aliases_entry = new_format_entry(aliases_str, FALSE);
  g_free(aliases_str);

  // clang-format off
  char *formatted = helper_string_replace_if_exists(
    (char *) format, // LOL C. "trust me bro"
    "{emoji}", bytes,
    "{name}", name,
    "{group}", group,
    "{subgroup}", subgroup,
    "{aliases}", aliases_entry,
    NULL
  );
  // clang-format on

  g_free(bytes);
  g_free(name);
  g_free(group);
  g_free(subgroup);
  g_free(aliases_entry);

  return formatted;
}
