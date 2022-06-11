#ifndef EMOJI_H
#define EMOJI_H

typedef struct Emoji {
  char *bytes;
  char *name;
  char *group;
  char *subgroup;

  char **aliases;
} Emoji;

Emoji *emoji_new(char *bytes, char *name, char *group, char *subgroup,
                 char **aliases);
void emoji_free_inside(Emoji *emoji);

#endif // EMOJI_H
