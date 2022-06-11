#ifndef UTILS_H
#define UTILS_H

#include "emoji.h"

typedef enum {
  SUCCESS = 1,
  NOT_A_FILE = 0,
  CANNOT_DETERMINE_PATH = -1
} FindDataFileResult;

FindDataFileResult find_data_file(const char *basename, char **path);
int find_clipboard_adapter(char **adapter, char **error);
int run_clipboard_adapter(char *action, const Emoji *emoji, char **error);
void strip_strv(char **in);
void capitalize(char *text);
void capitalize_v(char **text);

#endif // UTILS_H
