#include <glib.h>
#include <stdlib.h>
#include <string.h>

#include "emoji.h"

Emoji *emoji_new(char *bytes, char *name, char *keywords, char* group, char *subgroup) {
    Emoji *emoji = g_new(Emoji, 1);
    emoji->bytes = bytes;
    emoji->name = name;
    emoji->keywords = keywords;
    emoji->group = group;
    emoji->subgroup = subgroup;
    return emoji;
}

void emoji_free_inside(Emoji *emoji) {
  g_free(emoji->bytes);
  g_free(emoji->name);
  g_free(emoji->keywords);
  g_free(emoji->group);
  g_free(emoji->subgroup);
}
