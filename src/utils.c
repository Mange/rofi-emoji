#include <glib.h>
#include <stdio.h>
#include <stdlib.h>

#include "loader.h"

FindDataFileResult find_data_file(char *basename, char **path) {
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
    char *current_path = g_build_filename(data_dir, "rofi-emoji", basename, NULL);
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
