#include <errno.h>
#include <glib.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/wait.h>
#include <unistd.h>

#include "loader.h"
#include "utils.h"

FindDataFileResult find_data_file(const char *basename, char **path) {
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

int run_clipboard_adapter(const char *action, const char *text, char **error) {
  char *adapter;
  int ca_result = find_clipboard_adapter(&adapter, error);
  if (ca_result != TRUE) {
    return FALSE;
  }

  GPid child_pid;
  gint child_stdin;
  int exit_status = -1;
  g_autoptr(GError) child_error = NULL;

  g_spawn_async_with_pipes(
      /* working_directory */ NULL,
      /* argv */ (char *[]){adapter, (char *)action, NULL},
      /* envp */ NULL,

      // G_SPAWN_DO_NOT_REAP_CHILD allows us to call waitpid and get the staus
      // code.
      /* flags */ (G_SPAWN_DEFAULT | G_SPAWN_DO_NOT_REAP_CHILD),

      /* child_setup */ NULL,
      /* user_data */ NULL,
      /* child_pid */ &child_pid,
      /* standard_input */ &child_stdin,
      /* standard_output */ NULL,
      /* standard_error */ NULL,
      /* error */ &child_error);

  if (child_error == NULL) {
    FILE *stdin;
    if (!(stdin = fdopen(child_stdin, "ab"))) {
      *error = g_strdup_printf("Failed to open child's stdin");
      return FALSE;
    }
    fprintf(stdin, "%s", text);
    fclose(stdin);

    pid_t res = waitpid(child_pid, &exit_status, WUNTRACED);
    if (res < 0) {
      *error = g_strdup_printf(
          "Could not wait for child process (PID %i) to close", child_pid);
      g_spawn_close_pid(child_pid);
      return FALSE;
    }
    g_spawn_close_pid(child_pid);
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
 * Strips each string inside of a null-terminated list of char*.
 *
 * The list is modified in-place.
 */
void strip_strv(char **in) {
  if (in == NULL) {
    return;
  }

  int i = 0;
  char *str = in[i];

  while (str != NULL) {
    g_strstrip(str);
    str = in[++i];
  }
}

/*
 * Makes the first ASCII character in the string uppercase by modifying it
 * in-place.
 *
 * Does nothing on NULL values or empty strings.
 */
void capitalize(char *text) {
  if (text == NULL || *text == '\0') {
    return;
  }

  text[0] = g_ascii_toupper(text[0]);
}

void append(char **dest, const char *addition) {
  char *tmp;
  if (*dest == NULL) {
    tmp = g_strdup(addition);
  } else {
    tmp = g_strconcat(*dest, addition, NULL);
  }
  g_free(*dest);
  *dest = tmp;
}

void appendn(char **dest, const char *addition, int n) {
  char *tmp;
  if (*dest == NULL) {
    tmp = g_strndup(addition, n);
  } else {
    char *copy = g_strndup(addition, n);
    tmp = g_strconcat(*dest, copy, NULL);
    g_free(copy);
  }
  g_free(*dest);
  *dest = tmp;
}

void replace(char **dest, const char *replacement) {
  g_free(*dest);
  if (replacement != NULL) {
    *dest = g_strdup(replacement);
  } else {
    *dest = NULL;
  }
}

void replacen(char **dest, const char *replacement, int n) {
  g_free(*dest);
  if (replacement != NULL) {
    *dest = g_strndup(replacement, n);
  } else {
    *dest = NULL;
  }
}

void tokenize_search(const char *input, char **query, char **group_query,
                     char **subgroup_query) {
  *query = NULL;
  *group_query = NULL;
  *subgroup_query = NULL;

  const char *current = input;

  while (*current != '\0') {
    char *index = strchr(current, ' ');

    if (index == NULL) {
      // No more spaces, so rest of input is a single word.
      switch (current[0]) {
      case '@':
        if (strlen(current) > 1) {
          replace(group_query, current + 1);
        } else {
          replace(group_query, NULL);
        }
        break;
      case '#':
        if (strlen(current) > 1) {
          replace(subgroup_query, current + 1);
        } else {
          replace(subgroup_query, NULL);
        }
        break;
      default:
        append(query, current);
      }
      break;
    }

    int length = (index - current);

    switch (current[0]) {
    case '@':
      if (length > 1) {
        replacen(group_query, current + 1, length - 1);
      } else {
        replace(group_query, NULL);
      }
      break;
    case '#':
      if (length > 1) {
        replacen(subgroup_query, current + 1, length - 1);
      } else {
        replace(subgroup_query, NULL);
      }
      break;
    default:
      // Add one extra length for the space
      appendn(query, current, length + 1);
    }

    // Skip ahead to after the space
    current = index + 1;
  }

  // Query must always be something
  if (*query == NULL) {
    *query = g_strdup("");
  }

  g_strstrip(*query);
}

char *codepoint(char *bytes) {
  int added = 0;
  GString *str = g_string_new("");

  while (bytes[0] != '\0') {
    if (added > 0) {
      g_string_append(str, " ");
    }

    gunichar c = g_utf8_get_char_validated(bytes, -1);
    if (c == -1) { // Not valid
      g_string_append(str, "U+INVALID");
    } else if (c == -2) { // Incomplete
      g_string_append(str, "U+INCOMPLETE");
    } else {
      char *formatted = g_strdup_printf("U+%04X", c);
      g_string_append(str, formatted);
      g_free(formatted);
    }
    added++;
    bytes = g_utf8_find_next_char(bytes, NULL);
  }

  return g_string_free(str, FALSE);
}
