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
  const char *data_dir = g_get_user_data_dir();
  if (data_dir == NULL) {
    return CANNOT_DETERMINE_PATH;
  }

  *path = g_build_filename(data_dir, "rofi-emoji", "emoji-test.txt", NULL);
  if (*path == NULL) {
    return CANNOT_DETERMINE_PATH;
  }

  if (g_file_test(*path, (G_FILE_TEST_EXISTS | G_FILE_TEST_IS_REGULAR))) {
    return SUCCESS;
  } else {
    return NOT_A_FILE;
  }
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
