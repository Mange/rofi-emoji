#include <errno.h>
#include <glib.h>
#include <gmodule.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include <rofi/mode.h>
#include <rofi/helper.h>
#include <rofi/mode-private.h>

#include <stdint.h>

#define MAX_LINE_LENGTH 1024
G_MODULE_EXPORT Mode mode;

typedef struct Emoji {
  char *bytes;
  char *name;
  char *group;
  char *subgroup;
} Emoji;

typedef struct EmojiList {
  int capacity;
  int length;
  Emoji *emojis;
} EmojiList;

typedef struct {
    EmojiList *emojis;
    char **matcher_strings;
    char *message;
} EmojiModePrivateData;

Emoji *emoji_new(char *bytes, char *name, char* group, char *subgroup) {
    Emoji *emoji = malloc(sizeof(Emoji));
    emoji->bytes = bytes;
    emoji->name = name;
    emoji->group = group;
    emoji->subgroup = subgroup;
    return emoji;
}

void emoji_free(Emoji *emoji) {
  free(emoji->bytes);
  free(emoji->name);
  free(emoji->group);
  free(emoji->subgroup);
}

EmojiList *emoji_list_new(int capacity) {
  EmojiList *list = malloc(sizeof(EmojiList));
  if (list == NULL) {
    return NULL;
  }

  list->capacity = capacity;
  list->length = 0;
  list->emojis = NULL;

  if (capacity > 0) {
    Emoji *buf = malloc(sizeof(Emoji) * capacity);
    if (buf == NULL) {
      free(list);
      return NULL;
    } else {
      list->emojis = buf;
    }
  }

  return list;
}

int emoji_list_resize(EmojiList *self, int capacity) {
  if (capacity >= self->capacity) {
    Emoji* new_buf = malloc(sizeof(Emoji) * capacity);
    memcpy(new_buf, self->emojis, sizeof(Emoji) * self->length);
    free(self->emojis);
    self->emojis = new_buf;
    self->capacity = capacity;
    return 1;
  } else {
    printf("emoji_list_resize does not support shrinking\n");
    exit(127);
  }
}


int emoji_list_push(EmojiList *self, Emoji* emoji) {
  if (self->length == self->capacity) {
    int result = emoji_list_resize(self, self->capacity * 2);
    if (!result) {
      return 0;
    }
  }

  self->emojis[self->length] = *emoji;
  free(emoji);
  self->length += 1;
  return 1;
}

Emoji *emoji_list_get(EmojiList *self, unsigned int index) {
  if (self->length > index) {
    return &(self->emojis[index]);
  } else {
    return NULL;
  }
}

void emoji_list_free(EmojiList *self) {
  if (self == NULL) {
    return;
  }

  for (int i = 0; i < self->length; ++i) {
    emoji_free(&self->emojis[i]);
  }

  free(self->emojis);
  self->emojis = NULL;
  self->length = 0;
  self->capacity = 0;
  free(self);
}

int line_starts_with(char *line, char *prefix) {
  size_t prefix_len = strlen(prefix);
  if (strlen(line) < prefix_len) {
    return 0;
  } else {
    return strncmp(prefix, line, prefix_len) == 0;
  }
}

void strip_trailing_newline(char *line) {
  // Remove trailing newline by overwriting it with NUL
  char *newline_pos;
  if ((newline_pos = strchr(line, '\n')) != NULL) {
    *newline_pos = '\0';
  }
}

char *skip_to_after(char *haystack, char* needle) {
  char *offset;
  if ((offset = strstr(haystack, needle)) != NULL) {
    // Skip past the needle
    offset += strlen(needle);
    return offset;
  } else {
    return NULL;
  }
}

void debug_emoji_list(EmojiList *list) {
  if (list == NULL) {
    printf("debug_emoji_list: Passed NULL\n");
  } else if (list->length == 0) {
    printf("debug_emoji_list: capacity=%5d, length=%5d\n", list->capacity, list->length);
  } else {
    Emoji first = list->emojis[0];
    Emoji last = list->emojis[list->length - 1];
    printf("debug_emoji_list: capacity=%5d, length=%5d. First=%s, Last=%s\n", list->capacity, list->length, first.bytes, last.bytes);
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
    strip_trailing_newline(line);

    if (line_starts_with(line, "# group: ")) {
      char *group_name = line + strlen("# group: ");

      // Allocate storage for group name
      free(current_group);
      size_t len = strlen(group_name);
      current_group = malloc(len + 1);
      strcpy(current_group, group_name);
    } else if (line_starts_with(line, "# subgroup: ")) {
      char *subgroup_name = line + strlen("# subgroup: ");

      // Allocate storage for group name
      free(current_subgroup);
      size_t len = strlen(subgroup_name);
      current_subgroup = malloc(len + 1);
      strcpy(current_subgroup, subgroup_name);
    } else if (current_group != NULL && current_subgroup != NULL) {
      char *emoji_str = skip_to_after(line, "; fully-qualified ");
      if (emoji_str == NULL) { continue; }
      emoji_str = skip_to_after(emoji_str, "# ");
      if (emoji_str == NULL) { continue; }

      // Just some basic sanity checking
      if (strlen(emoji_str) < 5) { continue; }

      char *space = strchr(emoji_str, ' ');
      if (space == NULL) { continue; }

      size_t bytes_len = (space - emoji_str);
      char *bytes = malloc(bytes_len + 1); // freed by emoji_free
      strncpy(bytes, emoji_str, bytes_len);
      bytes[bytes_len] = '\0';

      size_t name_len = strlen(space);
      char *name = malloc(name_len + 1); // freed by emoji_free
      strncpy(name, space + 1, name_len);
      name[name_len] = '\0';

      char *group = malloc(strlen(current_group) + 1); // freed by emoji_free
      char *subgroup = malloc(strlen(current_subgroup) + 1); // freed by emoji_free
      strcpy(group, current_group);
      strcpy(subgroup, current_subgroup);

      Emoji *emoji = emoji_new(bytes, name, group, subgroup); // freed by emoji_free, in emoji_list_free
      emoji_list_push(list, emoji);
    }
  }


  free(current_group);
  free(current_subgroup);
  fclose(file);

  return list;
}

typedef enum {
  SUCCESS = 1,
  NOT_A_FILE = 0,
  CANNOT_DETERMINE_PATH = -1
} find_emoji_file_result;

find_emoji_file_result find_emoji_file(char **path) {
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

int copy_emoji_to_clipboard(Emoji *emoji, char **error) {
  char *xsel = g_find_program_in_path("xsel");
  if (xsel == NULL) {
    *error = g_strdup("Failed to run xsel: Cannot find xsel in PATH. Have you installed xsel?");
    return 0;
  }

  char *cmd = g_strdup_printf("\"%s\" --clipboard --input", xsel);
  free(xsel);

  FILE *stream = popen(cmd, "w");
  free(cmd);

  if (stream == NULL) {
    *error = g_strdup_printf("Failed to start xsel process: %s", strerror(errno));
    return 0;
  }

  fwrite(emoji->bytes, strlen(emoji->bytes), 1, stream);
  int status = pclose(stream);
  if (status == 0) {
    *error = NULL;
    return 1;
  } else if (status == -1) {
    *error = g_strdup_printf("Failed to run xsel: %s", strerror(errno));
    return 0;
  } else {
    *error = g_strdup_printf("xsel did not complete successfully (exit code %d)", status);
    return 0;
  }
}


static void get_emoji (  Mode *sw )
{
    EmojiModePrivateData *pd = (EmojiModePrivateData *) mode_get_private_data ( sw );
    char *path;

    find_emoji_file_result result = find_emoji_file(&path);
    if (result == SUCCESS) {
      pd->emojis = read_emojis_from_file(path);
      pd->matcher_strings = malloc(sizeof(char*) * pd->emojis->length);
      for (int i = 0; i < pd->emojis->length; ++i) {
        Emoji *emoji = emoji_list_get(pd->emojis, i);
        pd->matcher_strings[i] = g_strdup_printf(
            "%s %s %s / %s",
            emoji->bytes, emoji->name, emoji->group, emoji->subgroup
        );
      }
    } else {
      if (result == CANNOT_DETERMINE_PATH) {
        pd->message = g_strdup("Failed to load emoji file: The path could not be determined");
      } else if (result == NOT_A_FILE) {
        pd->message = g_markup_printf_escaped("Failed to load emoji file: <tt>%s</tt> is not a file\nThis will be fixed automatically in the future.", path);
      }
      pd->emojis = emoji_list_new(0);
      pd->matcher_strings = NULL;
    }
}


static int emoji_mode_init ( Mode *sw )
{
    /**
     * Called on startup when enabled (in modi list)
     */
    if ( mode_get_private_data ( sw ) == NULL ) {
        EmojiModePrivateData *pd = g_malloc0 ( sizeof ( *pd ) );
        pd->message = NULL;
        mode_set_private_data ( sw, (void *) pd );
        get_emoji ( sw );
    }
    return TRUE;
}
static unsigned int emoji_mode_get_num_entries ( const Mode *sw )
{
    const EmojiModePrivateData *pd = (const EmojiModePrivateData *) mode_get_private_data ( sw );
    return pd->emojis->length;
}

static ModeMode emoji_mode_result ( Mode *sw, int mretv, char **input, unsigned int selected_line )
{
    ModeMode           retv  = MODE_EXIT;
    EmojiModePrivateData *pd = (EmojiModePrivateData *) mode_get_private_data ( sw );
    if ( mretv & MENU_NEXT ) {
        retv = NEXT_DIALOG;
    } else if ( mretv & MENU_PREVIOUS ) {
        retv = PREVIOUS_DIALOG;
    } else if ( mretv & MENU_QUICK_SWITCH ) {
        retv = ( mretv & MENU_LOWER_MASK );
    } else if ( ( mretv & MENU_OK ) ) {
        int result = copy_emoji_to_clipboard(emoji_list_get(pd->emojis, selected_line), &(pd->message));
        if (result) {
          retv = MODE_EXIT;
        } else {
          retv = RELOAD_DIALOG;
        }
    } else if ( ( mretv & MENU_ENTRY_DELETE ) == MENU_ENTRY_DELETE ) {
        retv = RELOAD_DIALOG;
    }
    return retv;
}

static void emoji_mode_destroy ( Mode *sw )
{
    EmojiModePrivateData *pd = (EmojiModePrivateData *) mode_get_private_data ( sw );
    if ( pd != NULL ) {
        for (int i = 0; i < pd->emojis->length; ++i) {
          g_free(pd->matcher_strings[i]);
        }
        emoji_list_free(pd->emojis);
        g_free(pd->message);
        g_free(pd);
        mode_set_private_data ( sw, NULL );
    }
}

static char *emoji_get_message (const Mode *sw) {
    EmojiModePrivateData *pd = (EmojiModePrivateData *) mode_get_private_data ( sw );
    return g_strdup(pd->message);
}

static char *_get_display_value ( const Mode *sw, unsigned int selected_line, G_GNUC_UNUSED int *state, G_GNUC_UNUSED GList **attr_list, int get_entry )
{
    EmojiModePrivateData *pd = (EmojiModePrivateData *) mode_get_private_data ( sw );

    // Rofi is not yet exporting these constants in their headers
    // *state |= MARKUP;
    // https://github.com/DaveDavenport/rofi/blob/79adae77d72be3de96d1c4e6d53b6bae4cb7e00e/include/widgets/textbox.h#L104
    *state |= 8;

    // Only return the string if requested, otherwise only set state.
    if (!get_entry) {
      return NULL;
    }

    Emoji* emoji = emoji_list_get(pd->emojis, selected_line);

    if (emoji == NULL) {
      return g_strdup("n/a");
    } else {
      return g_markup_printf_escaped(
          "%s %s <span size='small'>(%s / %s)</span>",
          emoji->bytes, emoji->name, emoji->group, emoji->subgroup
      );
    }
}

/**
 * @param sw The mode object.
 * @param tokens The tokens to match against.
 * @param index  The index in this plugin to match against.
 *
 * Match the entry.
 *
 * @param returns try when a match.
 */
static int emoji_token_match ( const Mode *sw, rofi_int_matcher **tokens, unsigned int index )
{
    EmojiModePrivateData *pd = (EmojiModePrivateData *) mode_get_private_data ( sw );
    return index < pd->emojis->length && helper_token_match(tokens, pd->matcher_strings[index]);
}


Mode mode =
{
    .abi_version        = ABI_VERSION,
    .name               = "emoji",
    .cfg_name_key       = "emoji",
    ._init              = emoji_mode_init,
    ._get_num_entries   = emoji_mode_get_num_entries,
    ._result            = emoji_mode_result,
    ._destroy           = emoji_mode_destroy,
    ._token_match       = emoji_token_match,
    ._get_display_value = _get_display_value,
    ._get_message       = emoji_get_message,
    ._get_completion    = NULL,
    ._preprocess_input  = NULL,
    .private_data       = NULL,
    .free               = NULL,
};
