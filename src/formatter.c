#include <glib.h>
#include <rofi/helper.h>

#include "emoji.h"
#include "utils.h"

char *new_format_entry(const char *text) {
  if (text == NULL) {
    return NULL;
  }

  if (strlen(text) == 0) {
    return NULL;
  }

  char *escaped = g_markup_escape_text(text, -1);

  return escaped;
}

char *format_emoji(const Emoji *emoji, const char *format) {
  char *bytes = new_format_entry(emoji->bytes);
  char *name = new_format_entry(emoji->name);
  char *group = new_format_entry(emoji->group);
  char *subgroup = new_format_entry(emoji->subgroup);

  char *keywords_str = g_strjoinv(", ", emoji->keywords);
  char *keywords_entry = new_format_entry(keywords_str);
  g_free(keywords_str);

  char *cp = codepoint(emoji->bytes);

  // clang-format off
  char *formatted = helper_string_replace_if_exists(
    (char *) format, // LOL C. "trust me bro"
    "{emoji}", bytes,
    "{name}", name,
    "{group}", group,
    "{subgroup}", subgroup,
    "{keywords}", keywords_entry,
    "{codepoint}", cp,
    NULL
  );
  // clang-format on

  g_free(bytes);
  g_free(name);
  g_free(group);
  g_free(subgroup);
  g_free(keywords_entry);
  g_free(cp);

  return formatted;
}
