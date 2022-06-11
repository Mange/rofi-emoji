#include <glib.h>
#include <stdlib.h>
#include <string.h>

#include "emoji.h"

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
