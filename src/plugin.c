#include <glib.h>

// Must be included before other rofi includes.
#include <rofi/mode.h>

#include <rofi/helper.h>
#include <rofi/mode-private.h>

#include "actions.h"
#include "emoji.h"
#include "formatter.h"
#include "loader.h"
#include "menu.h"
#include "plugin.h"
#include "search.h"
#include "utils.h"

G_MODULE_EXPORT Mode mode;

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

static void get_emoji(EmojiModePrivateData *pd) {
  char *path;

  FindDataFileResult result = find_emoji_file(&path);
  if (result == SUCCESS) {
    pd->emojis = read_emojis_from_file(path);
  } else {
    if (result == CANNOT_DETERMINE_PATH) {
      pd->message = g_strdup(
          "Failed to load emoji file: The path could not be determined");
    } else if (result == NOT_A_FILE) {
      pd->message = g_markup_printf_escaped(
          "Failed to load emoji file: <tt>%s</tt> is not a file", path);
    }
    pd->emojis = NULL;
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
    pd->selected_emoji = NULL;
    pd->message = NULL;

    // Search
    pd->search_default_action = INSERT_EMOJI;
    pd->search_matcher_strings = NULL;
    pd->format = NULL;
    pd->group_matchers = NULL;
    pd->subgroup_matchers = NULL;

    // Menu
    pd->menu_matcher_strings = NULL;

    if (find_arg("-emoji-format")) {
      char *format;
      if (find_arg_str("-emoji-format", &format)) {
        // We want ownership of this data and not rely on a reference to global
        // data.
        pd->format = g_strdup(format);
      }
    }

    if (find_arg("-emoji-mode")) {
      char *format;
      if (find_arg_str("-emoji-mode", &format)) {
        if (strcmp(format, "insert") == 0) {
          pd->search_default_action = INSERT_EMOJI;
        } else if (strcmp(format, "copy") == 0) {
          pd->search_default_action = COPY_EMOJI;
        } else if (strcmp(format, "menu") == 0) {
          pd->search_default_action = OPEN_MENU;
        } else if (strcmp(format, "stdout") == 0) {
          pd->search_default_action = OUTPUT_EMOJI;
        } else {
          g_critical("Invalid emoji-mode: %s. Falling back to insert.", format);
          pd->search_default_action = INSERT_EMOJI;
        }
      }
    }

    get_emoji(pd);
    if (pd->emojis == NULL) {
      return FALSE;
    }

    emoji_search_init(pd);
    emoji_menu_init(pd);
    mode_set_private_data(sw, (void *)pd);
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
  if (pd->selected_emoji == NULL) {
    return emoji_search_get_num_entries(pd);
  } else {
    return emoji_menu_get_num_entries(pd);
  }
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
  EmojiModePrivateData *pd = (EmojiModePrivateData *)mode_get_private_data(sw);
  Event event = EXIT;

  if (mretv & MENU_NEXT) {
    return NEXT_DIALOG;
  } else if (mretv & MENU_PREVIOUS) {
    return PREVIOUS_DIALOG;
  } else if (mretv & MENU_QUICK_SWITCH) {
    return (mretv & MENU_LOWER_MASK);
  } else if ((mretv & MENU_ENTRY_DELETE) == MENU_ENTRY_DELETE) {
    return RESET_DIALOG;
  } else if (mretv & MENU_CANCEL) {
    event = EXIT;
  } else if (mretv & MENU_CUSTOM_COMMAND) {
    if ((mretv & MENU_LOWER_MASK) == 0) {
      event = SELECT_CUSTOM_1;
    } else {
      return RELOAD_DIALOG;
    }
  } else if ((mretv & MENU_OK)) {
    if ((mretv & MENU_CUSTOM_ACTION) == MENU_CUSTOM_ACTION) {
      event = SELECT_ALTERNATIVE;
    } else {
      event = SELECT_DEFAULT;
    }
  }

  Action action = EXIT_SEARCH;
  if (pd->selected_emoji == NULL) {
    action = emoji_search_on_event(pd, event, selected_line);
  } else {
    action = emoji_menu_on_event(pd, event, selected_line);
  }

  return perform_action(pd, action, selected_line);
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
    emoji_search_destroy(pd);
    emoji_menu_destroy(pd);

    pd->selected_emoji = NULL; // Freed via the emojis list
    g_ptr_array_free(pd->emojis, TRUE);

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

  if (pd->message != NULL) {
    return g_strdup(pd->message);
  }

  if (pd->selected_emoji == NULL) {
    return emoji_search_get_message(pd);
  } else {
    return emoji_menu_get_message(pd);
  }
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
static char *emoji_get_display_value(const Mode *sw, unsigned int selected_line,
                                     G_GNUC_UNUSED int *state,
                                     G_GNUC_UNUSED GList **attr_list,
                                     int get_entry) {
  EmojiModePrivateData *pd = (EmojiModePrivateData *)mode_get_private_data(sw);

  *state |= STATE_MARKUP;

  // Only return the string if requested, otherwise only set state.
  if (!get_entry) {
    return NULL;
  }

  if (pd->selected_emoji == NULL) {
    return emoji_search_get_display_value(pd, selected_line);
  } else {
    return emoji_menu_get_display_value(pd, selected_line);
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

  if (pd->selected_emoji == NULL) {
    return emoji_search_token_match(pd, tokens, index);
  } else {
    return emoji_menu_token_match(pd, tokens, index);
  }
}

static char *emoji_preprocess_input(Mode *sw, const char *input) {
  EmojiModePrivateData *pd = (EmojiModePrivateData *)mode_get_private_data(sw);
  if (pd->selected_emoji == NULL) {
    return emoji_search_preprocess_input(pd, input);
  } else {
    return emoji_menu_preprocess_input(pd, input);
  }
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
    ._get_display_value = emoji_get_display_value,
    ._get_message = emoji_get_message,
    ._get_completion = NULL,
    ._preprocess_input = emoji_preprocess_input,
    .private_data = NULL,
    .free = NULL,
};
