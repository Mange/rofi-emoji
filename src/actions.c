#include "actions.h"
#include "menu.h"
#include "utils.h"

Emoji *get_selected_emoji(EmojiModePrivateData *pd, unsigned int line) {
  if (pd->selected_emoji != NULL) {
    return pd->selected_emoji;
  }

  if (line >= pd->emojis->len) {
    return NULL;
  }

  return g_ptr_array_index(pd->emojis, line);
}

ModeMode text_adapter_action(const char *action, EmojiModePrivateData *pd,
                             const char *text) {
  if (run_clipboard_adapter(action, text, &(pd->message))) {
    return MODE_EXIT;
  } else {
    // Copying failed, reload dialog to show error message in pd->message.
    return RELOAD_DIALOG;
  }
}

ModeMode copy_emoji(EmojiModePrivateData *pd, unsigned int line) {
  const Emoji *emoji = get_selected_emoji(pd, line);
  if (emoji == NULL) {
    return MODE_EXIT;
  }

  return text_adapter_action("copy", pd, emoji->bytes);
}

ModeMode insert_emoji(EmojiModePrivateData *pd, unsigned int line) {
  const Emoji *emoji = get_selected_emoji(pd, line);
  if (emoji == NULL) {
    return MODE_EXIT;
  }

  // Must hide window and give back focus to whatever app should receive the
  // insert action.
  rofi_view_hide();
  text_adapter_action("insert", pd, emoji->bytes);

  // View is hidden and we cannot get it back again. We must exit at this point.
  return MODE_EXIT;
}

ModeMode output_emoji(EmojiModePrivateData *pd, unsigned int line) {
  const Emoji *emoji = get_selected_emoji(pd, line);
  if (emoji == NULL) {
    return MODE_EXIT;
  }

  // Reuse Rofi's dmenu format settings and semantics.
  char *format = "s";
  find_arg_str("-format", &format);
  rofi_output_formatted_line(format, emoji->bytes, line, "");

  return MODE_EXIT;
}

ModeMode copy_codepoint(EmojiModePrivateData *pd, unsigned int line) {
  const Emoji *emoji = get_selected_emoji(pd, line);
  if (emoji == NULL) {
    return MODE_EXIT;
  }

  return text_adapter_action("copy", pd, codepoint(emoji->bytes));
}

ModeMode copy_name(EmojiModePrivateData *pd, unsigned int line) {
  const Emoji *emoji = get_selected_emoji(pd, line);
  if (emoji == NULL) {
    return MODE_EXIT;
  }

  return text_adapter_action("copy", pd, emoji->name);
}

ModeMode open_menu(EmojiModePrivateData *pd, unsigned int line) {
  if (line >= pd->emojis->len) {
    return MODE_EXIT;
  }

  Emoji *emoji = g_ptr_array_index(pd->emojis, line);
  if (emoji == NULL) {
    return MODE_EXIT;
  }

  pd->selected_emoji = emoji;
  emoji_menu_init(pd);

  return RESET_DIALOG;
}

ModeMode exit_menu(EmojiModePrivateData *pd, unsigned int line) {
  emoji_menu_destroy(pd);
  pd->selected_emoji = NULL;
  return RESET_DIALOG;
}

ModeMode exit_search(EmojiModePrivateData *pd, unsigned int line) {
  return MODE_EXIT;
}

ModeMode perform_action(EmojiModePrivateData *pd, const Action action,
                        unsigned int line) {
  switch (action) {
  case NOOP:
    return RELOAD_DIALOG;
  case INSERT_EMOJI:
    return insert_emoji(pd, line);
  case COPY_EMOJI:
    return copy_emoji(pd, line);
  case OUTPUT_EMOJI:
    return output_emoji(pd, line);
  case COPY_NAME:
    return copy_name(pd, line);
  case COPY_CODEPOINT:
    return copy_codepoint(pd, line);
  case OPEN_MENU:
    return open_menu(pd, line);
  case EXIT_MENU:
    return exit_menu(pd, line);
  case EXIT_SEARCH:
    return exit_search(pd, line);
  default:
    g_assert_not_reached();
  }
}
