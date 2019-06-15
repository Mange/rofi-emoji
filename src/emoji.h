#ifndef EMOJI_H
#define EMOJI_H

typedef struct Emoji {
  char *bytes;
  char *name;
  char *keywords;
  char *group;
  char *subgroup;
} Emoji;

Emoji *emoji_new(char *bytes, char *name, char *keywords, char* group, char *subgroup);
void emoji_free_inside(Emoji *emoji);

#endif // EMOJI_H
