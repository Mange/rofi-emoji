#include <errno.h>
#include <glib.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/wait.h>
#include <unistd.h>

#include "loader.h"

FindDataFileResult find_data_file(char *basename, char **path) {
  const char *const *data_dirs = g_get_system_data_dirs();
  if (data_dirs == NULL) {
    return CANNOT_DETERMINE_PATH;
  }

  // Store first path in case file cannot be found; this path will then be the
  // path reported to the user in the error message.
  char *first_path = NULL;

  int index = 0;
  char const *data_dir = data_dirs[index];
  while (1) {
    char *current_path =
        g_build_filename(data_dir, "rofi-emoji", basename, NULL);
    if (current_path == NULL) {
      return CANNOT_DETERMINE_PATH;
    }

    if (g_file_test(current_path,
                    (G_FILE_TEST_EXISTS | G_FILE_TEST_IS_REGULAR))) {
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
    *error = g_strdup("Failed to load clipboard-adapter file: The path could "
                      "not be determined");
  } else if (result == NOT_A_FILE) {
    *error =
        g_markup_printf_escaped("Failed to load clipboard-adapter file: "
                                "<tt>%s</tt> is not a file\nAlso "
                                "searched in every path in $XDG_DATA_DIRS.",
                                *adapter);
  } else {
    *error = g_strdup("Unexpected error");
  }

  return FALSE;
}

int run_clipboard_adapter(char *action, Emoji *emoji, char **error) {
  char *adapter;
  int ca_result = find_clipboard_adapter(&adapter, error);
  if (ca_result != TRUE) {
    return FALSE;
  }

  gchar *codepoints = codepoints_string_for_utf8(emoji->bytes);

  g_autoptr(GError) child_error = NULL;
  int exit_status;

  g_spawn_sync(
      /* working_directory */ NULL,
      /* argv */
      (char *[]){"/bin/sh", adapter, action, emoji->bytes, codepoints, NULL},
      /* envp */ NULL,
      // G_SPAWN_DO_NOT_REAP_CHILD allows us to call waitpid and get the staus
      // code.
      /* flags */ (G_SPAWN_DEFAULT),
      /* child_setup */ NULL,
      /* user_data */ NULL,
      /* standard_output */ NULL,
      /* standard_error */ NULL,
      /* exit_status */ &exit_status,
      /* error */ &child_error);

  g_free(codepoints);

  if (child_error == NULL) {
    g_spawn_check_exit_status(exit_status, &child_error);
  }

  if (child_error != NULL) {
    *error = g_strdup_printf("Failed to run clipboard-adapter: %s",
                             child_error->message);
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

/*
 * Return a `gchar *` NULL-terminated list for all codepoints in the input
 * UTF-8 string in xdotool format.
 *
 * For example, for the input "🇸🇪", you get
 * {"U1F1F8", "U1F1EA", NULL} back.
 *
 * You need to `g_free` the items and the list yourself when you are done with
 * it.
 */
gchar **codepoints_for_utf8(gchar *string) {
  const char *codepoint_format = "U%04X";
  // gunichar is 32-bit, which can be represented in hex using 8 characters
  // (FFFF FFFF). With a prefix "U" and a suffix NUL byte, that gives the max
  // length of 10.
  const glong codepoint_max_len = 8 + 1 + 1;

  gchar *valid_input = g_utf8_make_valid(string, -1);
  glong len = g_utf8_strlen(valid_input, -1);
  gchar **codepoints = g_malloc((len + 1) * sizeof(gchar *));

  for (glong i = 0; i < len; ++i) {
    char *substring = g_utf8_substring(valid_input, i, i + 1);
    gunichar c = g_utf8_get_char(substring);
    g_free(substring);

    gchar *codepoint = g_malloc(codepoint_max_len);
    g_snprintf(codepoint, codepoint_max_len, codepoint_format, c);
    codepoints[i] = codepoint;
  }

  g_free(valid_input);
  codepoints[len] = NULL;
  return codepoints;
}

/*
 * Create a `gchar *` for all codepoints in the input UTF-8 string in
 * xdotool format separated with spaces.
 *
 * For example, for the input "🇸🇪", you get "U1F1 U1F1EA" back.
 *
 * You need to `g_free` the string after you are done with it.
 */
gchar *codepoints_string_for_utf8(gchar *input) {
  gchar **codepoints = codepoints_for_utf8(input);

  gchar *result = g_strjoinv(" ", codepoints);

  for (int i = 0; i < g_strv_length(codepoints); ++i) {
    if (codepoints[i] != NULL) {
      g_free(codepoints[i]);
    }
  }
  g_free(codepoints);

  return result;
}
