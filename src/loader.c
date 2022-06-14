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

int scan_line(const char *line, char **bytes, char **name, char **group,
              char **subgroup, char **keywords) {
  *bytes = NULL;
  *group = NULL;
  *subgroup = NULL;
  *name = NULL;
  *keywords = NULL;

  const char *cursor = line;

  // Each line in the file has this format:
  // [bytes]\t[group]\t[subgroup]\t[keywords_str]

  cursor = scan_until('\t', cursor, bytes);
  if (*bytes == NULL) {
    return 0;
  }
  cursor = scan_until('\t', cursor, group);
  if (*group == NULL) {
    g_free(*bytes);
    return 0;
  }
  cursor = scan_until('\t', cursor, subgroup);
  if (*subgroup == NULL) {
    g_free(*bytes);
    g_free(*group);
    return 0;
  }
  cursor = scan_until('\t', cursor, name);
  if (*name == NULL) {
    g_free(*bytes);
    g_free(*group);
    g_free(*subgroup);
    return 0;
  }
  cursor = scan_until('\n', cursor, keywords);
  if (*keywords == NULL) {
    g_free(*bytes);
    g_free(*group);
    g_free(*subgroup);
    g_free(*name);
    return 0;
  }

  return 1;
}

char **build_keyword_list(const char *keywords_str, const char *name) {
  // Build keyword list. Skip entries that are identical to the name as they
  // will just be redundant.
  char *name_casefold = g_utf8_casefold(name, -1);
  GPtrArray *kw_array = g_ptr_array_new();
  char **keywords;

  keywords = g_strsplit(keywords_str, "|", -1);

  for (int i = 0; keywords[i] != NULL; i++) {
    char *keyword = keywords[i];
    cleanup(keyword);
    char *keyword_casefold = g_utf8_casefold(keyword, -1);

    if (strcmp(name_casefold, keyword_casefold) != 0) {
      g_ptr_array_add(kw_array, g_strdup(keyword));
    }

    g_free(keyword_casefold);
  }

  // Original keywords can now be freed and replaced with the built list (which
  // is cleaned up and has no keywords equal to the name).
  g_strfreev(keywords);

  keywords = g_new(char *, kw_array->len + 1);
  for (int i = 0; i < kw_array->len; i++) {
    keywords[i] = g_strdup(g_ptr_array_index(kw_array, i));
  }
  keywords[kw_array->len] = NULL;

  g_ptr_array_free(kw_array, TRUE);
  g_free(name_casefold);

  return keywords;
}

Emoji *parse_emoji_from_line(const char *line) {
  const char *cursor = line;

  char *bytes = NULL;
  char *group = NULL;
  char *subgroup = NULL;
  char *name = NULL;
  char *keywords_str = NULL;

  if (!scan_line(cursor, &bytes, &name, &group, &subgroup, &keywords_str)) {
    return NULL;
  }

  g_strstrip(bytes);
  cleanup(name);
  cleanup(group);
  cleanup(subgroup);

  char **keywords = build_keyword_list(keywords_str, name);

  Emoji *emoji = emoji_new(bytes, name, group, subgroup, keywords);
  return emoji;
}
