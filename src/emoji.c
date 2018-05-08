#include <stdlib.h>

#include "emoji.h"

Emoji *emoji_new(char *bytes, char *name, char* group, char *subgroup) {
    Emoji *emoji = malloc(sizeof(Emoji));
    emoji->bytes = bytes;
    emoji->name = name;
    emoji->group = group;
    emoji->subgroup = subgroup;
    return emoji;
}

void emoji_free(Emoji *emoji) {
  free(emoji->bytes);
  free(emoji->name);
  free(emoji->group);
  free(emoji->subgroup);
}
