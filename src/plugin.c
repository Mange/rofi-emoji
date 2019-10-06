#include <glib.h>

#include <rofi/mode.h>
#include <rofi/helper.h>
#include <rofi/mode-private.h>

#include "emoji.h"
#include "emoji_list.h"
#include "loader.h"

G_MODULE_EXPORT Mode mode;

typedef struct {
  EmojiList *emojis;
  char **matcher_strings;
  char *message;
} EmojiModePrivateData;

// Execute the clipboard adapter with the "copy" action so the selected emoji
// is copied to the users' clipboard.
//
// Returns TRUE on success, or return FALSE on error after setting the provided
// error buffer to a user error message.
int copy_emoji(Emoji *emoji, char **error) {
  return run_clipboard_adapter("copy", emoji, error);
}

char **generate_matcher_strings(EmojiList *list) {
  char **strings = g_new(char*, list->length);
  for (int i = 0; i < list->length; ++i) {
    Emoji *emoji = emoji_list_get(list, i);
    strings[i] = g_strdup_printf(
      "%s %s %s %s / %s",
      emoji->bytes, emoji->name, emoji->keywords, emoji->group, emoji->subgroup
    );
  }
  return strings;
}

static void get_emoji(Mode *sw) {
  EmojiModePrivateData *pd = (EmojiModePrivateData *) mode_get_private_data(sw);
  char *path;

  FindDataFileResult result = find_emoji_file(&path);
  if (result == SUCCESS) {
    pd->emojis = read_emojis_from_file(path);
    pd->matcher_strings = generate_matcher_strings(pd->emojis);
  } else {
    if (result == CANNOT_DETERMINE_PATH) {
      pd->message = g_strdup(
        "Failed to load emoji file: The path could not be determined"
      );
    } else if (result == NOT_A_FILE) {
      pd->message = g_markup_printf_escaped(
        "Failed to load emoji file: <tt>%s</tt> is not a file\nAlso searched "\
        "in every path in $XDG_DATA_DIRS.",
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
  const EmojiModePrivateData *pd =
    (const EmojiModePrivateData *) mode_get_private_data(sw);
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
static ModeMode emoji_mode_result(
  Mode *sw,
  int mretv,
  char **input,
  unsigned int selected_line
) {
  ModeMode retv = MODE_EXIT;
  EmojiModePrivateData *pd = (EmojiModePrivateData *) mode_get_private_data(sw);

  if (mretv & MENU_NEXT) {
    retv = NEXT_DIALOG;
  } else if (mretv & MENU_PREVIOUS) {
    retv = PREVIOUS_DIALOG;
  } else if (mretv & MENU_QUICK_SWITCH) {
    retv = (mretv & MENU_LOWER_MASK);
  } else if ((mretv & MENU_OK) ) {
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
 * @return a new allocated (valid pango markup) message to display (user should
 * free).
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
static char *get_display_value(
  const Mode *sw,
  unsigned int selected_line,
  G_GNUC_UNUSED int *state,
  G_GNUC_UNUSED GList **attr_list,
  int get_entry
) {
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
      "%s <span weight='bold'>%s</span> <span size='small'>(%s)</span> <span size='x-small'>[%s / %s]</span>",
      emoji->bytes, emoji->name, emoji->keywords, emoji->group, emoji->subgroup
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
