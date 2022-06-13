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
const char *scan_until(const char until, const char *input, char **result) {
  char *index = strchr(input, until);

  if (index == NULL) {
    *result = NULL;
    return input;
  }

  int length = (index - input);
  *result = g_strndup(input, length);

  // Advance input to character after the `until` character.
  return index + 1;
}

void array_emoji_free_item(gpointer item) { emoji_free(item); }

GPtrArray *read_emojis_from_file(const char *path) {
  FILE *file = fopen(path, "r");
  if (!file) {
    return NULL;
  }

  char line[MAX_LINE_LENGTH];

  GPtrArray *list = g_ptr_array_sized_new(512);
  g_ptr_array_set_free_func(list, array_emoji_free_item);

  while (fgets(line, MAX_LINE_LENGTH, file) != NULL) {
    Emoji *emoji = parse_emoji_from_line(line);
    if (emoji == NULL) {
      break;
    }
    g_ptr_array_add(list, emoji);
  }

  fclose(file);

  return list;
}

void cleanup(char *str) {
  g_strstrip(str);
  capitalize(str);
}

Emoji *parse_emoji_from_line(const char *line) {
  const char *cursor = line;

  char *bytes = NULL;
  char *group = NULL;
  char *subgroup = NULL;
  char *name = NULL;
  char *keywords_str = NULL;

  // Each line in the file has this format:
  // [bytes]\t[group]\t[subgroup]\t[keywords_str]

  cursor = scan_until('\t', cursor, &bytes);
  if (bytes == NULL) {
    return NULL;
  }
  cursor = scan_until('\t', cursor, &group);
  if (group == NULL) {
    g_free(bytes);
    return NULL;
  }
  cursor = scan_until('\t', cursor, &subgroup);
  if (subgroup == NULL) {
    g_free(bytes);
    g_free(group);
    return NULL;
  }
  cursor = scan_until('\t', cursor, &name);
  if (name == NULL) {
    g_free(bytes);
    g_free(group);
    g_free(subgroup);
    return NULL;
  }
  cursor = scan_until('\n', cursor, &keywords_str);
  if (keywords_str == NULL) {
    g_free(bytes);
    g_free(group);
    g_free(subgroup);
    g_free(name);
    return NULL;
  }

  char **keywords = g_strsplit(keywords_str, "|", -1);

  g_strstrip(bytes);
  cleanup(name);
  cleanup(group);
  cleanup(subgroup);
  strip_strv(keywords);
  capitalize_v(keywords);

  Emoji *emoji = emoji_new(bytes, name, group, subgroup, keywords);
  return emoji;
}
