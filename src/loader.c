#include <glib.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "loader.h"

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

FindEmojiFileResult find_emoji_file(char **path) {
  const char * const *data_dirs = g_get_system_data_dirs();
  if (data_dirs == NULL) {
    return CANNOT_DETERMINE_PATH;
  }

  // Store first path in case file cannot be found; this path will then be the
  // path reported to the user in the error message.
  char *first_path = NULL;

  int index = 0;
  char const *data_dir = data_dirs[index];
  while (1) {
    char *current_path = g_build_filename(data_dir, "rofi-emoji", "all_emojis.txt", NULL);
    if (current_path == NULL) {
      return CANNOT_DETERMINE_PATH;
    }

    if (g_file_test(current_path, (G_FILE_TEST_EXISTS | G_FILE_TEST_IS_REGULAR))) {
      *path = current_path;
      g_free(first_path);
      return SUCCESS;
    }

    if (first_path == NULL) {
      first_path = current_path;
    } else {
      g_free(current_path);
    }

    index += 1;
    data_dir = data_dirs[index];
    if (data_dir == NULL) {
      break;
    }
  }

  *path = first_path;
  return NOT_A_FILE;
}

EmojiList *read_emojis_from_file(char *path) {
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
    char *keywords = NULL;

    // Each line in the file has this format:
    // [bytes]\t[group]\t[subgroup]\t[keywords]

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
    cursor = scan_until('\n', cursor, &keywords);
    if (keywords == NULL) {
      free(bytes);
      free(group);
      free(subgroup);
      break;
    }

    Emoji *emoji = emoji_new(bytes, keywords, group, subgroup); // freed by emoji_free_inside, in emoji_list_free
    emoji_list_push(list, emoji);
  }

  fclose(file);

  return list;
}
