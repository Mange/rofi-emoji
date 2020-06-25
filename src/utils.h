#ifndef UTILS_H
#define UTILS_H

#include "emoji.h"
#include <glib.h>

typedef enum {
  SUCCESS = 1,
  NOT_A_FILE = 0,
  CANNOT_DETERMINE_PATH = -1
} FindDataFileResult;

FindDataFileResult find_data_file(char *basename, char **path);
int find_clipboard_adapter(char **adapter, char **error);
int run_clipboard_adapter(char *action, Emoji *emoji, char **error);

gchar **codepoints_for_utf8(gchar *string);
gchar *codepoints_string_for_utf8(gchar *input);

#endif // UTILS_H
