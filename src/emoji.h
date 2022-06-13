#ifndef EMOJI_H
#define EMOJI_H

typedef struct Emoji {
  char *bytes;
  char *name;
  char *group;
  char *subgroup;

  char **keywords;
} Emoji;

Emoji *emoji_new(char *bytes, char *name, char *group, char *subgroup,
                 char **keywords);
void emoji_free(Emoji *emoji);

#endif // EMOJI_H
