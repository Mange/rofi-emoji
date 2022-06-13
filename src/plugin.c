#include <glib.h>

// Must be included before other rofi includes.
#include <rofi/mode.h>

#include <rofi/helper.h>
#include <rofi/mode-private.h>

#include "emoji.h"
#include "emoji_list.h"
#include "formatter.h"
#include "loader.h"

G_MODULE_EXPORT Mode mode;

typedef struct {
  EmojiList *emojis;
  char **matcher_strings;
  char *message;
  char *format;
} EmojiModePrivateData;

const char *DEFAULT_FORMAT = "{emoji} <span weight='bold'>{name}</span>"
                             "[ <span size='small'>({keywords})</span>]";

// Execute the clipboard adapter with the "copy" action so the selected emoji
// is copied to the users' clipboard.
//
// Returns TRUE on success, or return FALSE on error after setting the provided
// error buffer to a user error message.
int copy_emoji(Emoji *emoji, char **error) {
  return run_clipboard_adapter("copy", emoji, error);
}

char **generate_matcher_strings(EmojiList *list) {
  char **strings = g_new(char *, list->length);
  for (int i = 0; i < list->length; ++i) {
    Emoji *emoji = emoji_list_get(list, i);
    strings[i] =
        format_emoji(emoji, "{emoji} {name} {keywords} {group} {subgroup}");
  }
  return strings;
}

/*
 * Try to find the location of the emoji file by looking at command line
 * arguments and then falling back to the default filename in the XDG data
 * directories.
 */
FindDataFileResult find_emoji_file(char **path) {
  if (find_arg("-emoji-file") >= 0) {
    if (find_arg_str("-emoji-file", path)) {
      if (g_file_test(*path, G_FILE_TEST_EXISTS | G_FILE_TEST_IS_REGULAR)) {
        return SUCCESS;
      } else {
        return NOT_A_FILE;
      }
    } else {
      (*path) = NULL;
      return CANNOT_DETERMINE_PATH;
    }
  } else {
    return find_data_file("all_emojis.txt", path);
  }
}

static void get_emoji(Mode *sw) {
  EmojiModePrivateData *pd = (EmojiModePrivateData *)mode_get_private_data(sw);
  char *path;

  FindDataFileResult result = find_emoji_file(&path);
  if (result == SUCCESS) {
    pd->emojis = read_emojis_from_file(path);
    pd->matcher_strings = generate_matcher_strings(pd->emojis);
  } else {
    if (result == CANNOT_DETERMINE_PATH) {
      pd->message = g_strdup(
          "Failed to load emoji file: The path could not be determined");
    } else if (result == NOT_A_FILE) {
      pd->message = g_markup_printf_escaped(
          "Failed to load emoji file: <tt>%s</tt> is not a file", path);
    }
    pd->emojis = emoji_list_new(0);
    pd->matcher_strings = NULL;
  }
}

/**
 * Initialize mode
 *
 * @param mode The mode to initialize
 *
 * @returns FALSE if there was a failure, TRUE if successful
 */
static int emoji_mode_init(Mode *sw) {
  if (mode_get_private_data(sw) == NULL) {
    EmojiModePrivateData *pd = g_malloc0(sizeof(*pd));

    pd->emojis = NULL;
    pd->matcher_strings = NULL;
    pd->message = NULL;
    pd->format = NULL;

    if (find_arg("-emoji-format")) {
      char *format;
      if (find_arg_str("-emoji-format", &format)) {
        // We want ownership of this data and not rely on a reference to global
        // data.
        pd->format = g_strdup(format);
      }
    }

    mode_set_private_data(sw, (void *)pd);
    get_emoji(sw);
  }
  return TRUE;
}

/**
 * Get the number of entries in the mode.
 *
 * @param sw The mode to query
 *
 * @returns an unsigned in with the number of entries.
 */
static unsigned int emoji_mode_get_num_entries(const Mode *sw) {
  const EmojiModePrivateData *pd =
      (const EmojiModePrivateData *)mode_get_private_data(sw);
  return pd->emojis->length;
}

/**
 * Acts on the user interaction.
 *
 * @param sw The mode to query
 * @param menu_retv The menu return value.
 * @param input Pointer to the user input string. [in][out]
 * @param selected_line the line selected by the user.
 *
 * @returns the next #ModeMode.
 */
static ModeMode emoji_mode_result(Mode *sw, int mretv, char **input,
                                  unsigned int selected_line) {
  ModeMode retv = MODE_EXIT;
  EmojiModePrivateData *pd = (EmojiModePrivateData *)mode_get_private_data(sw);

  if (mretv & MENU_NEXT) {
    retv = NEXT_DIALOG;
  } else if (mretv & MENU_PREVIOUS) {
    retv = PREVIOUS_DIALOG;
  } else if (mretv & MENU_QUICK_SWITCH) {
    retv = (mretv & MENU_LOWER_MASK);
  } else if ((mretv & MENU_OK)) {
    Emoji *emoji = emoji_list_get(pd->emojis, selected_line);

    if (copy_emoji(emoji, &(pd->message))) {
      retv = MODE_EXIT;
    } else {
      // Copying failed, reload dialog to show error message in pd->message.
      retv = RELOAD_DIALOG;
    }
  } else if ((mretv & MENU_ENTRY_DELETE) == MENU_ENTRY_DELETE) {
    retv = RELOAD_DIALOG;
  }
  return retv;
}

/**
 * Destroy the mode
 *
 * @param sw The mode to destroy
 *
 */
static void emoji_mode_destroy(Mode *sw) {
  EmojiModePrivateData *pd = (EmojiModePrivateData *)mode_get_private_data(sw);
  if (pd != NULL) {
    // Free all generated matcher strings before freeing the list.
    for (int i = 0; i < pd->emojis->length; ++i) {
      g_free(pd->matcher_strings[i]);
    }
    emoji_list_free(pd->emojis);
    g_free(pd->message);
    g_free(pd->format);
    g_free(pd);
    mode_set_private_data(sw, NULL);
  }
}

/**
 * Query the mode for a user display.
 *
 * @param sw The mode to query
 *
 * @return a new allocated (valid pango markup) message to display (user should
 * free).
 */
static char *emoji_get_message(const Mode *sw) {
  EmojiModePrivateData *pd = (EmojiModePrivateData *)mode_get_private_data(sw);
  return g_strdup(pd->message);
}

/**
 * Returns the string as it should be displayed for the entry and the state of
 * how it should be displayed.
 * @param sw The mode to query
 * @param selected_line The entry to query
 * @param state The state of the entry [out]
 * @param attribute_list List of extra (pango) attribute to apply when
 * displaying. [out][null]
 * @param get_entry If the should be returned.
 *
 * @returns allocated new string and state when get_entry is TRUE otherwise just
 * the state.
 */
static char *get_display_value(const Mode *sw, unsigned int selected_line,
                               G_GNUC_UNUSED int *state,
                               G_GNUC_UNUSED GList **attr_list, int get_entry) {
  EmojiModePrivateData *pd = (EmojiModePrivateData *)mode_get_private_data(sw);

  // Rofi is not yet exporting these constants in their headers
  // *state |= MARKUP;
  // https://github.com/DaveDavenport/rofi/blob/79adae77d72be3de96d1c4e6d53b6bae4cb7e00e/include/widgets/textbox.h#L104
  *state |= 8;

  // Only return the string if requested, otherwise only set state.
  if (!get_entry) {
    return NULL;
  }

  Emoji *emoji = emoji_list_get(pd->emojis, selected_line);

  if (emoji == NULL) {
    return g_strdup("n/a");
  } else {
    const char *format = pd->format;

    if (format == NULL || format[0] == '\0') {
      format = DEFAULT_FORMAT;
    }

    return format_emoji(emoji, format);
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
static int emoji_token_match(const Mode *sw, rofi_int_matcher **tokens,
                             unsigned int index) {
  EmojiModePrivateData *pd = (EmojiModePrivateData *)mode_get_private_data(sw);
  return index < pd->emojis->length &&
         helper_token_match(tokens, pd->matcher_strings[index]);
}

Mode mode = {
    .abi_version = ABI_VERSION,
    .name = "emoji",
    .cfg_name_key = "emoji",
    ._init = emoji_mode_init,
    ._get_num_entries = emoji_mode_get_num_entries,
    ._result = emoji_mode_result,
    ._destroy = emoji_mode_destroy,
    ._token_match = emoji_token_match,
    ._get_display_value = get_display_value,
    ._get_message = emoji_get_message,
    ._get_completion = NULL,
    ._preprocess_input = NULL,
    .private_data = NULL,
    .free = NULL,
};
