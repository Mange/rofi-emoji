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
  char **error,
  int collect_stderr
) {
  char *adapter;
  int ca_result = find_clipboard_adapter(&adapter, error);
  if (ca_result != TRUE) {
    return FALSE;
  }

  GPid pid;
  g_autoptr(GError) child_error = NULL;
  gint child_stdin = -1, child_stderr = -1;

  gboolean spawn_result = g_spawn_async_with_pipes(
      /* working_directory */ NULL,
      /* argv */ (char*[]){"/bin/sh", adapter, action, NULL},
      /* envp */ NULL,
      // G_SPAWN_DO_NOT_REAP_CHILD allows us to call waitpid and get the staus code.
      /* flags */ (
        G_SPAWN_DEFAULT |
        G_SPAWN_STDOUT_TO_DEV_NULL |
        G_SPAWN_DO_NOT_REAP_CHILD |
        (collect_stderr ? 0 : G_SPAWN_STDERR_TO_DEV_NULL)
      ),
      /* child_setup */ NULL,
      /* user_data */ NULL,
      /* child_pid */ &pid,
      /* standard_input */ &child_stdin,
      /* standard_output */ NULL,
      /* standard_error */ (collect_stderr ? &child_stderr : NULL),
      /* error */ &child_error
  );

  if (child_error != NULL) {
    *error = g_strdup_printf(
      "Failed to run clipboard-adapter: %s",
      child_error->message
    );
    return FALSE;
  }

  // Write data to the child process
  write(child_stdin, emoji->bytes, strlen(emoji->bytes));
  close(child_stdin);

  if (collect_stderr) {
    GString *child_error_message = g_string_new("");
    int read_bytes;
    char buf[128];
    while ((read_bytes = read(child_stderr, buf, 128)) > 0) {
      g_string_append_len(child_error_message, buf, read_bytes);
    }
    close(child_stderr);
    *error = g_string_free(child_error_message, FALSE);
  } else if (child_stderr != -1) {
    close(child_stderr);
  }

  // Wait for it to close
  int status = 0;
  if (waitpid(pid, &status, WUNTRACED) == -1) {
    *error = g_strdup_printf("Process could not be reaped: %s", strerror(errno));
    return FALSE;
  }
  g_spawn_close_pid(pid);
  status = WEXITSTATUS(status);

  if (status == 0) {
    *error = NULL;
    return TRUE;
  } else {
    // if stderr was collected, then the *error was already set to the message
    // provided by the adapter. If we're not collecting anything, put a
    // placeholder string here instead.
    if (!collect_stderr) {
      *error = g_strdup_printf("clipboard-adapter exited with %d", status);
    }
    return FALSE;
  }
}
