#include <glib.h>
#include <stdio.h>
#include <stdlib.h>

#include "loader.h"

#define MAX_LINE_LENGTH 1024
#define GROUP_PREFIX "# group: "
#define SUBGROUP_PREFIX "# subgroup: "

char *skip_to_after(char *haystack, char* needle) {
  if (haystack != NULL) {
    char *offset = strstr(haystack, needle);
    if (offset != NULL) {
      // Offset is now at start of needle, so skip past it before returning.
      offset += strlen(needle);
      return offset;
    }
  }

  return NULL;
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
    char *current_path = g_build_filename(data_dir, "rofi-emoji", "emoji-test.txt", NULL);
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

  char *current_group = NULL;
  char *current_subgroup = NULL;

  EmojiList *list = emoji_list_new(512);

  while (fgets(line, MAX_LINE_LENGTH, file) != NULL) {
    g_strchomp(line);

    if (g_str_has_prefix(line, GROUP_PREFIX)) {
      char *group_name = line + strlen(GROUP_PREFIX);
      g_free(current_group);
      current_group = g_strdup(group_name);
    } else if (g_str_has_prefix(line, SUBGROUP_PREFIX)) {
      char *subgroup_name = line + strlen(SUBGROUP_PREFIX);
      g_free(current_subgroup);
      current_subgroup = g_strdup(subgroup_name);
    } else if (current_group != NULL && current_subgroup != NULL) {

      // Each emoji lines looks like this, but with variable content in some of
      // the sections:
      //
      //
      // 1F923    ; fully-qualified     # 🤣 rolling on the floor laughing
      // ──[skip]───────────────────^─────^
      // [human-readable bytes] ; [qualification] # [bytes] [name]
      //
      // Skipping ahead to the "fully-qualified" bit, then skip all the spaces
      // until where the actual emoji bytes start. Only fully-qualified emojis
      // should be found so it is an excellent marker to use.
      char *emoji_str = skip_to_after(line, "; fully-qualified ");
      emoji_str = skip_to_after(emoji_str, "# ");
      // If NULL, then the line did not match this format. Skip to next line.
      if (emoji_str == NULL) { continue; }

      // Just some basic sanity checking; if the remaning bytes are too short
      // then something is wrong. Skip line.
      if (strlen(emoji_str) < 5) { continue; }

      // Find the location of the space between the emoji bytes and the name.
      char *space = strchr(emoji_str, ' ');
      if (space == NULL) { continue; }

      // Length of emoji bytes
      size_t bytes_len = (space - emoji_str);
      char *bytes = g_strndup(emoji_str, bytes_len); // freed by emoji_free_inside

      // Remaining text on this line is the name, prefixed with the space. Make
      // sure that we don't skip past the end of the string when we skip past
      // the space.
      if (strlen(space) < 2) {
        g_free(bytes);
        continue;
      }
      char *name = g_strdup(space + 1); // freed by emoji_free_inside

      char *group = g_strdup(current_group); // freed by emoji_free_inside
      char *subgroup = g_strdup(current_subgroup); // freed by emoji_free_inside

      Emoji *emoji = emoji_new(bytes, name, group, subgroup); // freed by emoji_free_inside, in emoji_list_free
      emoji_list_push(list, emoji);
    }
  }


  g_free(current_group);
  g_free(current_subgroup);
  fclose(file);

  return list;
}
