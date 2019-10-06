#include <errno.h>
#include <glib.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/wait.h>
#include <unistd.h>

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

int find_clipboard_adapter(char **adapter, char **error) {
  FindDataFileResult result = find_data_file("clipboard-adapter.sh", adapter);

  if (result == SUCCESS) {
    return TRUE;
  } else if (result == CANNOT_DETERMINE_PATH) {
    *error = g_strdup(
      "Failed to load clipboard-adapter file: The path could not be determined"
    );
  } else if (result == NOT_A_FILE) {
    *error = g_markup_printf_escaped(
      "Failed to load clipboard-adapter file: <tt>%s</tt> is not a file\nAlso "\
      "searched in every path in $XDG_DATA_DIRS.",
      *adapter
    );
  } else {
    *error = g_strdup("Unexpected error");
  }

  return FALSE;
}

int run_clipboard_adapter(
  char *action,
  Emoji *emoji,
  char **error
) {
  char *adapter;
  int ca_result = find_clipboard_adapter(&adapter, error);
  if (ca_result != TRUE) {
    return FALSE;
  }

  g_autoptr(GError) child_error = NULL;
  int exit_status;

  g_spawn_sync(
      /* working_directory */ NULL,
      /* argv */ (char*[]){"/bin/sh", adapter, action, emoji->bytes, NULL},
      /* envp */ NULL,
      // G_SPAWN_DO_NOT_REAP_CHILD allows us to call waitpid and get the staus code.
      /* flags */ (
        G_SPAWN_DEFAULT
      ),
      /* child_setup */ NULL,
      /* user_data */ NULL,
      /* standard_output */ NULL,
      /* standard_error */ NULL,
      /* exit_status */ &exit_status,
      /* error */ &child_error
  );

  if (child_error == NULL) {
    g_spawn_check_exit_status(exit_status, &child_error);
  }

  if (child_error != NULL) {
    *error = g_strdup_printf(
      "Failed to run clipboard-adapter: %s",
      child_error->message
    );
    return FALSE;
  }

  if (exit_status == 0) {
    *error = NULL;
    return TRUE;
  } else {
    *error = g_strdup_printf("clipboard-adapter exited with %d", exit_status);
    return FALSE;
  }
}
