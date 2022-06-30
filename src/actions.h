#ifndef ACTIONS_H
#define ACTIONS_H

#include <rofi/helper.h>

typedef enum {
  NOOP,
  INSERT_EMOJI,
  COPY_EMOJI,
  OUTPUT_EMOJI,
  COPY_NAME,
  COPY_CODEPOINT,
  OPEN_MENU,
  EXIT_MENU,
  EXIT_SEARCH,
} Action;

#include "plugin.h"

ModeMode perform_action(EmojiModePrivateData *pd, const Action action,
                        unsigned int line);

#endif // ACTIONS_H
