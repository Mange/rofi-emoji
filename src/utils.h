#ifndef UTILS_H
#define UTILS_H

#include "emoji.h"

typedef enum {
  SUCCESS = 1,
  NOT_A_FILE = 0,
  CANNOT_DETERMINE_PATH = -1
} FindDataFileResult;

FindDataFileResult find_data_file(char *basename, char **path);
int find_clipboard_adapter(char **adapter, char **error);
int run_clipboard_adapter(char *action, Emoji *emoji, char **error);
void strip_strv(char **in);

#endif // UTILS_H
