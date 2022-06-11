#include <glib.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "loader.h"
#include "utils.h"

#define MAX_LINE_LENGTH 1024

// Copies the text from the `input` string up until (but not including) the
// next `until` character into a newly allocated buffer at `result`. You need
// to free `result` when you are done with it.
//
// Returns the position in the input string after the `until` character.
//
// If `until` could not be found in `string`, it will not advance and `result`
// will be set to `NULL`.
char *scan_until(const char until, char *input, char **result) {
  char *index = strchr(input, until);

  if (index == NULL) {
    *result = NULL;
    return input;
  }

  // Allocate result buffer. They need to equal the length of the matching
  // input, plus 1 for the NUL byte.
  int length = (index - input);
  *result = malloc(length + 1);
  if (result == NULL) {
    return input;
  }

  // Copy input into result
  strncpy(*result, input, length);
  (*result)[length] = '\0';

  // Advance input to character after the `until` character.
  return index + 1;
}

EmojiList *read_emojis_from_file(const char *path) {
  FILE *file = fopen(path, "r");
  if (!file) {
    return NULL;
  }

  char line[MAX_LINE_LENGTH];

  EmojiList *list = emoji_list_new(512);

  while (fgets(line, MAX_LINE_LENGTH, file) != NULL) {
    char *cursor = line;

    char *bytes = NULL;
    char *group = NULL;
    char *subgroup = NULL;
    char *name = NULL;
    char *keywords_str = NULL;

    // Each line in the file has this format:
    // [bytes]\t[group]\t[subgroup]\t[keywords_str]

    cursor = scan_until('\t', cursor, &bytes);
    if (bytes == NULL) {
      break;
    }
    cursor = scan_until('\t', cursor, &group);
    if (group == NULL) {
      free(bytes);
      break;
    }
    cursor = scan_until('\t', cursor, &subgroup);
    if (subgroup == NULL) {
      free(bytes);
      free(group);
      break;
    }
    cursor = scan_until('\t', cursor, &name);
    if (name == NULL) {
      free(bytes);
      free(group);
      free(subgroup);
      break;
    }
    cursor = scan_until('\n', cursor, &keywords_str);
    if (keywords_str == NULL) {
      free(bytes);
      free(group);
      free(subgroup);
      free(name);
      break;
    }

    char **keywords = g_strsplit(keywords_str, "|", -1);
    strip_strv(keywords);
    Emoji *emoji = emoji_new(bytes, name, group, subgroup, keywords);
    emoji_list_push(list, emoji);
  }

  fclose(file);

  return list;
}
