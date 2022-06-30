#ifndef UTILS_H
#define UTILS_H

// Rofi is not yet exporting these constants in their headers
// https://github.com/DaveDavenport/rofi/blob/79adae77d72be3de96d1c4e6d53b6bae4cb7e00e/include/widgets/textbox.h#L104
#define STATE_MARKUP 8

// Not exported by Rofi
void rofi_view_hide();

#include "emoji.h"

typedef enum {
  SUCCESS = 1,
  NOT_A_FILE = 0,
  CANNOT_DETERMINE_PATH = -1
} FindDataFileResult;

FindDataFileResult find_data_file(const char *basename, char **path);
int find_clipboard_adapter(char **adapter, char **error);
int run_clipboard_adapter(const char *action, const char *text, char **error);
void capitalize(char *text);

void tokenize_search(const char *input, char **query, char **group_query,
                     char **subgroup_query);

char *codepoint(char *bytes);

#endif // UTILS_H
