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

#include "emoji.h"
#include "emoji_list.h"

#define MAX_LINE_LENGTH 1024
#define GROUP_PREFIX "# group: "
#define SUBGROUP_PREFIX "# subgroup: "

G_MODULE_EXPORT Mode mode;

typedef struct {
  EmojiList *emojis;
  char **matcher_strings;
  char *message;
} EmojiModePrivateData;

char *skip_to_after(char *haystack, char* needle) {
  if (haystack != NULL) {
    char *offset = strstr(haystack, needle);
    if (offset != NULL) {
      // Offset is now at start of needle, so skip past it before returning.
      offset += strlen(needle);
      return offset;
    }
  }

  return NULL;
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
    g_strchomp(line);

    if (g_str_has_prefix(line, GROUP_PREFIX)) {
      char *group_name = line + strlen(GROUP_PREFIX);
      g_free(current_group);
      current_group = g_strdup(group_name);
    } else if (g_str_has_prefix(line, SUBGROUP_PREFIX)) {
      char *subgroup_name = line + strlen(SUBGROUP_PREFIX);
      g_free(current_subgroup);
      current_subgroup = g_strdup(subgroup_name);
    } else if (current_group != NULL && current_subgroup != NULL) {

      // Each emoji lines looks like this, but with variable content in some of
      // the sections:
      //
      //
      // 1F923    ; fully-qualified     # 🤣 rolling on the floor laughing
      // ──[skip]───────────────────^─────^
      // [human-readable bytes] ; [qualification] # [bytes] [name]
      //
      // Skipping ahead to the "fully-qualified" bit, then skip all the spaces
      // until where the actual emoji bytes start. Only fully-qualified emojis
      // should be found so it is an excellent marker to use.
      char *emoji_str = skip_to_after(line, "; fully-qualified ");
      emoji_str = skip_to_after(emoji_str, "# ");
      // If NULL, then the line did not match this format. Skip to next line.
      if (emoji_str == NULL) { continue; }

      // Just some basic sanity checking; if the remaning bytes are too short
      // then something is wrong. Skip line.
      if (strlen(emoji_str) < 5) { continue; }

      // Find the location of the space between the emoji bytes and the name.
      char *space = strchr(emoji_str, ' ');
      if (space == NULL) { continue; }

      // Length of emoji bytes
      size_t bytes_len = (space - emoji_str);
      char *bytes = g_strndup(emoji_str, bytes_len); // freed by emoji_free_inside

      // Remaining text on this line is the name, prefixed with the space. Make
      // sure that we don't skip past the end of the string when we skip past
      // the space.
      if (strlen(space) < 2) {
        g_free(bytes);
        continue;
      }
      char *name = g_strdup(space + 1); // freed by emoji_free_inside

      char *group = g_strdup(current_group); // freed by emoji_free_inside
      char *subgroup = g_strdup(current_subgroup); // freed by emoji_free_inside

      Emoji *emoji = emoji_new(bytes, name, group, subgroup); // freed by emoji_free_inside, in emoji_list_free
      emoji_list_push(list, emoji);
    }
  }


  g_free(current_group);
  g_free(current_subgroup);
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
    return FALSE;
  }

  char *cmd = g_strdup_printf("\"%s\" --clipboard --input", xsel);
  free(xsel);

  FILE *stream = popen(cmd, "w");
  free(cmd);

  if (stream == NULL) {
    *error = g_strdup_printf("Failed to start xsel process: %s", strerror(errno));
    return FALSE;
  }

  fwrite(emoji->bytes, strlen(emoji->bytes), 1, stream);
  int status = pclose(stream);
  if (status == 0) {
    *error = NULL;
    return TRUE;
  } else if (status == -1) {
    *error = g_strdup_printf("Failed to run xsel: %s", strerror(errno));
    return FALSE;
  } else {
    *error = g_strdup_printf("xsel did not complete successfully (exit code %d)", status);
    return FALSE;
  }
}

char **generate_matcher_strings(EmojiList *list) {
  char **strings = g_new(char*, list->length);
  for (int i = 0; i < list->length; ++i) {
    Emoji *emoji = emoji_list_get(list, i);
    strings[i] = g_strdup_printf(
      "%s %s %s / %s",
      emoji->bytes, emoji->name, emoji->group, emoji->subgroup
    );
  }
  return strings;
}

static void get_emoji( Mode *sw) {
  EmojiModePrivateData *pd = (EmojiModePrivateData *) mode_get_private_data(sw);
  char *path;

  find_emoji_file_result result = find_emoji_file(&path);
  if (result == SUCCESS) {
    pd->emojis = read_emojis_from_file(path);
    pd->matcher_strings = generate_matcher_strings(pd->emojis);
  } else {
    if (result == CANNOT_DETERMINE_PATH) {
      pd->message = g_strdup("Failed to load emoji file: The path could not be determined");
    } else if (result == NOT_A_FILE) {
      pd->message = g_markup_printf_escaped(
        "Failed to load emoji file: <tt>%s</tt> is not a file\nThis will be fixed automatically in the future.",
        path
      );
    }
    pd->emojis = emoji_list_new(0);
    pd->matcher_strings = NULL;
  }
}

/**
 * @param mode The mode to initialize
 *
 * Initialize mode
 *
 * @returns FALSE if there was a failure, TRUE if successful
 */
static int emoji_mode_init(Mode *sw) {
  if (mode_get_private_data(sw) == NULL) {
    EmojiModePrivateData *pd = g_malloc0(sizeof(*pd));
    mode_set_private_data(sw, (void *)pd);
    get_emoji(sw);
  }
  return TRUE;
}

/**
 * @param mode The mode to query
 *
 * Get the number of entries in the mode.
 *
 * @returns an unsigned in with the number of entries.
 */
static unsigned int emoji_mode_get_num_entries(const Mode *sw) {
  const EmojiModePrivateData *pd = (const EmojiModePrivateData *) mode_get_private_data(sw);
  return pd->emojis->length;
}

/**
 * @param mode The mode to query
 * @param menu_retv The menu return value.
 * @param input Pointer to the user input string. [in][out]
 * @param selected_line the line selected by the user.
 *
 * Acts on the user interaction.
 *
 * @returns the next #ModeMode.
 */
static ModeMode emoji_mode_result(Mode *sw, int mretv, char **input, unsigned int selected_line) {
  ModeMode retv = MODE_EXIT;
  EmojiModePrivateData *pd = (EmojiModePrivateData *) mode_get_private_data(sw);

  if (mretv & MENU_NEXT) {
    retv = NEXT_DIALOG;
  } else if (mretv & MENU_PREVIOUS) {
    retv = PREVIOUS_DIALOG;
  } else if (mretv & MENU_QUICK_SWITCH) {
    retv = (mretv & MENU_LOWER_MASK);
  } else if ((mretv & MENU_OK) ) {
    int result = copy_emoji_to_clipboard(emoji_list_get(pd->emojis, selected_line), &(pd->message));
    if (result) {
      retv = MODE_EXIT;
    } else {
      retv = RELOAD_DIALOG;
    }
  } else if ((mretv & MENU_ENTRY_DELETE) == MENU_ENTRY_DELETE) {
    retv = RELOAD_DIALOG;
  }
  return retv;
}

/**
 * @param mode The mode to destroy
 *
 * Destroy the mode
 */
static void emoji_mode_destroy(Mode *sw) {
  EmojiModePrivateData *pd = (EmojiModePrivateData *) mode_get_private_data(sw);
  if (pd != NULL) {
    // Free all generated matcher strings before freeing the list.
    for (int i = 0; i < pd->emojis->length; ++i) {
      g_free(pd->matcher_strings[i]);
    }
    emoji_list_free(pd->emojis);
    g_free(pd->message);
    g_free(pd);
    mode_set_private_data(sw, NULL);
  }
}

/**
 * @param mode The mode to query
 *
 * Query the mode for a user display.
 *
 * @return a new allocated (valid pango markup) message to display (user should free).
 */
static char *emoji_get_message(const Mode *sw) {
  EmojiModePrivateData *pd = (EmojiModePrivateData *) mode_get_private_data(sw);
  return g_strdup(pd->message);
}

/**
 * @param mode The mode to query
 * @param selected_line The entry to query
 * @param state The state of the entry [out]
 * @param attribute_list List of extra (pango) attribute to apply when displaying. [out][null]
 * @param get_entry If the should be returned.
 *
 * Returns the string as it should be displayed for the entry and the state of how it should be displayed.
 *
 * @returns allocated new string and state when get_entry is TRUE otherwise just the state.
 */
static char *get_display_value(const Mode *sw, unsigned int selected_line, G_GNUC_UNUSED int *state, G_GNUC_UNUSED GList **attr_list, int get_entry) {
  EmojiModePrivateData *pd = (EmojiModePrivateData *) mode_get_private_data(sw);

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
static int emoji_token_match(const Mode *sw, rofi_int_matcher **tokens, unsigned int index) {
  EmojiModePrivateData *pd = (EmojiModePrivateData *) mode_get_private_data(sw);
  return index < pd->emojis->length && helper_token_match(tokens, pd->matcher_strings[index]);
}


Mode mode = {
  .abi_version        = ABI_VERSION,
  .name               = "emoji",
  .cfg_name_key       = "emoji",
  ._init              = emoji_mode_init,
  ._get_num_entries   = emoji_mode_get_num_entries,
  ._result            = emoji_mode_result,
  ._destroy           = emoji_mode_destroy,
  ._token_match       = emoji_token_match,
  ._get_display_value = get_display_value,
  ._get_message       = emoji_get_message,
  ._get_completion    = NULL,
  ._preprocess_input  = NULL,
  .private_data       = NULL,
  .free               = NULL,
};
