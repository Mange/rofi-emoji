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
void emoji_free_inside(Emoji *emoji);

char *emoji_format(const Emoji *emoji, const char *format);

#endif // EMOJI_H
