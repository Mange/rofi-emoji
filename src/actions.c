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

ModeMode copy_emoji(EmojiModePrivateData *pd, unsigned int line) {
  Emoji *emoji = get_selected_emoji(pd, line);
  if (emoji == NULL) {
    return MODE_EXIT;
  }

  if (run_clipboard_adapter("copy", emoji, &(pd->message))) {
    return MODE_EXIT;
  } else {
    // Copying failed, reload dialog to show error message in pd->message.
    return RELOAD_DIALOG;
  }
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
  case COPY_EMOJI:
    return copy_emoji(pd, line);
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
