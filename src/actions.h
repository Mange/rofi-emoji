#ifndef ACTIONS_H
#define ACTIONS_H

#include <rofi/helper.h>

#include "plugin.h"

typedef enum {
  NOOP,
  COPY_EMOJI,
  COPY_CODEPOINT,
  OPEN_MENU,
  EXIT_MENU,
  EXIT_SEARCH,
} Action;

ModeMode perform_action(EmojiModePrivateData *pd, const Action action,
                        unsigned int line);

#endif // ACTIONS_H
